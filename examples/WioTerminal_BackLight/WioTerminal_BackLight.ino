#include <TFT_eSPI.h>
#include "lcd_backlight.hpp"
#include <cstdint>

TFT_eSPI tft;
static LCDBackLight backLight;
void setup() {
    Serial.begin(115200);
    while(!Serial);

    tft.begin();
    tft.setRotation(3);

    tft.fillScreen(tft.color565(255, 0, 0));

    Serial.println("initializing backlight...");
    backLight.initialize();
}

static std::uint8_t brightness = 0;
void loop() {
    std::uint8_t maxBrightness = backLight.getMaxBrightness();
    brightness += 1;
    if( brightness > maxBrightness ) {
        brightness = 0;
    }
    backLight.setBrightness(brightness);
    delay(50);
}
