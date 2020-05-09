#include "ATSerial.h"
#include "Protocol.h"
#include "KCT202.h"
#include <Wire.h>
#include "rgb_lcd.h"
#include <TimerTC3.h>
#include "Adafruit_NeoPixel.h"
#ifdef __AVR__
    #include <avr/power.h>
#endif

#if defined(ARDUINO_ARCH_AVR)
    #define debug  Serial
    SoftwareSerial uart(2, 3);
    FingerPrint_KCT202<SoftwareSerial, HardwareSerial> kct202;
#elif defined(ARDUINO_ARCH_SAM)
    #define debug  SerialUSB
    #define uart Serial
    FingerPrint_KCT202<Uart, Serial_> kct202;
#elif defined(ARDUINO_ARCH_SAMD)
    #define debug SerialUSB
    #define uart  Serial1
    FingerPrint_KCT202<Uart,Serial_> kct202;
#else
    #define debug  Serial
    SoftwareSerial uart(2, 3);
    FingerPrint_KCT202<SoftwareSerial, HardwareSerial> kct202;
#endif

const int RealyPin = 0;

char buffer [20];

char name_table[][8]={"None","Hansen","None","None","Junjie"};

rgb_lcd lcd;

const int colorR = 255;
const int colorG = 0;
const int colorB = 0;

// Which pin on the Arduino is connected to the NeoPixels?
// On a Trinket or Gemma we suggest changing this to 1
#define PIN            1

// How many NeoPixels are attached to the Arduino?
#define NUMPIXELS      20

// When we setup the NeoPixel library, we tell it how many pixels, and which pin to use to send signals.
// Note that for older NeoPixel strips you might need to change the third parameter--see the strandtest
// example for more information on possible values.
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

void display_update(rgb_lcd* lcddev,char* str)
{
  lcddev->clear();
  lcddev->print(str);
}
void led_ring_enable(bool enable)
{
  for (int i = 0; i < NUMPIXELS; i++) {
    // pixels.Color takes RGB values, from 0,0,0 up to 255,255,255
    if(enable){
      pixels.setPixelColor(i, pixels.Color(0, 150, 0)); // Moderately bright green color.
    }
    else{
      pixels.setPixelColor(i, pixels.Color(0, 0, 0)); // Moderately bright green color.
    }
    pixels.show(); // This sends the updated pixel color to the hardware.
  }
}
void open_door()
{
  digitalWrite(RealyPin, HIGH);
  delay(2000);
  digitalWrite(RealyPin, LOW);
}

void setup() {
  // setup fingerprint sensor
    debug.begin(115200);
    kct202.begin(uart, debug);
  // set up the LCD's number of columns and rows:
    lcd.begin(16, 2); 
  // set up the TimerTc3 
    TimerTc3.initialize(1000000);
    TimerTc3.attachInterrupt(timerIsr);
  // set up Realy Pin
    pinMode(RealyPin, OUTPUT);
    digitalWrite(RealyPin, LOW);
    // End of trinket special code
    pixels.setBrightness(100);
    pixels.begin(); // This initializes the NeoPixel library.
}

uint16_t finger_num;

void loop() {
  //The first param is the finger-print ID to check.
  //if set 0xffff,indicates that search for all the finger-print templates and try to match.
  kct202.autoVerifyFingerPrint(CHECK_ALL_FINGER_TEMP,
                            LED_OFF_AFTER_GET_GRAGH | PRETREATMENT_GRAGH | NOT_RET_FOR_EVERY_STEP);
  debug.println(" ");
  debug.println("Please put your finger on the touchpad.");
  debug.println("To verify your finger print.");
  debug.println(" ");
  debug.println(" ");
  debug.println(" ");
  sprintf(buffer, "Put finger");
  led_ring_enable(false);
  if (0 == kct202.getVerifyResponAndparse(finger_num)) {
      debug.println("Verify ok!");
      debug.print("Your finger temp id = ");
      debug.println(finger_num, HEX);
      sprintf (buffer, "welcome %s", name_table[finger_num]);
      display_update(&lcd,buffer);
      led_ring_enable(true);
      open_door();
  }
  delay(1000);
}
void timerIsr()
{    
  display_update(&lcd,buffer);
}
