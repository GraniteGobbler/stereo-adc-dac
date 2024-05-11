/*
 * SPDX-FileCopyrightText: 2021-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */

#include <stdint.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2s_std.h"
#include "driver/gpio.h"
#include "esp_check.h"
#include "sdkconfig.h"
#include "./pcm1862/pcm1862.c"
#include "./IFX_PeakingFilter/IFX_PeakingFilter.h"

static const char *TAG = "I2S TEST";

/* Set 1 to allocate rx & tx channels in duplex mode on a same I2S controller, they will share the BCLK and WS signal
 * Set 0 to allocate rx & tx channels in simplex mode, these two channels will be totally separated,
 * Specifically, due to the hardware limitation, the simplex rx & tx channels can't be registered on the same controllers on ESP32 and ESP32-S2,
 * and ESP32-S2 has only one I2S controller, so it can't allocate two simplex channels */
#define EXAMPLE_I2S_DUPLEX_MODE         CONFIG_USE_DUPLEX

#if CONFIG_IDF_TARGET_ESP32
    #define EXAMPLE_STD_BCLK_IO1        GPIO_NUM_4      // I2S bit clock io number
    #define EXAMPLE_STD_WS_IO1          GPIO_NUM_5      // I2S word select io number
    #define EXAMPLE_STD_DOUT_IO1        GPIO_NUM_18     // I2S data out io number
    #define EXAMPLE_STD_DIN_IO1         GPIO_NUM_19     // I2S data in io number
    #if !EXAMPLE_I2S_DUPLEX_MODE
        #define EXAMPLE_STD_BCLK_IO2    GPIO_NUM_22     // I2S bit clock io number
        #define EXAMPLE_STD_WS_IO2      GPIO_NUM_23     // I2S word select io number
        #define EXAMPLE_STD_DOUT_IO2    GPIO_NUM_25     // I2S data out io number
        #define EXAMPLE_STD_DIN_IO2     GPIO_NUM_26     // I2S data in io number
    #endif
#else
    #define EXAMPLE_STD_BCLK_IO1        GPIO_NUM_15      // I2S bit clock io number
    #define EXAMPLE_STD_WS_IO1          GPIO_NUM_16      // I2S word select io number
    #define EXAMPLE_STD_DOUT_IO1        GPIO_NUM_18      // I2S data out io number
    #define EXAMPLE_STD_DIN_IO1         GPIO_NUM_17      // I2S data in io number
    #if !EXAMPLE_I2S_DUPLEX_MODE
        #define EXAMPLE_STD_BCLK_IO2    GPIO_NUM_6      // I2S bit clock io number
        #define EXAMPLE_STD_WS_IO2      GPIO_NUM_7      // I2S word select io number
        #define EXAMPLE_STD_DOUT_IO2    GPIO_NUM_8      // I2S data out io number
        #define EXAMPLE_STD_DIN_IO2     GPIO_NUM_9      // I2S data in io number
    #endif
#endif

#define SAMPLE_RATE_HZ_F                96000.0f
#define SAMPLE_RATE_HZ                  96000

#define EXAMPLE_BUFF_SIZE               2048
#define DATA_QUEUE_LEN                  5

#define SLOW_MODE   1
#if SLOW_MODE
    #define READ_TASK_DELAY                 2000   // miliseconds
    #define WRITE_TASK_DELAY                2000
#else
    #define READ_TASK_DELAY                 100
    #define WRITE_TASK_DELAY                100
#endif

static QueueHandle_t                    read_queue;     // I2S read queue
static QueueHandle_t                    write_queue;    // I2S write queue
static i2s_chan_handle_t                tx_chan;        // I2S tx channel handler
static i2s_chan_handle_t                rx_chan;        // I2S rx channel handler
static IFX_PeakingFilter                peak_filt;      // Peaking Filter struct
static SemaphoreHandle_t                mutex;

float outVolume             =   1.0f;
float centerFrequency_Hz    =   100.0f;
float bandwidth_Hz          =   1.0f; 
float boostCut_linear       =   1.0f;

uint8_t *read_data; // Pouzit namiesto queue tuto globalnu premennu, DigiKey mutex



