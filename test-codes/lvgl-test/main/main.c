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

#include "lvgl.h"
#include "display_init/display_init.h"
#include "../ui.c"





static lv_style_t main_style;
static lv_style_t style_line;
static lv_point_t line_points[] = { {5, 55}, {200, 55}};

lv_obj_t *ui_main_screen;
lv_obj_t *ui_main_list;

lv_obj_t *menu_title1;

lv_obj_t * chart;
lv_chart_series_t * ser1;

lv_obj_t * line1;




void app_main() {

    vTaskDelay(pdMS_TO_TICKS(3000));

    lvgl_init();

    vTaskDelay(pdMS_TO_TICKS(2000));
    

    if (pdTRUE == xSemaphoreTake(xGuiSemaphore, portMAX_DELAY)) {

        ui_init();


        xSemaphoreGive(xGuiSemaphore);

        vTaskDelay(pdMS_TO_TICKS(1000));
    }

    int counter = 0;

    while (true) {
        ESP_LOGI("MAIN", "\n");

        vTaskDelay(pdMS_TO_TICKS(1000));



    }

}