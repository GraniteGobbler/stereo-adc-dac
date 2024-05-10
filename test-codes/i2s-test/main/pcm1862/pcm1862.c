#include "pcm1862.h"

    // static uint8_t registers[]= {
    //     // { 0x00, 0x00 },
    //     // { 0x01, 0x00 },
    //     // { 0x02, 0x00 },
    //     // { 0x03, 0x00 },
    //     // { 0x04, 0x00 },
    //     { 0x05, 0x86 },
    //     { 0x06, 0x50 },
    //     { 0x07, 0x50 },
    //     // { 0x08, 0x42 },
    //     // { 0x09, 0x42 },
    //     // { 0x0a, 0x00 },
    //     // { 0x0b, 0x44 },
    //     // { 0x0c, 0x00 },
    //     // { 0x0d, 0x00 },
    //     // { 0x0e, 0x00 },
    //     // { 0x0f, 0x00 },
    //     // { 0x10, 0x01 },
    //     // { 0x11, 0x20 },
    //     // { 0x12, 0x00 },
    //     // { 0x13, 0x00 },
    //     // { 0x14, 0x00 },
    //     // { 0x15, 0x00 },
    //     // { 0x16, 0x00 },
    //     // { 0x17, 0x00 },
    //     // { 0x18, 0x00 },
    //     // { 0x19, 0x00 },
    //     // { 0x1a, 0x00 },
    //     { 0x20, 0x11 },
    //     // { 0x21, 0x03 },
    //     // { 0x22, 0x00 },
    //     // { 0x23, 0x01 },
    //     // { 0x25, 0x07 },
    //     // { 0x26, 0x03 },
    //     // { 0x27, 0x3f },
    //     // { 0x28, 0x11 },
    //     // { 0x29, 0x01 },
    //     // { 0x2a, 0x01 },
    //     // { 0x2b, 0x08 },
    //     // { 0x2c, 0x00 },
    //     // { 0x2d, 0x00 },
    //     { 0x30, 0x00 },
    //     { 0x31, 0x00 },
    //     { 0x32, 0x00 },
    //     { 0x33, 0x01 },
    //     { 0x34, 0x00 },
    //     { 0x36, 0x01 },
    //     { 0x40, 0x80 },
    //     { 0x41, 0x7f },
    //     { 0x42, 0x00 },
    //     { 0x43, 0x80 },
    //     { 0x44, 0x7f },
    //     { 0x45, 0x00 },
    //     { 0x46, 0x80 },
    //     { 0x47, 0x7f },
    //     { 0x48, 0x00 },
    //     { 0x49, 0x80 },
    //     { 0x4a, 0x7f },
    //     { 0x4b, 0x00 },
    //     { 0x4c, 0x80 },
    //     { 0x4d, 0x7f },
    //     { 0x4e, 0x00 },
    //     { 0x4f, 0x80 },
    //     { 0x50, 0x7f },
    //     { 0x51, 0x00 },
    //     { 0x52, 0x80 },
    //     { 0x53, 0x7f },
    //     { 0x54, 0x00 },
    //     { 0x55, 0x80 },
    //     { 0x56, 0x7f },
    //     { 0x57, 0x00 },
    //     { 0x58, 0x80 },
    //     { 0x59, 0x00 },
    //     { 0x5a, 0x00 },
    //     { 0x60, 0x11 },
    //     { 0x61, 0x00 },
    //     { 0x62, 0x10 },
    //     { 0x70, 0x70 },
    //     { 0x71, 0x10 },
    //     { 0x72, 0x0f },
    //     // { 0x73, 0x03 },
    //     // { 0x74, 0x32 },
    //     // { 0x75, 0x00 },
    //     // { 0x76, 0x11 },
    //     // { 0x77, 0x44 },
    //     // { 0x78, 0x07 },
    //     // { 0x00, 0x01 },
    //     // { 0x01, 0x00 },
    //     // { 0x02, 0x2d },
    //     // { 0x03, 0x00 },
    //     // { 0x04, 0x00 },
    //     // { 0x05, 0x00 },
    //     // { 0x06, 0x00 },
    //     // { 0x07, 0x00 },
    //     // { 0x08, 0x00 },
    //     // { 0x09, 0x3a },
    //     // { 0x0a, 0x45 },
    //     // { 0x0b, 0x00 },
    //     // { 0x0c, 0x00 },
    //     // { 0x0d, 0x00 },
    //     // { 0x0e, 0x00 },
    //     // { 0x0f, 0x00 },
    // };
    
    // struct reg_struct {
    //     uint8_t address;
    //     uint8_t data;
        
    // };

    // struct reg_struct pcm1862_init[] = {
    //     // {0x00,0x00},
    //     // {0xFF,0x0A},
    //     {0x06,0x50},
    //     {0x07,0x50},
    //     // {0xFF,0x05},
    //     // {0x0B,0x44},
    //     // {0x0C,0x00},
    //     {0x20,0x11},
    //     // {0xFF,0x0A},
    //     // {0x28,0x03},
    //     // {0xFF,0x0A}
    // };

/*          Vars            */
static uint8_t rx_data[1] = {0};
static uint8_t buf[2] = {0,0};
static uint8_t scan_regs[14] = {0x72,0x73,0x74,0x75,0x78,0x20,
                                0x70,0x06,0x07,0x08,0x09,0x0B,0x01,0x02};
static uint8_t ADCwake[2] = {0x20,0x11};
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

    // for(int i = 0; i < 3; i++){
    //     i2c_master_write_to_device(I2C_NUM_0, I2C_SLAVE_ADDR, pcm1862_init[i].address, 1U, pdMS_TO_TICKS(TIMEOUT_MS));
    //     i2c_master_write_to_device(I2C_NUM_0, I2C_SLAVE_ADDR, pcm1862_init[i].data, 1U, pdMS_TO_TICKS(TIMEOUT_MS));
    // }
}