static void i2s_example_read_task(void *args)
{
    uint8_t *r_buf = (uint8_t *)calloc(1, EXAMPLE_BUFF_SIZE);
    assert(r_buf); // Check if r_buf allocation success
    size_t r_bytes = 0;

    /* Enable the RX channel */
    ESP_ERROR_CHECK(i2s_channel_enable(rx_chan));

    /* ATTENTION: The print and delay in the read task only for monitoring the data by human,
     * Normally there shouldn't be any delays to ensure a short polling time,
     * Otherwise the dma buffer will overflow and lead to the data lost */
    while (1) {
        // Take mutex
        if (xSemaphoreTake(mutex, 0) == pdTRUE){

            /* Read i2s data */
            if (i2s_channel_read(rx_chan, r_buf, EXAMPLE_BUFF_SIZE, &r_bytes, 1000) == ESP_OK) {
                // xQueueSendToBack(read_queue, &r_buf, 1);   // Send read buffer to read_queue
                read_data = r_buf;
                printf("Read Task: i2s read %d bytes\n-----------------------------------\n", r_bytes);
                printf("[0] %x [1] %x [2] %x [3] %x\n[4] %x [5] %x [6] %x [7] %x\n\n",
                    r_buf[0], r_buf[1], r_buf[2], r_buf[3], r_buf[4], r_buf[5], r_buf[6], r_buf[7]);
                
                // Give mutex
                xSemaphoreGive(mutex);  

            } else {
                printf("Read Task: i2s read failed\n");
            }
            vTaskDelay(pdMS_TO_TICKS(READ_TASK_DELAY));
        }
        
    }

    

    free(r_buf);
    vTaskDelete(NULL);
}


// /*  Digital filtering function from Phil's Lab #89  */
// static void digital_filter_task(void *args){

//     uint8_t *filtIN_buf = (uint8_t *)calloc(1, EXAMPLE_BUFF_SIZE);
//     assert(filtIN_buf); // Check if filtIN_buf allocation success
//     uint8_t *filtOUT_buf = (uint8_t *)calloc(1, EXAMPLE_BUFF_SIZE);
//     assert(filtOUT_buf); // Check if filtOUT_buf allocation success

//     static float leftIn, rightIn;
//     static float leftProcessed, rightProcessed;
//     static float leftOut, rightOut;
//     // static float Out;

//     xQueueReceive(read_queue, &filtIN_buf, 1);

//     for (uint16_t n = 0; n < (EXAMPLE_BUFF_SIZE/2) - 1; n += 2){
        
//         /* // Convert uint_8 to float     UINT8_TO_FLOAT *  */
//         leftIn = ((float) filtIN_buf[n]);            
//         rightIn = ((float) filtIN_buf[n+1]);
        
//         leftProcessed = IFX_PeakingFilter_Update(&peak_filt, leftIn);
//         rightProcessed = IFX_PeakingFilter_Update(&peak_filt, rightIn);
        
//         /* // Convert float to uint_8      FLOAT_TO_UINT8 *  */
//         leftOut = (uint8_t) (outVolume * leftProcessed);    
//         rightOut = (uint8_t) (outVolume * rightProcessed);

//         /* Set output buffer samples */
//         filtOUT_buf[n]   = leftOut;
//         filtOUT_buf[n+1] = rightOut;

//     }
    
//     xQueueSendToBack(write_queue, &filtOUT_buf, 1);

//     free(filtIN_buf);
//     free(filtOUT_buf);
//     vTaskDelete(NULL);
// }


