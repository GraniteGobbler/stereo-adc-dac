#include "display_init.h"


void lv_tick_task(void *arg);
void _lvg_init(void *pvParameter);


SemaphoreHandle_t xGuiSemaphore = NULL;


void lvgl_init()
{
    xTaskCreatePinnedToCore(_lvg_init, "lvgl_init_task", 4096*2, NULL, 0, NULL, 1);
}

void _lvg_init(void *pvParameter) {

    (void) pvParameter;
    xGuiSemaphore = xSemaphoreCreateMutex();

    lv_init();

    /* Initialize SPI or I2C bus used by the drivers */
    lvgl_driver_init();

    lv_color_t* buf1 = heap_caps_malloc(DISP_BUF_SIZE * sizeof(lv_color_t), MALLOC_CAP_DMA);
    assert(buf1 != NULL);


    static lv_color_t *buf2 = NULL;

    static lv_disp_draw_buf_t disp_buf;

    uint32_t size_in_px = DISP_BUF_SIZE;

    

    /* Initialize the working buffer depending on the selected display.
     * NOTE: buf2 == NULL when using monochrome displays. */
    lv_disp_draw_buf_init(&disp_buf, buf1, buf2, size_in_px);

    lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.ver_res = SCREEN_VER_RES;
    disp_drv.hor_res = SCREEN_HOR_RES;
    
    // disp_drv.ver_res = 296;
    // disp_drv.hor_res = 128;
    disp_drv.flush_cb = disp_driver_flush;
    
    // ! Use of Rounder ?
    disp_drv.rounder_cb = disp_driver_rounder;


    disp_drv.draw_buf = &disp_buf;
    lv_disp_t *disp = lv_disp_drv_register(&disp_drv);
    // lv_disp_t *disp = lv_disp_drv_register(&disp_drv); //lvgl v8+

    

    /* Create and start a periodic timer interrupt to call lv_tick_inc */
    const esp_timer_create_args_t periodic_timer_args = {
        .callback = &lv_tick_task,
        .name = "periodic_gui"
    };
    
    esp_timer_handle_t periodic_timer = NULL; //lvgl v8+
    ESP_ERROR_CHECK(esp_timer_create(&periodic_timer_args, &periodic_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(periodic_timer, LV_TICK_PERIOD_MS * 1000));

    

    // xSemaphoreGive(guiTaskSemaphore);

    while (1) {
        /* Delay 1 tick (assumes FreeRTOS tick is 10ms */
        vTaskDelay(pdMS_TO_TICKS(10));

        /* Try to take the semaphore, call lvgl related function on success */
        if (pdTRUE == xSemaphoreTake(xGuiSemaphore, portMAX_DELAY)) {
            lv_task_handler();
            xSemaphoreGive(xGuiSemaphore);
       }
    }

    /* A task should NEVER return */
    free(buf1);
    #ifndef CONFIG_LV_TFT_DISPLAY_MONOCHROME
        free(buf2);
    #endif
        vTaskDelete(NULL);
}

void lv_tick_task(void *arg) {
    (void) arg;
    lv_tick_inc(LV_TICK_PERIOD_MS);
}