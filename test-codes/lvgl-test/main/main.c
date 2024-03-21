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
static lv_point_t line_points[] = { {0, 40}, {180, 40}};

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

        // ui_init();
        /*------- Init screens -------*/
        ui_main_screen = lv_obj_create(NULL);
        lv_obj_set_size(ui_main_screen, SCREEN_HOR_RES, SCREEN_VER_RES);
        lv_obj_set_pos(ui_main_screen, 0, 0);

        // --------------- TEXT LABEL ---------------

        lv_obj_t * label1 = lv_label_create(lv_scr_act());
        lv_label_set_long_mode(label1, LV_LABEL_LONG_WRAP);     /*Break the long lines*/
        lv_label_set_text(label1, "ESP DSP");
        lv_obj_set_width(label1, 170);  /*Set smaller width to make the lines wrap*/
        lv_obj_set_style_text_align(label1, LV_TEXT_ALIGN_LEFT, 0);
        lv_obj_align(label1, LV_ALIGN_TOP_LEFT, 0, 0);
        lv_obj_set_style_text_color(label1, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
        lv_obj_set_style_text_font(label1, &lv_font_montserrat_36, LV_PART_MAIN);
         

        // --------------- LINE --------------- 
        /*Create style*/
        lv_style_init(&style_line);
        lv_style_set_line_width(&style_line, 2);
        lv_style_set_line_color(&style_line, lv_color_hex(0xFF0000));
        lv_style_set_line_rounded(&style_line, true);

        /*Create a line and apply the new style*/
        line1 = lv_line_create(lv_scr_act());
        lv_line_set_points(line1, line_points, 2);     /*Set the points*/
        lv_obj_add_style(line1, &style_line, 0);
        // lv_obj_center(line1);





        // /*Create a list*/
        // ui_main_list = lv_list_create(ui_main_screen);
        // lv_obj_set_size(ui_main_list, SCREEN_HOR_RES, SCREEN_VER_RES-160);
        // lv_obj_center(ui_main_list);

        
        

        // lv_disp_load_scr(ui_main_screen);

        lv_obj_set_style_bg_color(lv_scr_act() , lv_color_hex(0x000000), LV_PART_MAIN);
        lv_disp_load_scr(lv_scr_act());

        // menu_title1 = lv_label_create(lv_scr_act());
        // // lv_obj_add_style(menu_title1, &main_style, 0);
        // lv_label_set_text(menu_title1, "XX");
        // lv_obj_align(menu_title1, LV_ALIGN_TOP_LEFT, 0, 0);
        // lv_disp_load_scr(lv_scr_act());

        xSemaphoreGive(xGuiSemaphore);

        vTaskDelay(pdMS_TO_TICKS(1000));
    }

    int counter = 0;

    while (true) {
        ESP_LOGI("MAIN", "\n");
        // lv_obj_t *menu_title1 = lv_list_add_text(ui_main_list, "X");
        // lv_obj_add_style(menu_title1, &main_style, 0);

        // char formatted_time[20];
        // snprintf(formatted_time, 20, "%04d", counter);
        // lv_label_set_text(menu_title1, formatted_time);

        // // counter++;
        // lv_chart_set_next_value(chart, ser1, counter);

        // lv_obj_invalidate(lv_scr_act());
        vTaskDelay(pdMS_TO_TICKS(1000));



    }

}