static void i2s_example_write_task(void *args)
{
    uint8_t *w_buf = (uint8_t *)calloc(1, EXAMPLE_BUFF_SIZE);
    assert(w_buf); // Check if w_buf allocation success


    size_t w_bytes = EXAMPLE_BUFF_SIZE;

    /* (Optional) Preload the data before enabling the TX channel, so that the valid data can be transmitted immediately */
    while (w_bytes == EXAMPLE_BUFF_SIZE) {
        /* Here we load the target buffer repeatedly, until all the DMA buffers are preloaded */
        ESP_ERROR_CHECK(i2s_channel_preload_data(tx_chan, w_buf, EXAMPLE_BUFF_SIZE, &w_bytes));
    }

    /* Enable the TX channel */
    ESP_ERROR_CHECK(i2s_channel_enable(tx_chan));


    while (1) {
        // Take mutex
        if (xSemaphoreTake(mutex, 0) == pdTRUE){

            // ESP_LOGE(TAG, "Write\n");
            // if(xQueueReceive(read_queue, &w_buf, 1)==pdTRUE /*No filter output*/){
            //     ESP_LOGI(TAG,"Queue receive OK!\n");
            // }       
            // xQueueReceive(write_queue, &w_buf, 1);      // Filtered output
            

            w_buf = read_data;
            /* Write i2s data */
            if (i2s_channel_write(tx_chan, w_buf, EXAMPLE_BUFF_SIZE, &w_bytes, 1000) == ESP_OK) {
                
                // ESP_LOGI(TAG, "I2S channel write\n");
                
                printf("Write Task: i2s write %d bytes\n-----------------------------------\n", w_bytes);
                printf("[0] %x [1] %x [2] %x [3] %x\n[4] %x [5] %x [6] %x [7] %x\n\n",
                       w_buf[0], w_buf[1], w_buf[2], w_buf[3], w_buf[4], w_buf[5], w_buf[6], w_buf[7]);
            
                // Give mutex
                xSemaphoreGive(mutex); 

            } else {
                printf("Write Task: i2s write failed\n");
            }
            vTaskDelay(pdMS_TO_TICKS(WRITE_TASK_DELAY));
        }
    }
    free(w_buf);
    vTaskDelete(NULL);
}




//############################################//
//############################################//
//############################################//
////    Callback for encoder value change   ////
////                                        ////
void boban(){
    IFX_PeakingFilter_SetParameters(&peak_filt, centerFrequency_Hz, bandwidth_Hz, boostCut_linear);
}
////                                        ////
//############################################//
//############################################//
//############################################//




#if EXAMPLE_I2S_DUPLEX_MODE
static void i2s_example_init_std_duplex(void)
{
    /* Setp 1: Determine the I2S channel configuration and allocate both channels
     * The default configuration can be generated by the helper macro,
     * it only requires the I2S controller id and I2S role */
    i2s_chan_config_t chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_AUTO, I2S_ROLE_SLAVE);
    ESP_ERROR_CHECK(i2s_new_channel(&chan_cfg, &tx_chan, &rx_chan));

    /* Step 2: Setting the configurations of standard mode, and initialize rx & tx channels
     * The slot configuration and clock configuration can be generated by the macros
     * These two helper macros is defined in 'i2s_std.h' which can only be used in STD mode.
     * They can help to specify the slot and clock configurations for initialization or re-configuring */
    i2s_std_config_t std_cfg = {
        .clk_cfg  = I2S_STD_CLK_DEFAULT_CONFIG(SAMPLE_RATE_HZ),
        .slot_cfg = I2S_STD_MSB_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_32BIT, I2S_SLOT_MODE_STEREO),
        .gpio_cfg = {
            .mclk = I2S_GPIO_UNUSED,    // some codecs may require mclk signal, this example doesn't need it
            .bclk = EXAMPLE_STD_BCLK_IO1,
            .ws   = EXAMPLE_STD_WS_IO1,
            .dout = EXAMPLE_STD_DOUT_IO1,
            .din  = EXAMPLE_STD_DIN_IO1, // In duplex mode, bind output and input to a same gpio can loopback internally
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

    // // Create event queue for I2S data
    // read_queue = xQueueCreate(DATA_QUEUE_LEN, EXAMPLE_BUFF_SIZE);
    // write_queue = xQueueCreate(DATA_QUEUE_LEN, EXAMPLE_BUFF_SIZE);
}

#else

