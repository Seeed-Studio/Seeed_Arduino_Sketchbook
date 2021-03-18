#ifndef WIO_TERMINAL_WEATHER_GET_HIST_WEATHER_H_
#define WIO_TERMINAL_WEATHER_GET_HIST_WEATHER_H_

#include <rpcWiFi.h>
#include <ArduinoJson.h> 
#include <CircularBuffer.h>
#include "resources.h"
#include "gui.h"

extern CircularBuffer<float, 72> stack;

bool getCurrent();
bool getHistorical();
DynamicJsonDocument get_data(char request[], uint8_t kb);
bool setupWIFI();

#endif
