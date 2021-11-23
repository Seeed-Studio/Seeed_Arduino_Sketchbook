# TinyML Course 4 IoT Weather Prediction (TensorFlow Micro)

To run test on static data you will need sufficiently new version of Tensorflow Lite for Microcontrollers. Read on how to get it in [Seeed Wiki](https://wiki.seeedstudio.com/Wio-Terminal-TinyML-TFLM-1/#install-the-arduino-tensorflow-lite-library).

To run LVGL example you need:

1) Install WiFi libraries as specified [here](https://wiki.seeedstudio.com/Wio-Terminal-Network-Overview/). Before flashing RTL8720 firmware, check if you already have latest version by uploading the following code

```C
#include "rpcWiFi.h"
 
void setup() {
    Serial.begin(115200);
    while(!Serial); // Wait to open Serial Monitor
    Serial.printf("RTL8720 Firmware Version: %s", rpc_system_version());
}
 
void loop() {
}
```
If you already have the latest code and libraries for WiFi, proceed to the next step.

2) Install [ArduinoJson](https://github.com/bblanchon/ArduinoJson) 6.18.5. You can find it by searching for "ArduinoJson" in Arduino IDE libraries mananger.

3) Install [CircularBuffer](https://github.com/rlogiacco/CircularBuffer) 1.3.3. You can find it by searching for "CircularBuffer" in Arduino IDE libraries mananger.

4) Install LVGL 8.0.2 following the instructions [here](https://docs.lvgl.io/8/). Wio Terminal screen resolution is (320,240) and color depth is 16 bit. Make sure you install this particular version of LVGL - since the framework is under active development, API changes rapidly.

Additionally in lv_conf.h file, make the following changes:

```line 30 #define LV_COLOR_16_SWAP   1```

```line 208 #define LV_SPRINTF_USE_FLOAT 1```

```C
line 265 and below
#define LV_FONT_MONTSERRAT_8     0
#define LV_FONT_MONTSERRAT_10    0
#define LV_FONT_MONTSERRAT_12    1
#define LV_FONT_MONTSERRAT_14    1
#define LV_FONT_MONTSERRAT_16    1
#define LV_FONT_MONTSERRAT_18    1
#define LV_FONT_MONTSERRAT_20    0
#define LV_FONT_MONTSERRAT_22    1
#define LV_FONT_MONTSERRAT_24    0
#define LV_FONT_MONTSERRAT_26    0
#define LV_FONT_MONTSERRAT_28    0
#define LV_FONT_MONTSERRAT_30    0
#define LV_FONT_MONTSERRAT_32    0
#define LV_FONT_MONTSERRAT_34    0
#define LV_FONT_MONTSERRAT_36    0
#define LV_FONT_MONTSERRAT_38    0
#define LV_FONT_MONTSERRAT_40    0
#define LV_FONT_MONTSERRAT_42    0
#define LV_FONT_MONTSERRAT_44    0
#define LV_FONT_MONTSERRAT_46    0
#define LV_FONT_MONTSERRAT_48    0
```

5) Install [Grove - Barometer Sensor (BME280) Library](https://github.com/Seeed-Studio/Grove_BME280). You can find it by searching for "Grove BME280" in Arduino IDE libraries mananger.

6) Make sure you are using 1.8.2 board definitions for WIo Terminal. That is important for Tensorflow Lite for Microcontrollers to compile properly.


