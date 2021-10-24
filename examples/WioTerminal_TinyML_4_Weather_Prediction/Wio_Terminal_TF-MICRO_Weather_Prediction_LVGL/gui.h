#ifndef WIO_TERMINAL_WEATHER_GUI_H_
#define WIO_TERMINAL_WEATHER_GUI_H_

#include <lvgl.h>
#include "resources.h"
#include <TFT_eSPI.h>
#include <stdio.h>
#include <stdarg.h>
#include <TimerTC3.h>



bool setupLVGL();
bool update_screen(float temp_value, float pres_value, float humid_value, float precip_value, uint8_t weather_type);
void disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p);
void lv_tick_handler();
void Log(const char *format, ...);
void LogTFT(const char *format, ...);

#endif
