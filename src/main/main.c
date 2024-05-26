#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_check.h"
#include "sdkconfig.h"
#include <esp_system.h>
#include <string.h>
#include <inttypes.h>
#include <time.h>
#include "freertos/semphr.h"
#include "freertos/queue.h"


#include "driver/i2s_std.h"
#include "./pcm1862/pcm1862.h"
#include "./ParamEQ/ParamEQ.h"
#include <encoder.h>
#include "lvgl.h"
#include "display_init/display_init.h"
#include "../ui.c"


static const char *TAG = "stereo_adc_dac";
static const char err_reason[][30] = {"input param is invalid", "operation timeout"};




/* ------------ Default Filter Config ------------ */
float volume                =   1.0f;
float fc                    =   1000.0f;
float B                     =   10.0f;
float g                     =   1.0f;
/*-------------------------------------------------*/




/* ----------------- I2S defines ----------------- */
#define I2S_DUPLEX_MODE         CONFIG_USE_DUPLEX
#define BCLK        GPIO_NUM_15     // I2S bit clock io number
#define WS          GPIO_NUM_16     // I2S word select io number
#define DOUT        GPIO_NUM_18     // I2S data out io number
#define DIN         GPIO_NUM_17     // I2S data in io number
#define BUFF_SIZE               256
#define SAMPLE_RATE_HZ          96000
#define SAMPLE_RATE_HZ_F        96000.0f
/*-------------------------------------------------*/




/* ------------- I2S struct handles -------------- */
static i2s_chan_handle_t      tx_chan;        // I2S tx channel handler
static i2s_chan_handle_t      rx_chan;        // I2S rx channel handler
static ParamEQ                peak_filt_r;    // Peaking Filter struct
static ParamEQ                peak_filt_l;    // Peaking Filter struct
/*-------------------------------------------------*/




/* ---------------- GPIO defines ----------------- */
#define DA_SMUTE    GPIO_NUM_46
#define DA_PWR_DOWN GPIO_NUM_9
#define AMP_ENABLE  GPIO_NUM_10
/*-------------------------------------------------*/




/* ------------ Encoder pin assignment ----------- */
#define RE0_A_GPIO   45
#define RE0_B_GPIO   48
#define RE0_BTN_GPIO 47
#define RE1_A_GPIO   21
#define RE1_B_GPIO   20
#define RE1_BTN_GPIO 19

#define EV_QUEUE_LEN 5
/*-------------------------------------------------*/




/* ------------ Encoder struct handles ----------- */
static QueueHandle_t event_queue;
static rotary_encoder_t re0;
static rotary_encoder_t re1;

extern int8_t menu_scroller = 1;
extern int8_t menu_id = 0;

struct input_manager{
    int8_t id;
    int8_t scroller;
    int8_t diff;
    bool change;
    bool click;
};
struct input_manager menuCon = {0, 1, 0, 0, 0};
/*-------------------------------------------------*/




/* -------- LVGL menu element declaration -------- */
static lv_style_t main_style;
static lv_style_t style_line;
static lv_point_t line_points[] = { {5, 55}, {200, 55}};

lv_obj_t *ui_main_screen;
lv_obj_t *ui_main_list;

lv_obj_t *menu_title1;

lv_obj_t * chart;
lv_chart_series_t * ser1;

lv_obj_t * line1;

struct labels{
    char filtType[2][30];
};
struct labels entries = {{"Low Pass Filter","High Pass Filter"}};
/*-------------------------------------------------*/




