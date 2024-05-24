/*
 * SPDX-FileCopyrightText: 2021-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2s_std.h"
#include "driver/gpio.h"
#include "esp_check.h"
#include "sdkconfig.h"
#include "./pcm1862/pcm1862.c"
#include "./ParamEQ/ParamEQ.h"

static const char *TAG = "I2S TEST";
static const char err_reason[][30] = {"input param is invalid",
                                      "operation timeout"
                                     };

#define I2S_DUPLEX_MODE         CONFIG_USE_DUPLEX

#define BCLK        GPIO_NUM_15 // I2S bit clock io number
#define WS          GPIO_NUM_16   // I2S word select io number
#define DOUT        GPIO_NUM_18 // I2S data out io number
#define DIN         GPIO_NUM_17  // I2S data in io number
#define DA_SMUTE    GPIO_NUM_46
#define DA_PWR_DOWN GPIO_NUM_9
#define AMP_ENABLE  GPIO_NUM_10

#define SAMPLE_RATE_HZ          96000
#define SAMPLE_RATE_HZ_F        96000.0f

#define BUFF_SIZE               256

// #define UINT32_TO_FLOAT                 1.0f/(65536.0f)
// #define FLOAT_TO_UINT32                 65536.0f

static i2s_chan_handle_t                tx_chan;        // I2S tx channel handler
static i2s_chan_handle_t                rx_chan;        // I2S rx channel handler
static ParamEQ                peak_filt_r;    // Peaking Filter struct
static ParamEQ                peak_filt_l;    // Peaking Filter struct


// Default Filter Config
float volume                =   1.0f;
float fc                    =   1000.0f;
float B                     =   10.0f;
float g                     =   1.0f;





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





static void i2s_process(void *args)
{
    int32_t *filtIN_buf = malloc(BUFF_SIZE);
    if (!filtIN_buf) {
        ESP_LOGE(TAG, "[echo] No memory for filterIn data buffer");
        abort();
    }
    int32_t *filtOUT_buf = malloc(BUFF_SIZE);
    if (!filtOUT_buf) {
        ESP_LOGE(TAG, "[echo] No memory for filterOut data buffer");
        abort();
    }

    static float leftIn, rightIn;
    static float leftProcessed, rightProcessed;
    static int32_t leftOut, rightOut;

    esp_err_t ret = ESP_OK;
    size_t bytes_read = 0;
    size_t bytes_write = 0;


    ESP_LOGI(TAG, "[echo] Echo start");

    while (1) {
        memset(filtIN_buf, 0, BUFF_SIZE);
        memset(filtOUT_buf, 0, BUFF_SIZE);

        /* Read I2S data */
        ret = i2s_channel_read(rx_chan, filtIN_buf, BUFF_SIZE, &bytes_read, 1000);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "[echo] i2s read failed, %s", err_reason[ret == ESP_ERR_TIMEOUT]);
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
            ESP_LOGE(TAG, "[echo] i2s write failed, %s", err_reason[ret == ESP_ERR_TIMEOUT]);
            abort();
        }
        if (bytes_read != bytes_write) {
            ESP_LOGW(TAG, "[echo] %d bytes read but only %d bytes are written", bytes_read, bytes_write);
        }

    }
    vTaskDelete(NULL);
}






void app_main(void)
{
    ESP_LOGW(TAG,"GPIO init\n");
    gpio_set_direction(DA_SMUTE, GPIO_MODE_OUTPUT);
    gpio_set_direction(DA_PWR_DOWN, GPIO_MODE_OUTPUT);
    gpio_set_direction(AMP_ENABLE, GPIO_MODE_OUTPUT);

    gpio_set_level(DA_SMUTE, 0); // DA_SMUTE - L
    gpio_set_level(DA_PWR_DOWN, 1); // DA_PWR_DOWN - H
    gpio_set_level(AMP_ENABLE, 1); // AMP_ENABLE - H


    ESP_LOGW(TAG,"I2C Init\n");
    pcm1862_init();


    ESP_LOGW(TAG,"I2S Init\n");
    /* Initialize i2s peripheral */
    if (i2s_init_std_duplex() != ESP_OK) {
        ESP_LOGE(TAG, "i2s driver init failed");
        abort();
    } else {
        ESP_LOGI(TAG, "i2s driver init success");
    }

    ESP_LOGW(TAG,"ParamFilter Init\n");
    ParamEQ_Init(&peak_filt_r, SAMPLE_RATE_HZ_F);
    ParamEQ_Init(&peak_filt_l, SAMPLE_RATE_HZ_F);
    ParamEQ_SetParameters(&peak_filt_r, fc, B, g);
    ParamEQ_SetParameters(&peak_filt_l, fc, B, g);


    xTaskCreate(i2s_process, "i2s_process", 8192, NULL, 5, NULL);

}
