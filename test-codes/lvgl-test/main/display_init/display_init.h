#pragma once

#include "lvgl_helpers.h"
#include "lvgl.h"

#include "driver/spi_master.h"
#include "freertos/semphr.h"
#include "esp_timer.h"

#define LV_TICK_PERIOD_MS 1


// 480x320
#define SCREEN_HOR_RES 480
#define SCREEN_VER_RES 320


extern SemaphoreHandle_t xGuiSemaphore;


void lvgl_init();