/* --------- I2S initialization function --------- */
static esp_err_t i2s_init_std_duplex(void)
{
    i2s_chan_config_t chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_AUTO, I2S_ROLE_SLAVE);
    chan_cfg.auto_clear = true; // Auto clear the legacy data in the DMA buffer
    ESP_ERROR_CHECK(i2s_new_channel(&chan_cfg, &tx_chan, &rx_chan));

    i2s_std_config_t std_cfg = {
        .clk_cfg  = I2S_STD_CLK_DEFAULT_CONFIG(SAMPLE_RATE_HZ),
        .slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_32BIT, I2S_SLOT_MODE_STEREO),
        .gpio_cfg = {
            .mclk = I2S_GPIO_UNUSED,    // some codecs may require mclk signal, this example doesn't need it
            .bclk = BCLK,
            .ws   = WS,
            .dout = DOUT,
            .din  = DIN, // In duplex mode, bind output and input to a same gpio can loopback internally
            .invert_flags = {
                .mclk_inv = false,
                .bclk_inv = false,
                .ws_inv   = false,
            },
        },
    };
    /* Initialize the channels */
    ESP_ERROR_CHECK(i2s_channel_init_std_mode(tx_chan, &std_cfg));
    ESP_ERROR_CHECK(i2s_channel_init_std_mode(rx_chan, &std_cfg));
    /* Enable the TX channel */
    ESP_ERROR_CHECK(i2s_channel_enable(tx_chan));
    /* Enable the RX channel */
    ESP_ERROR_CHECK(i2s_channel_enable(rx_chan));

    return ESP_OK;

}
/*-------------------------------------------------*/




/* ------------- Digital filter task ------------- */
static void i2s_process(void *args)
{
    int32_t *filtIN_buf = malloc(BUFF_SIZE);
    if (!filtIN_buf) {
        ESP_LOGE("i2s_process", "No memory for filterIn data buffer");
        abort();
    }
    int32_t *filtOUT_buf = malloc(BUFF_SIZE);
    if (!filtOUT_buf) {
        ESP_LOGE("i2s_process", "No memory for filterOut data buffer");
        abort();
    }

    static float leftIn, rightIn;
    static float leftProcessed, rightProcessed;
    static int32_t leftOut, rightOut;

    esp_err_t ret = ESP_OK;
    size_t bytes_read = 0;
    size_t bytes_write = 0;


    ESP_LOGI("i2s_process", "i2s_process start");

    while (1) {
        memset(filtIN_buf, 0, BUFF_SIZE);
        memset(filtOUT_buf, 0, BUFF_SIZE);

        /* Read I2S data */
        ret = i2s_channel_read(rx_chan, filtIN_buf, BUFF_SIZE, &bytes_read, 1000);
        if (ret != ESP_OK) {
            ESP_LOGE("i2s_process", "i2s read failed, %s", err_reason[ret == ESP_ERR_TIMEOUT]);
            abort();
        }

        /* Apply filter */
        for (int32_t n = 0; n < (BUFF_SIZE) - 1; n += 2){
            
            /* Conver int to float */
            leftIn = (float)filtIN_buf[n];
            rightIn = (float)filtIN_buf[n+1];
            
            /* Process samples via filter */
            leftProcessed = ParamEQ_Update(&peak_filt_l, leftIn);
            rightProcessed = ParamEQ_Update(&peak_filt_r, rightIn);
            
            /* Conver float to int */
            leftOut = (int32_t)(leftProcessed * volume);
            rightOut = (int32_t)(rightProcessed * volume);

            /* Set output buffer samples */
            filtOUT_buf[n]   = leftOut;
            filtOUT_buf[n+1] = rightOut;

        }

        /* Write I2S data */
        ret = i2s_channel_write(tx_chan, filtOUT_buf, BUFF_SIZE, &bytes_write, 1000);
        if (ret != ESP_OK) {
            ESP_LOGE("i2s_process", "i2s write failed, %s", err_reason[ret == ESP_ERR_TIMEOUT]);
            abort();
        }
        if (bytes_read != bytes_write) {
            ESP_LOGW("i2s_process", "%d bytes read but only %d bytes are written", bytes_read, bytes_write);
        }

    }
    vTaskDelete(NULL);
}
/*-------------------------------------------------*/




