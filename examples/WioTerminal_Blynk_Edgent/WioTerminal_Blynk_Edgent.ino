/**
 * @file       Edgent_Wio_Terminal.ino
 * @author     Blynk Inc.
 * @modified   Dmitry Maslov (Seeed Studio)
 * @license    This project is released under the MIT License (MIT)
 * @copyright  Copyright (c) 2021 Blynk Inc.
 * @date       May 2021
 * @brief
 *
 */

// Fill-in information from your Blynk Template here
#define BLYNK_TEMPLATE_ID ""
#define BLYNK_DEVICE_NAME ""

#define BLYNK_FIRMWARE_VERSION        "0.1.0"

#define BLYNK_PRINT Serial
//#define BLYNK_DEBUG

#define APP_DEBUG

#include "BlynkEdgent.h"

void sendData() {


  uint16_t noise = analogRead(WIO_MIC);
  uint16_t light = analogRead(WIO_LIGHT);

  String str = String(noise);
  Blynk.virtualWrite(V0, str);
  Serial.println(str);
  str = String(light);
  Blynk.virtualWrite(V1, str);
  Serial.println(str);  
}

void setup()
{

  Serial.begin(115200);
  delay(100);
  
#ifdef APP_DEBUG 
  while (!Serial) {delay(10);}
#endif

  BlynkEdgent.begin();
  timer.setInterval(1000L, sendData);
}

void loop() {
  BlynkEdgent.run();
}
