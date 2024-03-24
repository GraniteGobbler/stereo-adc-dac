/*------- Init screens -------*/
        ui_main_screen = lv_obj_create(NULL);
        lv_obj_set_size(ui_main_screen, SCREEN_HOR_RES, SCREEN_VER_RES);
        lv_obj_set_pos(ui_main_screen, 0, 0);

        // --------------- MENU --------------- 
        /*Create a menu object*/
        lv_obj_t * menu = lv_menu_create(lv_scr_act());
        lv_obj_set_size(menu, lv_disp_get_hor_res(NULL), lv_disp_get_ver_res(NULL));
        lv_obj_center(menu);

        lv_obj_t * cont;
        lv_obj_t * label;
        lv_obj_t * label_logo;

        /*Create a sub page*/
        lv_obj_t * sub_page = lv_menu_page_create(menu, NULL);
       
        //* --------------- Menu setup --------------- 
        cont = lv_menu_cont_create(sub_page);
        label = lv_label_create(cont);
        lv_label_set_text(label, "Hello, I am hiding here");

        /*Create a main page*/
        lv_obj_t * main_page = lv_menu_page_create(menu, NULL);

        // --------------- TEXT LABEL ---------------
        cont = lv_menu_cont_create(main_page);
        label_logo = lv_label_create(cont);
        lv_label_set_long_mode(label_logo, LV_LABEL_LONG_WRAP);     /*Break the long lines*/
        lv_label_set_text(label_logo, "ESP DSP");
        lv_obj_set_width(label_logo, 170);  /*Set smaller width to make the lines wrap*/
        lv_obj_set_style_text_align(label_logo, LV_TEXT_ALIGN_LEFT, 0);
        lv_obj_align(label_logo, LV_ALIGN_TOP_LEFT, 0, 0);
        lv_obj_set_style_text_color(label_logo, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
        lv_obj_set_style_text_font(label_logo, &lv_font_montserrat_36, LV_PART_MAIN);
         

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


        // --------------- MENU LABELS --------------- 
        cont = lv_menu_cont_create(main_page);
        label = lv_label_create(cont);
        lv_label_set_text(label, "Item 1");
        lv_obj_set_style_text_color(label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);


        cont = lv_menu_cont_create(main_page);
        label = lv_label_create(cont);
        lv_label_set_text(label, "Item 2");
        lv_obj_set_style_text_color(label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);

        cont = lv_menu_cont_create(main_page);
        label = lv_label_create(cont);
        lv_label_set_text(label, "Item 3 (Click me!)");
        lv_obj_set_style_text_color(label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
        lv_menu_set_load_page_event(menu, cont, sub_page);


        /* Load main menu*/ 
        lv_menu_set_page(menu, main_page);


                
        

        // lv_disp_load_scr(ui_main_screen);

        lv_obj_set_style_bg_color(menu, lv_color_hex(0x000000), LV_PART_MAIN);
        lv_disp_load_scr(lv_scr_act());