/* ----------------- Encoder task ---------------- */
void encoder_task(void *arg)
{
    // Create event queue for rotary encoders
    event_queue = xQueueCreate(EV_QUEUE_LEN, sizeof(rotary_encoder_event_t));

    // Setup rotary encoder library
    ESP_ERROR_CHECK(rotary_encoder_init(event_queue));

    // Add one encoder
    memset(&re0, 0, sizeof(rotary_encoder_t));
    re0.pin_a = RE0_A_GPIO;
    re0.pin_b = RE0_B_GPIO;
    re0.pin_btn = RE0_BTN_GPIO;
    ESP_ERROR_CHECK(rotary_encoder_add(&re0));

    // Add one encoder
    memset(&re1, 0, sizeof(rotary_encoder_t));
    re1.pin_a = RE1_A_GPIO;
    re1.pin_b = RE1_B_GPIO;
    re1.pin_btn = RE1_BTN_GPIO;
    ESP_ERROR_CHECK(rotary_encoder_add(&re1));

    rotary_encoder_event_t e;
    int32_t en0val = 0;
    int32_t en1val = 0;
    int32_t enID = 0;


    ESP_LOGI("encoder_task", "Initial value: %" PRIi32, en0val);
    ESP_LOGI("encoder_task", "Initial value: %" PRIi32, en1val);
    while (1)
    {
        xQueueReceive(event_queue, &e, portMAX_DELAY);
        rotary_encoder_t *sender = e.sender;
        enID = sender->index;

        switch (e.type)
        {
            case RE_ET_BTN_PRESSED:
                // ESP_LOGI(TAG, "Button pressed");
                ESP_LOGI("encoder_task", "Button pressed");
                break;
            case RE_ET_BTN_RELEASED:
                ESP_LOGI("encoder_task", "Button released");
                break;
            case RE_ET_BTN_CLICKED:
                if (enID == 1){
                    ESP_LOGE("encoder_task", "Encoders status:");
                    ESP_LOGE("encoder_task", "%d: %d", (int)enID - 1, (int)en0val);
                    ESP_LOGE("encoder_task", "%d: %d", (int)enID, (int)en1val);
                    break;
                }
                ESP_LOGI("encoder_task", "Button clicked");

                menuCon.click = 1;

                break;
            case RE_ET_BTN_LONG_PRESSED:
                if (enID == 1){
                    en0val = 0;
                    en1val = 0;
                    ESP_LOGW("encoder_task", "Encoders reset");
                    break;
                }
                ESP_LOGI("encoder_task", "Looooong pressed button");
                break;
            case RE_ET_CHANGED:
                // enID = sender->index;
                if (enID == 0){
                    en0val += e.diff;
                    menuCon.diff = e.diff;

                    menuCon.scroller -= menuCon.diff;
                    menuCon.change = 1;
                    if (menuCon.id == 0){
                        if (menuCon.scroller > 3) {
                            menuCon.scroller = 0;
                        }
                        else if (menuCon.scroller < 0){
                            menuCon.scroller = 3;
                        }
                    }
                    if (menuCon.id == 1){
                        if (menuCon.scroller > 1) {
                            menuCon.scroller = 0;
                        }
                        else if (menuCon.scroller < 0){
                            menuCon.scroller = 1;
                        }
                    }
                    

                    // ESP_LOGI(TAG, "Value = %" PRIi32, en0val);
                    ESP_LOGI("encoder_task", "Encoder %d: %d", (int)enID, (int)en0val);
                }
                else if (enID == 1){
                    en1val += e.diff;
                    // ESP_LOGI(TAG, "Value = %" PRIi32, en1val);
                    ESP_LOGI("encoder_task", "Encoder %d: %d", (int)enID, (int)en1val);
                }
                break;
            default:
                break;
        }
    }
}
/*-------------------------------------------------*/



