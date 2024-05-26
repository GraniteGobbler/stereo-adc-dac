#include "pcm1862.h"

/*          Vars            */
// static uint8_t rx_data[1] = {0};
static uint8_t buf[2] = {0,0};
// static uint8_t scan_regs[14] = {0x72,0x73,0x74,0x75,0x78,0x20,
                                // 0x70,0x06,0x07,0x08,0x09,0x0B,0x01,0x02};
static uint8_t ADCwake[2] = {0x20,0x11};    // Set master mode
static uint8_t ADC1_L[2] = {0x06,0x50};     // Enable differential 
static uint8_t ADC1_R[2] = {0x07,0x50};     // channels L/R on ADC1
static uint8_t ADC2_L[2] = {0x08,0x40};     // Disable ADC2 inputs
static uint8_t ADC2_R[2] = {0x09,0x40};     
static uint8_t ADC_data_format[2] = {0x0B,0x00};    // Set RX/TX to 32bit, I2S format     
static uint8_t ADC_gain_L[2] = {0x01,0x0C}; // 6 dB gain
static uint8_t ADC_gain_R[2] = {0x02,0x0C}; // 6 dB gain

/*          Funcs           */
void pcm1862_init(){
    
    i2c_config_t conf = {
            .mode = I2C_MODE_MASTER,
            .sda_io_num = 5,
            .scl_io_num = 4,
            .sda_pullup_en = GPIO_PULLUP_ENABLE,
            .scl_pullup_en = GPIO_PULLUP_ENABLE,
            .master.clk_speed = 100000,
        };

    i2c_param_config(I2C_NUM_0, &conf);

    ESP_ERROR_CHECK(i2c_driver_install(I2C_NUM_0, I2C_MODE_MASTER, 0, 0, 0));

    i2c_master_write_to_device(I2C_NUM_0, I2C_SLAVE_ADDR, ADC_gain_L, sizeof(buf), pdMS_TO_TICKS(TIMEOUT_MS));
    i2c_master_write_to_device(I2C_NUM_0, I2C_SLAVE_ADDR, ADC_gain_R, sizeof(buf), pdMS_TO_TICKS(TIMEOUT_MS));
    i2c_master_write_to_device(I2C_NUM_0, I2C_SLAVE_ADDR, ADCwake, sizeof(buf), pdMS_TO_TICKS(TIMEOUT_MS));
    i2c_master_write_to_device(I2C_NUM_0, I2C_SLAVE_ADDR, ADC1_L, sizeof(buf), pdMS_TO_TICKS(TIMEOUT_MS));
    i2c_master_write_to_device(I2C_NUM_0, I2C_SLAVE_ADDR, ADC1_R, sizeof(buf), pdMS_TO_TICKS(TIMEOUT_MS));
    i2c_master_write_to_device(I2C_NUM_0, I2C_SLAVE_ADDR, ADC2_L, sizeof(buf), pdMS_TO_TICKS(TIMEOUT_MS));
    i2c_master_write_to_device(I2C_NUM_0, I2C_SLAVE_ADDR, ADC2_R, sizeof(buf), pdMS_TO_TICKS(TIMEOUT_MS));
    i2c_master_write_to_device(I2C_NUM_0, I2C_SLAVE_ADDR, ADC_data_format, sizeof(buf), pdMS_TO_TICKS(TIMEOUT_MS));

}