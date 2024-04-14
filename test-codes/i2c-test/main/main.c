// #include <freertos/FreeRTOS.h>
// #include <freertos/task.h>
#include "esp_log.h"
#include "driver/i2c.h"
#include <stdio.h>

#define I2C_SLAVE_ADDR  0x4A
#define TIMEOUT_MS      2000
#define DELAY_MS        2000

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
    uint8_t regs[8] = {0x72,0x73,0x74,0x75,0x78,0x20,0x06,0x70};
    uint8_t ADCwake[2] = {0x20,0x11};
    uint8_t ADCinput_R[2] = {0x06,0x50};
    uint8_t ADCinput_L[2] = {0x07,0x50};

    uint8_t registers[][2]= {
        { 0x00, 0x00 },
        { 0x01, 0x00 },
        { 0x02, 0x00 },
        { 0x03, 0x00 },
        { 0x04, 0x00 },
        { 0x05, 0x86 },
        { 0x06, 0x41 },
        { 0x07, 0x41 },
        { 0x08, 0x42 },
        { 0x09, 0x42 },
        { 0x0a, 0x00 },
        { 0x0b, 0x44 },
        { 0x0c, 0x00 },
        { 0x0d, 0x00 },
        { 0x0e, 0x00 },
        { 0x0f, 0x00 },
        { 0x10, 0x01 },
        { 0x11, 0x20 },
        { 0x12, 0x00 },
        { 0x13, 0x00 },
        { 0x14, 0x00 },
        { 0x15, 0x00 },
        { 0x16, 0x00 },
        { 0x17, 0x00 },
        { 0x18, 0x00 },
        { 0x19, 0x00 },
        { 0x1a, 0x00 },
        { 0x1b, 0x00 },
        { 0x1c, 0x00 },
        { 0x1d, 0x00 },
        { 0x1e, 0x00 },
        { 0x1f, 0x00 },
        { 0x20, 0x11 },
        { 0x21, 0x03 },
        { 0x22, 0x00 },
        { 0x23, 0x01 },
        { 0x24, 0x50 },
        { 0x25, 0x07 },
        { 0x26, 0x03 },
        { 0x27, 0x3f },
        { 0x28, 0x11 },
        { 0x29, 0x01 },
        { 0x2a, 0x01 },
        { 0x2b, 0x08 },
        { 0x2c, 0x00 },
        { 0x2d, 0x00 },
        { 0x2e, 0x00 },
        { 0x2f, 0x00 },
        { 0x30, 0x00 },
        { 0x31, 0x00 },
        { 0x32, 0x00 },
        { 0x33, 0x01 },
        { 0x34, 0x00 },
        { 0x35, 0x00 },
        { 0x36, 0x01 },
        { 0x37, 0x00 },
        { 0x38, 0x00 },
        { 0x39, 0x00 },
        { 0x3a, 0x00 },
        { 0x3b, 0x00 },
        { 0x3c, 0x00 },
        { 0x3d, 0x00 },
        { 0x3e, 0x00 },
        { 0x3f, 0x00 },
        { 0x40, 0x80 },
        { 0x41, 0x7f },
        { 0x42, 0x00 },
        { 0x43, 0x80 },
        { 0x44, 0x7f },
        { 0x45, 0x00 },
        { 0x46, 0x80 },
        { 0x47, 0x7f },
        { 0x48, 0x00 },
        { 0x49, 0x80 },
        { 0x4a, 0x7f },
        { 0x4b, 0x00 },
        { 0x4c, 0x80 },
        { 0x4d, 0x7f },
        { 0x4e, 0x00 },
        { 0x4f, 0x80 },
        { 0x50, 0x7f },
        { 0x51, 0x00 },
        { 0x52, 0x80 },
        { 0x53, 0x7f },
        { 0x54, 0x00 },
        { 0x55, 0x80 },
        { 0x56, 0x7f },
        { 0x57, 0x00 },
        { 0x58, 0x80 },
        { 0x59, 0x00 },
        { 0x5a, 0x00 },
        { 0x5b, 0x00 },
        { 0x5c, 0x00 },
        { 0x5d, 0x00 },
        { 0x5e, 0x00 },
        { 0x5f, 0x00 },
        { 0x60, 0x11 },
        { 0x61, 0x00 },
        { 0x62, 0x10 },
        { 0x63, 0x00 },
        { 0x64, 0x00 },
        { 0x65, 0x00 },
        { 0x66, 0x00 },
        { 0x67, 0x00 },
        { 0x68, 0x00 },
        { 0x69, 0x00 },
        { 0x6a, 0x00 },
        { 0x6b, 0x00 },
        { 0x6c, 0x00 },
        { 0x6d, 0x00 },
        { 0x6e, 0x00 },
        { 0x6f, 0x00 },
        { 0x70, 0x70 },
        { 0x71, 0x10 },
        { 0x72, 0x0f },
        { 0x73, 0x03 },
        { 0x74, 0x32 },
        { 0x75, 0x00 },
        { 0x76, 0x11 },
        { 0x77, 0x44 },
        { 0x78, 0x07 },
        { 0x79, 0x00 },
        { 0x7a, 0x00 },
        { 0x7b, 0x00 },
        { 0x7c, 0x00 },
        { 0x7d, 0x00 },
        { 0x7e, 0x00 },
        { 0x7f, 0x00 },
        { 0x00, 0x01 },
        { 0x01, 0x00 },
        { 0x02, 0x2d },
        { 0x03, 0x00 },
        { 0x04, 0x00 },
        { 0x05, 0x00 },
        { 0x06, 0x00 },
        { 0x07, 0x00 },
        { 0x08, 0x00 },
        { 0x09, 0x3a },
        { 0x0a, 0x45 },
        { 0x0b, 0x00 },
        { 0x0c, 0x00 },
        { 0x0d, 0x00 },
        { 0x0e, 0x00 },
        { 0x0f, 0x00 },
    };
    

    

    // i2c_master_write_read_device(I2C_NUM_0, I2C_SLAVE_ADDR, ADCwake, sizeof(ADCwake), rx_data, sizeof(rx_data), pdMS_TO_TICKS(TIMEOUT_MS)); // Set ADC to master mode, CLK autodetect {0x11}
    // i2c_master_write_read_device(I2C_NUM_0, I2C_SLAVE_ADDR, ADCinput_R, sizeof(ADCinput_R), rx_data, sizeof(rx_data), pdMS_TO_TICKS(TIMEOUT_MS)); // Set ADC input channel Right differential {0x50}
    // i2c_master_write_read_device(I2C_NUM_0, I2C_SLAVE_ADDR, ADCinput_L, sizeof(ADCinput_L), rx_data, sizeof(rx_data), pdMS_TO_TICKS(TIMEOUT_MS)); // Set ADC input channel Left differential {0x50} 

    for(int i = 0; i <= 144; i++){
        i2c_master_write_read_device(I2C_NUM_0, I2C_SLAVE_ADDR, &registers[i][0], 1U, rx_data, sizeof(rx_data), pdMS_TO_TICKS(TIMEOUT_MS));
        vTaskDelay(pdMS_TO_TICKS(100));

    }

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
        i2c_master_write_read_device(I2C_NUM_0, I2C_SLAVE_ADDR, &regs[7], 1U, rx_data, sizeof(rx_data), pdMS_TO_TICKS(TIMEOUT_MS));
        vTaskDelay(pdMS_TO_TICKS(DELAY_MS));
        
        



        ESP_LOG_BUFFER_HEX(TAG, rx_data, sizeof(rx_data));
        vTaskDelay(pdMS_TO_TICKS(DELAY_MS)); 
    }
}