void app_main() {
        
    /* ---------------- Initialize LVGL -------------- */
    vTaskDelay(pdMS_TO_TICKS(3000));
    lvgl_init();
    vTaskDelay(pdMS_TO_TICKS(2000));
    
    if (pdTRUE == xSemaphoreTake(xGuiSemaphore, portMAX_DELAY)) {
        
        ui_init();
        xSemaphoreGive(xGuiSemaphore);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
    /*-------------------------------------------------*/


    vTaskDelay(pdMS_TO_TICKS(100));


    /* ------- Set GPIO levels for DAC and AMP ------- */
    ESP_LOGW(TAG,"GPIO init\n");
    gpio_set_direction(DA_SMUTE, GPIO_MODE_OUTPUT);
    gpio_set_direction(DA_PWR_DOWN, GPIO_MODE_OUTPUT);
    gpio_set_direction(AMP_ENABLE, GPIO_MODE_OUTPUT);

    gpio_set_level(DA_SMUTE, 0); // DA_SMUTE - L
    gpio_set_level(DA_PWR_DOWN, 1); // DA_PWR_DOWN - H
    gpio_set_level(AMP_ENABLE, 1); // AMP_ENABLE - H
    /*-------------------------------------------------*/


    vTaskDelay(pdMS_TO_TICKS(100));


    /* ----------- Initialize I2C and ADC ------------ */
    ESP_LOGW(TAG,"ADC Init\n");
    pcm1862_init();
    /*-------------------------------------------------*/


    vTaskDelay(pdMS_TO_TICKS(100));


    /* ---------------- Initialize I2S --------------- */
    ESP_LOGW(TAG,"I2S Init\n");
    if (i2s_init_std_duplex() != ESP_OK) {          // Verify init success, abort if failed
        ESP_LOGE(TAG, "i2s driver init failed");
        abort();
    } else {
        ESP_LOGI(TAG, "i2s driver init success");
    }
    /*-------------------------------------------------*/


    vTaskDelay(pdMS_TO_TICKS(100));


    /* ------------ Initialize ParamFilter ----------- */
    ESP_LOGW(TAG,"ParamFilter Init\n");
    ParamEQ_Init(&peak_filt_r, SAMPLE_RATE_HZ_F);
    ParamEQ_Init(&peak_filt_l, SAMPLE_RATE_HZ_F);
    ParamEQ_SetParameters(&peak_filt_r, fc, B, g);
    ParamEQ_SetParameters(&peak_filt_l, fc, B, g);
    /*-------------------------------------------------*/


    vTaskDelay(pdMS_TO_TICKS(100));


    /* ----------------- Create tasks ---------------- */
    if (xTaskCreatePinnedToCore(encoder_task, "encoder_task", configMINIMAL_STACK_SIZE * 8, NULL, 5, NULL, 1) != pdPASS){ 
        ESP_LOGE(TAG,"encoder_task init fail!\n");
        abort();    // Verify task creation success, abort if failed
    } else {
        ESP_LOGW(TAG,"encoder_task init success, assigned to CPU1\n"); // Assigned to CPU1
    }

    vTaskDelay(pdMS_TO_TICKS(1000));

    if (xTaskCreatePinnedToCore(i2s_process, "i2s_process", 8192, NULL, 5, NULL, 0) != pdPASS){ 
        ESP_LOGE(TAG,"i2s_process init fail!\n");
        abort();    // Verify task creation success, abort if failed
    } else {
        ESP_LOGW(TAG,"i2s_process init success, assigned to CPU0\n"); // Assigned to CPU0
    }
    /*-------------------------------------------------*/





    vTaskDelay(pdMS_TO_TICKS(100));

    /* - While loop for menu navigaiton via encoders - */
    while (true) {
        // ESP_LOGI("MAIN", "\n");

        if (menuCon.id == 0 && menuCon.change){      // If on main menu, wrap around scroller. Trigger on encoder change.
            menuCon.change = 0;
            lv_roller_set_selected(ui_Roller1, menuCon.scroller, LV_ANIM_OFF);      // Roller entry selector
        }
        // else if (menuCon.id == 1 && menuCon.change){
        //     lv_label_set_text(ui_Label2, entries.filtType[menuCon.scroller]);
        // }
        if (menuCon.click){
            menuCon.click = 0;
            if (menuCon.id == 0 && menuCon.scroller == 1){      // Selecting "Filter Type" entry
                menuCon.id = 1;
                menuCon.scroller = 0;
            }
            
        }
        
        vTaskDelay(pdMS_TO_TICKS(250));
    }
    /*-------------------------------------------------*/
}