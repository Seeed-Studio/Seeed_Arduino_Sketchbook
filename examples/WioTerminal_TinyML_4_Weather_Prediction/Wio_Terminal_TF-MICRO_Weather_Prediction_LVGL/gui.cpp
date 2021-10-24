#include "gui.h"

#define HW_TIMER_INTERVAL_US 5000

static lv_disp_draw_buf_t disp_buf;
static lv_color_t buf[LV_HOR_RES_MAX * 10];

TFT_eSPI tft = TFT_eSPI();

lv_obj_t *city_label;
lv_obj_t *rain_meter;
lv_meter_indicator_t *rain_indic;
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

/* Display flushing */
void disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p)
{
   uint32_t w = (area->x2 - area->x1 + 1);
   uint32_t h = (area->y2 - area->y1 + 1);

   tft.startWrite();
   tft.setAddrWindow(area->x1, area->y1, w, h);
   tft.pushColors((uint16_t*)&color_p->full, w * h,true);
   tft.endWrite();

   lv_disp_flush_ready( disp );
}

void lv_tick_handler()
{
  lv_tick_inc(5);
}

bool setupLVGL() {
  
    TimerTc3.initialize(HW_TIMER_INTERVAL_US * 1000);
    TimerTc3.attachInterrupt(lv_tick_handler);
    
    lv_init();
    tft.begin(); /* TFT init */
    tft.setRotation(3); /* Landscape orientation */
  
    lv_disp_draw_buf_init(&disp_buf, buf, NULL, LV_HOR_RES_MAX * 10);
    
    /*Initialize the display driver*/
    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = LV_HOR_RES_MAX;
    disp_drv.ver_res = LV_VER_RES_MAX;
    disp_drv.flush_cb = disp_flush;
    disp_drv.draw_buf = &disp_buf;
    lv_disp_drv_register(&disp_drv);

    lv_theme_t * th = lv_theme_default_init(NULL, lv_color_black(), lv_color_white(), LV_THEME_DEFAULT_DARK, &lv_font_montserrat_14);

    img1 = lv_img_create(lv_scr_act());
    city_label = lv_label_create(lv_scr_act());
    rain_meter = lv_meter_create(lv_scr_act());
    
    rain_static = lv_label_create(lv_scr_act());
    temp_label = lv_label_create(lv_scr_act());
    pressure_label = lv_label_create(lv_scr_act());
    humidity_label = lv_label_create(lv_scr_act());
    temp_value_label = lv_label_create(lv_scr_act());
    humid_value_label = lv_label_create(lv_scr_act());
    pres_value_label = lv_label_create(lv_scr_act());

    static lv_style_t large_style;
    lv_style_init(&large_style);
    lv_style_set_text_font(&large_style, &lv_font_montserrat_22);
    
    lv_img_set_src(img1, &cloudy);
    lv_obj_set_x(img1, 20);
    lv_obj_set_y(img1, 120);
    //lv_obj_set_style_local_image_recolor_opa(img1, LV_IMG_PART_MAIN, LV_STATE_DEFAULT, 255);
    //lv_obj_set_style_local_image_recolor(img1, LV_IMG_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);

    lv_obj_add_style(city_label, &large_style, 0);
    lv_obj_set_x(city_label, 115);
    lv_obj_set_y(city_label, 10);
    lv_obj_set_height(city_label, 20);
    lv_obj_set_width(city_label, 120);
    lv_label_set_text(city_label, location);

    lv_meter_scale_t * scale = lv_meter_add_scale(rain_meter);
    lv_meter_set_scale_range(rain_meter, scale, 0, 100, 270, 90);
    rain_indic = lv_meter_add_arc(rain_meter, scale, 10, lv_palette_main(LV_PALETTE_RED), 0);  
       
    lv_obj_set_x(rain_meter, 15);
    lv_obj_set_y(rain_meter, 10);
    lv_obj_set_width(rain_meter, 75);
    lv_obj_set_height(rain_meter, 75);
   
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

    lv_obj_add_style(temp_value_label, &large_style, 0);
    lv_obj_set_x(temp_value_label, 240);
    lv_obj_set_y(temp_value_label, 50);

    lv_obj_add_style(pres_value_label, &large_style, 0);    
    lv_obj_set_x(pres_value_label, 240);
    lv_obj_set_y(pres_value_label, 100);

    lv_obj_add_style(humid_value_label, &large_style, 0);
    lv_obj_set_x(humid_value_label, 240);
    lv_obj_set_y(humid_value_label, 150);
    
    LogOutput = lv_label_create(lv_scr_act());
    lv_label_set_long_mode(LogOutput, LV_LABEL_LONG_CLIP);
    lv_label_set_recolor(LogOutput, true);
    lv_obj_set_style_text_align(LogOutput, LV_TEXT_ALIGN_LEFT, 0);    
    lv_obj_set_width(LogOutput, 280);
    lv_obj_align(LogOutput, LV_ALIGN_TOP_LEFT, 20, 200);
}

bool update_screen(float temp_value, float pres_value, float humid_value, float precip_value, uint8_t weather_type) {

  lv_meter_set_indicator_end_value(rain_meter, rain_indic, int8_t(precip_value));
  
  lv_label_set_text_fmt(temp_value_label, "%.1f", temp_value);
  lv_label_set_text_fmt(pres_value_label, "%d", int16_t(pres_value));
  lv_label_set_text_fmt(humid_value_label, "%.1f", humid_value);

  lv_img_set_src(img1, weather_types[weather_type]);  

  lv_task_handler();
  
}
