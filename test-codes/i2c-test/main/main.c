// #include <freertos/FreeRTOS.h>
// #include <freertos/task.h>
#include "esp_log.h"
#include "driver/i2c.h"
#include <stdio.h>

#define I2C_SLAVE_ADDR  0x4A
#define TIMEOUT_MS      1000
#define DELAY_MS        1000

static const char *TAG = "I2C TEST";

void app_main()
{
    ESP_LOGI(TAG, "Testiik");
    i2c_config_t conf = {
            .mode = I2C_MODE_MASTER,
            .sda_io_num = 14,
            .scl_io_num = 13,
            .sda_pullup_en = GPIO_PULLUP_ENABLE,
            .scl_pullup_en = GPIO_PULLUP_ENABLE,
            .master.clk_speed = 100000,
        };

    i2c_param_config(I2C_NUM_0, &conf);



    ESP_ERROR_CHECK(i2c_driver_install(I2C_NUM_0, I2C_MODE_MASTER, 0, 0, 0));

    uint8_t buf[1] = {0x6};
    uint8_t rx_data[1] = {0};
    uint8_t regs[7] = {0x72,0x73,0x74,0x75,0x78,0x20,0x06};
    uint8_t ADCwake[2] = {0x20,0x11};
    uint8_t ADCinput_R[2] = {0x06,0x50};
    uint8_t ADCinput_L[2] = {0x07,0x50};


    i2c_master_write_read_device(I2C_NUM_0, I2C_SLAVE_ADDR, ADCwake, sizeof(ADCwake), rx_data, sizeof(rx_data), pdMS_TO_TICKS(TIMEOUT_MS)); // Set ADC to master mode, CLK autodetect {0x11}
    i2c_master_write_read_device(I2C_NUM_0, I2C_SLAVE_ADDR, ADCinput_R, sizeof(ADCinput_R), rx_data, sizeof(rx_data), pdMS_TO_TICKS(TIMEOUT_MS)); // Set ADC input channel Right differential {0x50}
    i2c_master_write_read_device(I2C_NUM_0, I2C_SLAVE_ADDR, ADCinput_L, sizeof(ADCinput_L), rx_data, sizeof(rx_data), pdMS_TO_TICKS(TIMEOUT_MS)); // Set ADC input channel Left differential {0x50}



    while (1) {
        // i2c_master_write_read_device(I2C_NUM_0, I2C_SLAVE_ADDR, buf, sizeof(buf), rx_data, sizeof(rx_data), pdMS_TO_TICKS(TIMEOUT_MS)); 
        // vTaskDelay(pdMS_TO_TICKS(DELAY_MS)); 
        
        i2c_master_write_read_device(I2C_NUM_0, I2C_SLAVE_ADDR, &regs[0], 1U, rx_data, sizeof(rx_data), pdMS_TO_TICKS(TIMEOUT_MS));
        vTaskDelay(pdMS_TO_TICKS(DELAY_MS)); 
        i2c_master_write_read_device(I2C_NUM_0, I2C_SLAVE_ADDR, &regs[1], 1U, rx_data, sizeof(rx_data), pdMS_TO_TICKS(TIMEOUT_MS));
        vTaskDelay(pdMS_TO_TICKS(DELAY_MS)); 
        i2c_master_write_read_device(I2C_NUM_0, I2C_SLAVE_ADDR, &regs[2], 1U, rx_data, sizeof(rx_data), pdMS_TO_TICKS(TIMEOUT_MS));
        vTaskDelay(pdMS_TO_TICKS(DELAY_MS)); 
        i2c_master_write_read_device(I2C_NUM_0, I2C_SLAVE_ADDR, &regs[3], 1U, rx_data, sizeof(rx_data), pdMS_TO_TICKS(TIMEOUT_MS));
        vTaskDelay(pdMS_TO_TICKS(DELAY_MS)); 
        i2c_master_write_read_device(I2C_NUM_0, I2C_SLAVE_ADDR, &regs[4], 1U, rx_data, sizeof(rx_data), pdMS_TO_TICKS(TIMEOUT_MS));
        vTaskDelay(pdMS_TO_TICKS(DELAY_MS)); 
        i2c_master_write_read_device(I2C_NUM_0, I2C_SLAVE_ADDR, &regs[5], 1U, rx_data, sizeof(rx_data), pdMS_TO_TICKS(TIMEOUT_MS));
        vTaskDelay(pdMS_TO_TICKS(DELAY_MS));
        i2c_master_write_read_device(I2C_NUM_0, I2C_SLAVE_ADDR, &regs[6], 1U, rx_data, sizeof(rx_data), pdMS_TO_TICKS(TIMEOUT_MS));
        vTaskDelay(pdMS_TO_TICKS(DELAY_MS));
        
        
        



        ESP_LOG_BUFFER_HEX(TAG, rx_data, sizeof(rx_data));
        vTaskDelay(pdMS_TO_TICKS(DELAY_MS)); 
    }
}






