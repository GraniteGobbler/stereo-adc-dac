#include <stdio.h>
#include "esp_log.h"
#include "driver/i2c.h"

#define I2C_SLAVE_ADDR    0x40
#define TIMEOUT_MS        1000
#define DELAY_MS        1000

void PMIC_handle();

// ##########################

// #include "PMIC.h"

static const char *TAG = "I2C TEST";

void PMIC_handle() {
  ESP_LOGI(TAG, "Testiik");

    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = 14,
        .scl_io_num = 13,
        .sda_pullup_en = GPIO_PULLUP_DISABLE,
        .scl_pullup_en = GPIO_PULLUP_DISABLE,
        .master.clk_speed = 100000,
    };
    i2c_param_config(I2C_NUM_0, &conf);

    ESP_ERROR_CHECK(i2c_driver_install(I2C_NUM_0, I2C_MODE_MASTER, 0, 0, 0));

  uint8_t buf[1] = {0x6};
  uint8_t rx_data[2] = {0};

    while (1) {
        i2c_master_write_read_device(I2C_NUM_0, I2C_SLAVE_ADDR, buf, sizeof(buf), rx_data, sizeof(rx_data), TIMEOUT_MS/portTICK_PERIOD_MS);
        ESP_LOG_BUFFER_HEX(TAG, rx_data, sizeof(rx_data));
        vTaskDelay(DELAY_MS/portTICK_PERIOD_MS);
    }
}
