#include "ATSerial.h"
#include "Protocol.h"
#include "KCT202.h"
#include <Wire.h>
#include "rgb_lcd.h"
#include <TimerTC3.h>
#include "Adafruit_NeoPixel.h"
#include <Servo.h>
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

char name_table[][8]={"None","Hansen","None","None","Junjie","None","None","None","None","None"};

rgb_lcd lcd;

const int colorR = 255;
const int colorG = 0;
const int colorB = 0;

#define OFF 0
#define GREEN 1
#define RED 2

// Which pin on the Arduino is connected to the NeoPixels?
// On a Trinket or Gemma we suggest changing this to 1
#define PIN            1

// How many NeoPixels are attached to the Arduino?
#define NUMPIXELS      20

// When we setup the NeoPixel library, we tell it how many pixels, and which pin to use to send signals.
// Note that for older NeoPixel strips you might need to change the third parameter--see the strandtest
// example for more information on possible values.
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);
uint8_t dot_num;

Servo myservo;  // create servo object to control a servo
const int servo_pin = 0;  // analog pin used to connect the potentiometer
void display_update(rgb_lcd* lcddev,char* str)
{
  if(!dot_num)lcddev->clear();
  dot_num++;
  if(dot_num > 5)
  {
    dot_num = 0;
  }
  lcddev->print(str);
}
void led_ring_enable(uint8_t led_ring_color)
{
  for (int i = 0; i < NUMPIXELS; i++) {
    // pixels.Color takes RGB values, from 0,0,0 up to 255,255,255
    if(GREEN == led_ring_color){
      pixels.setPixelColor(i, pixels.Color(0, 150, 0)); // Moderately bright green color.
    }
    else if(RED == led_ring_color){
      pixels.setPixelColor(i, pixels.Color(150, 0, 0)); // Moderately bright red color.
    }
    else
    {
      pixels.setPixelColor(i, pixels.Color(0, 0, 0)); 
    }
    
    pixels.show(); // This sends the updated pixel color to the hardware.
  }
}
void open_door()
{
  myservo.write(90);                  // sets the servo position according to the scaled value
  delay(2000);
  myservo.write(0);                  // sets the servo position according to the scaled value
  // digitalWrite(servo_pin, HIGH);
  // delay(2000);
  // digitalWrite(servo_pin, LOW);
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
  // set up servo
    myservo.attach(servo_pin);  // attaches the servo on pin 0 to the servo object
  // End of trinket special code
    pixels.setBrightness(100);
    pixels.begin(); // This initializes the NeoPixel library.
}

uint16_t finger_num;
uint8_t led_ring_color;
void loop() {

  kct202.autoVerifyFingerPrint(CHECK_ALL_FINGER_TEMP,
                            LED_OFF_AFTER_GET_GRAGH | PRETREATMENT_GRAGH | NOT_RET_FOR_EVERY_STEP);
  //The first param is the finger-print ID to check.
  //if set 0xffff,indicates that search for all the finger-print templates and try to match.
  debug.println(" ");
  debug.println("Please put your finger on the touchpad.");
  debug.println("To verify your finger print.");
  debug.println(" ");
  debug.println(" ");
  debug.println(" "); 
  sprintf(buffer, ".");
  led_ring_color = OFF;
  if (0 == kct202.getVerifyResponAndparse(finger_num)) {
      debug.println("Verify ok!");
      debug.print("Your finger temp id = ");
      debug.println(finger_num, HEX);
      sprintf (buffer, "welcome %s", name_table[finger_num]);
      dot_num = 0;
      display_update(&lcd,buffer);
      TimerTc3.stop();
      led_ring_enable(GREEN);
      open_door();
      dot_num = 0;
      sprintf(buffer, ".");
      display_update(&lcd,buffer);
      TimerTc3.start();
  }
  else
  {
      sprintf (buffer, "cannot recognize");
      dot_num = 0;
      display_update(&lcd,buffer);
      TimerTc3.stop();
      led_ring_enable(RED);
      delay(1000);
      dot_num = 0;
      sprintf(buffer, ".");
      display_update(&lcd,buffer);
      TimerTc3.start();
  }
  
}
void timerIsr()
{    
  display_update(&lcd,buffer);
  led_ring_enable(led_ring_color);
}