static void i2s_example_init_std_simplex(void)
{
    /* Setp 1: Determine the I2S channel configuration and allocate two channels one by one
     * The default configuration can be generated by the helper macro,
     * it only requires the I2S controller id and I2S role
     * The tx and rx channels here are registered on different I2S controller,
     * Except ESP32 and ESP32-S2, others allow to register two separate tx & rx channels on a same controller */
    i2s_chan_config_t tx_chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_AUTO, I2S_ROLE_MASTER);
    ESP_ERROR_CHECK(i2s_new_channel(&tx_chan_cfg, &tx_chan, NULL));
    i2s_chan_config_t rx_chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_AUTO, I2S_ROLE_MASTER);
    ESP_ERROR_CHECK(i2s_new_channel(&rx_chan_cfg, NULL, &rx_chan));

    /* Step 2: Setting the configurations of standard mode and initialize each channels one by one
     * The slot configuration and clock configuration can be generated by the macros
     * These two helper macros is defined in 'i2s_std.h' which can only be used in STD mode.
     * They can help to specify the slot and clock configurations for initialization or re-configuring */
    i2s_std_config_t tx_std_cfg = {
        .clk_cfg  = I2S_STD_CLK_DEFAULT_CONFIG(16000),
        .slot_cfg = I2S_STD_MSB_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_32BIT, I2S_SLOT_MODE_STEREO),
        .gpio_cfg = {
            .mclk = I2S_GPIO_UNUSED,    // some codecs may require mclk signal, this example doesn't need it
            .bclk = EXAMPLE_STD_BCLK_IO1,
            .ws   = EXAMPLE_STD_WS_IO1,
            .dout = EXAMPLE_STD_DOUT_IO1,
            .din  = EXAMPLE_STD_DIN_IO1,
            .invert_flags = {
                .mclk_inv = false,
                .bclk_inv = false,
                .ws_inv   = false,
            },
        },
    };
    ESP_ERROR_CHECK(i2s_channel_init_std_mode(tx_chan, &tx_std_cfg));

    i2s_std_config_t rx_std_cfg = {
        .clk_cfg  = I2S_STD_CLK_DEFAULT_CONFIG(44100),
        .slot_cfg = I2S_STD_MSB_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_MONO),
        .gpio_cfg = {
            .mclk = I2S_GPIO_UNUSED,    // some codecs may require mclk signal, this example doesn't need it
            .bclk = EXAMPLE_STD_BCLK_IO2,
            .ws   = EXAMPLE_STD_WS_IO2,
            .dout = EXAMPLE_STD_DOUT_IO2,
            .din  = EXAMPLE_STD_DIN_IO2,
            .invert_flags = {
                .mclk_inv = false,
                .bclk_inv = false,
                .ws_inv   = false,
            },
        },
    };
    /* Default is only receiving left slot in mono mode,
     * update to right here to show how to change the default configuration */
    rx_std_cfg.slot_cfg.slot_mask = I2S_STD_SLOT_RIGHT;
    ESP_ERROR_CHECK(i2s_channel_init_std_mode(rx_chan, &rx_std_cfg));
}
#endif

void app_main(void)
{
    // Create mutex before starting tasks
    mutex = xSemaphoreCreateMutex();

    // Create event queue for I2S data
    read_queue = xQueueCreate(DATA_QUEUE_LEN, EXAMPLE_BUFF_SIZE);
    write_queue = xQueueCreate(DATA_QUEUE_LEN, EXAMPLE_BUFF_SIZE);

    ESP_LOGW(TAG,"I2C Init\n");
    pcm1862_init(); 

    ESP_LOGW(TAG,"I2S Init\n");
    #if EXAMPLE_I2S_DUPLEX_MODE
        i2s_example_init_std_duplex();
    #else
        i2s_example_init_std_simplex();
    #endif

    /* Step 3: Create writing and reading task, enable and start the channels */
    xTaskCreate(i2s_example_read_task, "i2s_example_read_task", 4096, NULL, 5, NULL);
    // xTaskCreate(digital_filter_task, "digital_filter_task", 4096, NULL, 5, NULL);
    xTaskCreate(i2s_example_write_task, "i2s_example_write_task", 4096, NULL, 5, NULL);
   

    // IFX_PeakingFilter_Init(&peak_filt, SAMPLE_RATE_HZ_F);


    // while (1) {
      
    //     for(int i = 0; i < 14; i++){
    //         i2c_master_write_read_device(I2C_NUM_0, I2C_SLAVE_ADDR, &scan_regs[i], 1U, rx_data, sizeof(rx_data), pdMS_TO_TICKS(TIMEOUT_MS));
    //         vTaskDelay(pdMS_TO_TICKS(DELAY_MS));
    //         ESP_LOG_BUFFER_HEX(TAG, rx_data, sizeof(rx_data));
    //         // vTaskDelay(pdMS_TO_TICKS(DELAY_MS));
    //     }

    //     ESP_LOGI(TAG,"#########\n");

    //     // ESP_LOG_BUFFER_HEX(TAG, rx_data, sizeof(rx_data));
    //     // vTaskDelay(pdMS_TO_TICKS(DELAY_MS)); 
    // }
}