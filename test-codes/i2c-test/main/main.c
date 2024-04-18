#include <./pcm1862/pcm1862.c>


static const char *TAG = "I2C TEST";

void app_main()
{
    ESP_LOGI(TAG, "Testiik");
    pcm1862_init();    

    while (1) {
      
        for(int i = 0; i < 14; i++){
            i2c_master_write_read_device(I2C_NUM_0, I2C_SLAVE_ADDR, &scan_regs[i], 1U, rx_data, sizeof(rx_data), pdMS_TO_TICKS(TIMEOUT_MS));
            vTaskDelay(pdMS_TO_TICKS(DELAY_MS));
            ESP_LOG_BUFFER_HEX(TAG, rx_data, sizeof(rx_data));
            // vTaskDelay(pdMS_TO_TICKS(DELAY_MS));
        }

        ESP_LOGI(TAG,"#########\n");

        // ESP_LOG_BUFFER_HEX(TAG, rx_data, sizeof(rx_data));
        // vTaskDelay(pdMS_TO_TICKS(DELAY_MS)); 
    }
}






