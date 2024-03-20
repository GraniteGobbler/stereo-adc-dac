#ifdef __cplusplus
extern "C"

#endif


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
#include "benchmark/lv_demo_benchmark.h"


void app_main() {

    vTaskDelay(pdMS_TO_TICKS(3000));

    lvgl_init();

    vTaskDelay(pdMS_TO_TICKS(2000));

    lv_demo_benchmark();


}