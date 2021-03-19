#include "gui.h"

#define HW_TIMER_INTERVAL_MS 5

TFT_eSPI tft = TFT_eSPI();

lv_obj_t *city_label;
lv_obj_t *rain_meter;
lv_obj_t *rain_static;
lv_obj_t *temp_label;
lv_obj_t *pressure_label;
lv_obj_t *humidity_label;
lv_obj_t *temp_value_label;
lv_obj_t *humid_value_label;
lv_obj_t *pres_value_label;

lv_obj_t *img1;
lv_obj_t *LogOutput;

extern const char* location;

const lv_img_dsc_t *weather_types[4] = {&cloudy, &rainy, &sunny, &foggy};

void Log(const char *format, ...) {
   static char print_buf[1024] = { 0 };

   va_list args;
   va_start(args, format);
   int r = vsnprintf(print_buf, sizeof(print_buf), format, args);
   va_end(args);

   if (r > 0) {
       Serial.write(print_buf);
   }
}

void LogTFT(const char *format, ...) {
   static char print_buf[1024] = { 0 };

   va_list args;
   va_start(args, format);
   int r = vsnprintf(print_buf, sizeof(print_buf), format, args);
   va_end(args);

   if (r > 0) {
       Serial.write(print_buf);
       lv_label_set_text(LogOutput, print_buf);
       lv_task_handler();
   }
}

void disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p)
{
  uint16_t c;

  tft.startWrite(); /* Start new TFT transaction */
  tft.setAddrWindow(area->x1, area->y1, (area->x2 - area->x1 + 1), (area->y2 - area->y1 + 1)); /* set the working window */
  for (int y = area->y1; y <= area->y2; y++) {
    for (int x = area->x1; x <= area->x2; x++) {
      c = color_p->full;
      tft.writeColor(c, 1);
      color_p++;
    }
  }
  tft.endWrite(); /* terminate TFT transaction */
  lv_disp_flush_ready(disp); /* tell lvgl that flushing is done */
}

void lv_tick_handler()
{
  lv_tick_inc(5);
}

bool setupLVGL() {
  
    TC.startTimer(HW_TIMER_INTERVAL_MS * 1000, lv_tick_handler);
    
    lv_init();
    tft.begin(); /* TFT init */
    tft.setRotation(3); /* Landscape orientation */
  
    lv_disp_buf_init(&disp_buf, buf, NULL, LV_HOR_RES_MAX * 10);
  
    /*Initialize the display*/
    lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = 320;
    disp_drv.ver_res = 240;
    disp_drv.flush_cb = disp_flush;
    disp_drv.buffer = &disp_buf;
    lv_disp_drv_register(&disp_drv);

    lv_theme_t * th = lv_theme_material_init(LV_COLOR_BLACK, LV_COLOR_WHITE, LV_THEME_MATERIAL_FLAG_DARK, &lv_font_montserrat_14, &lv_font_montserrat_16, &lv_font_montserrat_18, &lv_font_montserrat_22);
    lv_theme_set_act(th);

    img1 = lv_img_create(lv_scr_act(), NULL);
    city_label = lv_label_create(lv_scr_act(), NULL);
    rain_meter = lv_linemeter_create(lv_scr_act(), NULL);
    rain_static = lv_label_create(lv_scr_act(), NULL);
    temp_label = lv_label_create(lv_scr_act(), NULL);
    pressure_label = lv_label_create(lv_scr_act(), NULL);
    humidity_label = lv_label_create(lv_scr_act(), NULL);
    temp_value_label = lv_label_create(lv_scr_act(), NULL);
    humid_value_label = lv_label_create(lv_scr_act(), NULL);
    pres_value_label = lv_label_create(lv_scr_act(), NULL);

    static lv_style_t large_style;
    lv_style_init(&large_style);
    lv_style_set_text_font(&large_style, LV_STATE_DEFAULT, lv_theme_get_font_title());
    
    lv_img_set_src(img1, &cloudy);
    lv_obj_set_x(img1, 20);
    lv_obj_set_y(img1, 120);
    lv_obj_set_style_local_image_recolor_opa(img1, LV_IMG_PART_MAIN, LV_STATE_DEFAULT, 255);
    lv_obj_set_style_local_image_recolor(img1, LV_IMG_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);

    lv_obj_add_style(city_label, LV_LABEL_PART_MAIN, &large_style);
    lv_obj_set_x(city_label, 115);
    lv_obj_set_y(city_label, 10);
    lv_obj_set_height(city_label, 20);
    lv_obj_set_width(city_label, 50);
    lv_label_set_text(city_label, location);
        
    lv_obj_set_x(rain_meter, 15);
    lv_obj_set_y(rain_meter, 10);
    lv_obj_set_width(rain_meter, 75);
    lv_obj_set_height(rain_meter, 75);
    lv_linemeter_set_range(rain_meter, 0, 100);
    
    lv_obj_set_x(rain_static, 25);
    lv_obj_set_y(rain_static, 90);
    lv_obj_set_width(rain_static, 60);
    lv_obj_set_height(rain_static, 40);
    lv_label_set_text(rain_static, "% Rain");
    
    lv_obj_set_x(temp_label, 110);
    lv_obj_set_y(temp_label, 50);
    lv_label_set_text(temp_label, "Temp (C°)");
    
    lv_obj_set_x(pressure_label, 110);
    lv_obj_set_y(pressure_label, 100);
    lv_label_set_text(pressure_label, "Pressure (bar)");
       
    lv_obj_set_x(humidity_label, 110);
    lv_obj_set_y(humidity_label, 150);
    lv_label_set_text(humidity_label, "Humidity (%)");  

    lv_obj_add_style(temp_value_label, LV_LABEL_PART_MAIN, &large_style);
    lv_obj_set_x(temp_value_label, 240);
    lv_obj_set_y(temp_value_label, 50);

    lv_obj_add_style(pres_value_label, LV_LABEL_PART_MAIN, &large_style);    
    lv_obj_set_x(pres_value_label, 240);
    lv_obj_set_y(pres_value_label, 100);

    lv_obj_add_style(humid_value_label, LV_LABEL_PART_MAIN, &large_style);
    lv_obj_set_x(humid_value_label, 240);
    lv_obj_set_y(humid_value_label, 150);
    
    LogOutput = lv_label_create(lv_scr_act(), NULL);
    lv_label_set_long_mode(LogOutput, LV_LABEL_LONG_CROP);
    lv_label_set_recolor(LogOutput, true);
    lv_label_set_align(LogOutput, LV_LABEL_ALIGN_LEFT);
    lv_obj_set_width(LogOutput, 280);
    lv_obj_align(LogOutput, NULL, LV_ALIGN_IN_TOP_LEFT, 20, 200);
    lv_obj_set_style_local_bg_opa(LogOutput, LV_PAGE_PART_BG, LV_STATE_DEFAULT, 64);    
  
}

bool update_screen(float temp_value, float pres_value, float humid_value, float precip_value, uint8_t weather_type) {

  lv_linemeter_set_value(rain_meter, int8_t(precip_value));

  lv_label_set_text_fmt(temp_value_label, "%.1f", temp_value);
  lv_label_set_text_fmt(pres_value_label, "%d", int16_t(pres_value));
  lv_label_set_text_fmt(humid_value_label, "%.1f", humid_value);

  lv_img_set_src(img1, weather_types[weather_type]);  

  lv_task_handler();
  
}
