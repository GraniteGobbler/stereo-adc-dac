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
#include "./IFX_PeakingFilter/IFX_PeakingFilter.h"
#include "./RC_Filter/RC_Filter.h"

static const char *TAG = "I2S TEST";
static const char err_reason[][30] = {"input param is invalid",
                                      "operation timeout"
                                     };

#define EXAMPLE_I2S_DUPLEX_MODE         CONFIG_USE_DUPLEX

#define EXAMPLE_STD_BCLK_IO1 GPIO_NUM_15 // I2S bit clock io number
#define EXAMPLE_STD_WS_IO1 GPIO_NUM_16   // I2S word select io number
#define EXAMPLE_STD_DOUT_IO1 GPIO_NUM_18 // I2S data out io number
#define EXAMPLE_STD_DIN_IO1 GPIO_NUM_17  // I2S data in io number

#define SAMPLE_RATE_HZ                  96000
#define SAMPLE_RATE_HZ_F                96000.0f

#define EXAMPLE_BUFF_SIZE               256

// #define UINT32_TO_FLOAT                 1.0f/(65536.0f)
// #define FLOAT_TO_UINT32                 65536.0f

static i2s_chan_handle_t                tx_chan;        // I2S tx channel handler
static i2s_chan_handle_t                rx_chan;        // I2S rx channel handler
static IFX_PeakingFilter                peak_filt;      // Peaking Filter struct
static RC_Filter                        lpf;      // Peaking Filter struct

float outVolume             =   1.0;
float fc                    =   10000.0;
float B                     =   100.0; 
float g                     =   0.0001;


static esp_err_t i2s_example_init_std_duplex(void)
{
    i2s_chan_config_t chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_AUTO, I2S_ROLE_SLAVE);
    chan_cfg.auto_clear = true; // Auto clear the legacy data in the DMA buffer
    ESP_ERROR_CHECK(i2s_new_channel(&chan_cfg, &tx_chan, &rx_chan));

    i2s_std_config_t std_cfg = {
        .clk_cfg  = I2S_STD_CLK_DEFAULT_CONFIG(SAMPLE_RATE_HZ),
        .slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_32BIT, I2S_SLOT_MODE_STEREO),
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
    /* Enable the TX channel */
    ESP_ERROR_CHECK(i2s_channel_enable(tx_chan));
    /* Enable the RX channel */
    ESP_ERROR_CHECK(i2s_channel_enable(rx_chan));

    return ESP_OK;
    
}


static void i2s_echo(void *args)
{
    int32_t *filtIN_buf = malloc(EXAMPLE_BUFF_SIZE);
    if (!filtIN_buf) {
        ESP_LOGE(TAG, "[echo] No memory for filterIn data buffer");
        abort();
    }
    int32_t *filtOUT_buf = malloc(EXAMPLE_BUFF_SIZE);
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
        memset(filtIN_buf, 0, EXAMPLE_BUFF_SIZE);
        memset(filtOUT_buf, 0, EXAMPLE_BUFF_SIZE);

        /* Read sample data from mic */
        ret = i2s_channel_read(rx_chan, filtIN_buf, EXAMPLE_BUFF_SIZE, &bytes_read, 1000);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "[echo] i2s read failed, %s", err_reason[ret == ESP_ERR_TIMEOUT]);
            abort();
        }

        // filtOUT_buf = filtIN_buf;

        for (int32_t n = 0; n < (EXAMPLE_BUFF_SIZE) - 1; n += 2){
            
            /* // Convert uint_32 to float     UINT32_TO_FLOAT *  */
            leftIn = (float)filtIN_buf[n];            
            rightIn = (float)filtIN_buf[n+1];
            
            leftProcessed = IFX_PeakingFilter_Update(&peak_filt, leftIn);
            // rightProcessed = IFX_PeakingFilter_Update(&peak_filt, rightIn);
            rightProcessed = rightIn;
            // leftProcessed = RC_Filter_Update(&lpf, leftIn);
            // rightProcessed = RC_Filter_Update(&lpf, rightIn);
            
            /* // Convert float to uint_32      FLOAT_TO_UINT32 *  */
            leftOut = (int32_t)(leftProcessed);    
            rightOut = (int32_t)(rightProcessed);

            /* Set output buffer samples */
            filtOUT_buf[n]   = leftOut;
            filtOUT_buf[n+1] = rightOut;

            // printf("%lu", filtIN_buf[n]);
            // printf("  ");

        }

        // printf("\n");

        /* Write sample data to earphone */
        ret = i2s_channel_write(tx_chan, filtOUT_buf, EXAMPLE_BUFF_SIZE, &bytes_write, 1000);
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
    ESP_LOGW(TAG,"I2C Init\n");
    pcm1862_init(); 

    ESP_LOGW(TAG,"I2S Init\n");
    /* Initialize i2s peripheral */
    if (i2s_example_init_std_duplex() != ESP_OK) {
        ESP_LOGE(TAG, "i2s driver init failed");
        abort();
    } else {
        ESP_LOGI(TAG, "i2s driver init success");
    }


    IFX_PeakingFilter_Init(&peak_filt, SAMPLE_RATE_HZ_F);
    IFX_PeakingFilter_SetParameters(&peak_filt, fc, B, g);
    // RC_Filter_Init(&lpf, fc, SAMPLE_RATE_HZ_F);


    /* Echo the sound from MIC in echo mode */
    xTaskCreate(i2s_echo, "i2s_echo", 8192, NULL, 5, NULL);

}
