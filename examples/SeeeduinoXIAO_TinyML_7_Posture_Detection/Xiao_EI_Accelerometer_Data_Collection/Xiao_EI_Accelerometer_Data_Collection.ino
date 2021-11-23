// This example shows the 3 axis acceleration.
#include "LIS3DHTR.h"
#include <Wire.h>
LIS3DHTR<TwoWire> LIS; //IIC
#define WIRE Wire

#define CONVERT_G_TO_MS2    9.80665f
#define FREQUENCY_HZ        50
#define INTERVAL_MS         (1000 / (FREQUENCY_HZ + 1))

static unsigned long last_interval_ms = 0;

void setup() {
    Serial.begin(115200);
    while (!Serial) {};
    Serial.println("hi");
    //LIS.begin(WIRE); //IIC init dafault :0x18
    LIS.begin(WIRE, 0x23); //IIC init
    delay(100);
    //  LIS.setFullScaleRange(LIS3DHTR_RANGE_2G);
    //  LIS.setFullScaleRange(LIS3DHTR_RANGE_4G);
    //  LIS.setFullScaleRange(LIS3DHTR_RANGE_8G);
    //  LIS.setFullScaleRange(LIS3DHTR_RANGE_16G);
    //  LIS.setOutputDataRate(LIS3DHTR_DATARATE_1HZ);
    //  LIS.setOutputDataRate(LIS3DHTR_DATARATE_10HZ);
    //  LIS.setOutputDataRate(LIS3DHTR_DATARATE_25HZ);
    //  LIS.setOutputDataRate(LIS3DHTR_DATARATE_50HZ);
    //LIS.setOutputDataRate(LIS3DHTR_DATARATE_100HZ);
    //  LIS.setOutputDataRate(LIS3DHTR_DATARATE_200HZ);
    //  LIS.setOutputDataRate(LIS3DHTR_DATARATE_1_6KHZ);
    //  LIS.setOutputDataRate(LIS3DHTR_DATARATE_5KHZ);
    //LIS.setHighSolution(true); //High solution enable
    Serial.println(LIS.getDeviceID());
    if (!LIS.isConnection()) {
    Serial.println("LIS3DHTR didn't connect.");
    while (1);
    return;
    }
}
void loop() {

    float x, y, z;

    if (millis() > last_interval_ms + INTERVAL_MS) {
        last_interval_ms = millis();

        LIS.getAcceleration(&x, &y, &z);

        Serial.print(x * CONVERT_G_TO_MS2);
        Serial.print('\t');
        Serial.print(y * CONVERT_G_TO_MS2);
        Serial.print('\t');
        Serial.println(z * CONVERT_G_TO_MS2);
    }
}
