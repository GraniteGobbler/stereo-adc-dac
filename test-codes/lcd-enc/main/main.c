#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_system.h"
#include "sdkconfig.h"
#include <esp_system.h>
#include <string.h>
#include <inttypes.h>
#include <time.h>
#include "freertos/semphr.h"
#include "freertos/queue.h"

#include <encoder.h>

#include "lvgl.h"
#include "display_init/display_init.h"
#include "../ui.c"




#define RE0_A_GPIO   42
#define RE0_B_GPIO   41
#define RE0_BTN_GPIO 40
#define RE1_A_GPIO   39
#define RE1_B_GPIO   38
#define RE1_BTN_GPIO 37

#define EV_QUEUE_LEN 5


static const char *TAG = "encoder_example";

static QueueHandle_t event_queue;
static rotary_encoder_t re0;
static rotary_encoder_t re1;

static lv_style_t main_style;
static lv_style_t style_line;
static lv_point_t line_points[] = { {5, 55}, {200, 55}};

lv_obj_t *ui_main_screen;
lv_obj_t *ui_main_list;

lv_obj_t *menu_title1;

lv_obj_t * chart;
lv_chart_series_t * ser1;

lv_obj_t * line1;

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


struct labels{
    char filtType[2][30];
};
struct labels entries = {{"Low Pass Filter","High Pass Filter"}};




void test(void *arg)
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


    ESP_LOGI(TAG, "Initial value: %" PRIi32, en0val);
    ESP_LOGI(TAG, "Initial value: %" PRIi32, en1val);
    while (1)
    {
        xQueueReceive(event_queue, &e, portMAX_DELAY);
        rotary_encoder_t *sender = e.sender;
        enID = sender->index;

        switch (e.type)
        {
            case RE_ET_BTN_PRESSED:
                // ESP_LOGI(TAG, "Button pressed");
                ESP_LOGI(TAG, "Button pressed");
                break;
            case RE_ET_BTN_RELEASED:
                ESP_LOGI(TAG, "Button released");
                break;
            case RE_ET_BTN_CLICKED:
                if (enID == 1){
                    ESP_LOGE(TAG, "Encoders status:");
                    ESP_LOGE(TAG, "%d: %d", (int)enID - 1, (int)en0val);
                    ESP_LOGE(TAG, "%d: %d", (int)enID, (int)en1val);
                    break;
                }
                ESP_LOGI(TAG, "Button clicked");

                menuCon.click = 1;

                break;
            case RE_ET_BTN_LONG_PRESSED:
                if (enID == 1){
                    en0val = 0;
                    en1val = 0;
                    ESP_LOGW(TAG, "Encoders reset");
                    break;
                }
                ESP_LOGI(TAG, "Looooong pressed button");
                break;
            case RE_ET_CHANGED:
                // enID = sender->index;
                if (enID == 0){
                    en0val -= e.diff;
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
                    ESP_LOGI(TAG, "Encoder %d: %d", (int)enID, (int)en0val);
                }
                else if (enID == 1){
                    en1val -= e.diff;
                    // ESP_LOGI(TAG, "Value = %" PRIi32, en1val);
                    ESP_LOGI(TAG, "Encoder %d: %d", (int)enID, (int)en1val);
                }
                break;
            default:
                break;
        }
    }
}



void app_main() {

    xTaskCreate(test, TAG, configMINIMAL_STACK_SIZE * 8, NULL, 5, NULL);

    vTaskDelay(pdMS_TO_TICKS(3000));
    lvgl_init();
    vTaskDelay(pdMS_TO_TICKS(2000));
    

    if (pdTRUE == xSemaphoreTake(xGuiSemaphore, portMAX_DELAY)) {

        ui_init();

        xSemaphoreGive(xGuiSemaphore);

        vTaskDelay(pdMS_TO_TICKS(1000));
    }


    
    while (true) {
        // ESP_LOGI("MAIN", "\n");

        if (menuCon.id == 0 && menuCon.change){      // If on main menu, wrap around scroller. Trigger on encoder change.
            menuCon.change = 0;
            lv_roller_set_selected(ui_Roller1, menuCon.scroller, LV_ANIM_OFF);      // Roller entry selector
        }
        else if (menuCon.id == 1 && menuCon.change){
            lv_label_set_text(ui_Label2, entries.filtType[menuCon.scroller]);
        }
        if (menuCon.click){
            menuCon.click = 0;
            if (menuCon.id == 0 && menuCon.scroller == 1){      // Selecting "Filter Type" entry
                menuCon.id = 1;
                menuCon.scroller = 0;
            }
            
        }
        
        vTaskDelay(pdMS_TO_TICKS(250));

    }

}