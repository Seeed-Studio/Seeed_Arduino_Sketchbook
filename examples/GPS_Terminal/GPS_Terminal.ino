#include <SoftwareSerial.h>
#include <TFT_eSPI.h>

static const int RXPin = 0, TXPin = 1;
static const uint32_t GPSBaud = 9600;

// The serial connection to the GPS device
SoftwareSerial ss(RXPin, TXPin);
 
TFT_eSPI tft;
TFT_eSprite spr = TFT_eSprite(&tft);  // Sprite 

void setup()
{
  Serial.begin(9600);
  ss.begin(GPSBaud);
  tft.begin();
  tft.setRotation(3);
  spr.createSprite(tft.width(),tft.height()); 
}
void loop()
{
    spr.fillSprite(TFT_BLACK);
    spr.setFreeFont(&FreeSansBoldOblique18pt7b); 
    spr.setTextColor(TFT_GREEN);
    spr.drawString("GPS RAW DATA", 20, 10 , 1);// Print the test text in the cust
    for(int8_t line_index = 0;line_index < 5 ; line_index++)
    {
    spr.drawLine(0, 50 + line_index, tft.width(), 50 + line_index, TFT_GREEN);
    }
    spr.setTextColor(TFT_WHITE);
    spr.setTextFont(2); 
    spr.setCursor(0,60);
    while(ss.available())                     // if date is coming from software serial port ==> data is coming from ss shield
    {              
        uint8_t reciver = ss.read();
        Serial.write(reciver);
        spr.write(reciver);
        int basetime = millis();
        while (!ss.available());
        int timecost = millis() - basetime;
        if(timecost > 709){
            break;
        } 
    }
    if (Serial.available())                 // if data is available on hardware serial port ==> data is coming from PC or notebook
        ss.write(Serial.read());        // write it to the ss shield
    spr.pushSprite(0, 0);
    Serial.print("----------------------------------------------------------\r\n");
}