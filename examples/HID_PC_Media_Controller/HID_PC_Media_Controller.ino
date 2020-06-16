/*    
 * A demo for Consumer HID uses TinyUSB library to control PC Media.
 * Such as Volume Up, Volume Down, PC Mute, Media Player Play/Pause, Open Brower Home, 
 * Open Windows Media Player, Open Calculator,Open My Computor and etc.
 *   
 * Copyright (c) 2020 seeed technology co., ltd.  
 * Author      : weihong.cai (weihong.cai@seeed.cc)  
 * Create Time : June 2020
 * Change Log  : 
 *
 * The MIT License (MIT)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software istm
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS INcommInterface
 * THE SOFTWARE.
 * 
 * Get start:
 * 1. Download the Adafruit TinyUSB Library from: https://github.com/adafruit/Adafruit_TinyUSB_Arduino.
 *    And add it to your local arduino library.
 * 2. Arduino IDE tool-->Select the board: Seeeduino Wio Terminal. 
 * 3. Arduino IDE tool-->Select the USB Stack: TinyUSB. 
 * 4. Arduino IDE tool-->Select the COM port. 
 * 
 * If you have any questions, you can leave a message on the forum: https://forum.seeedstudio.com/t/seeeduionxiao/252299
 */

#include "Adafruit_TinyUSB.h"

//You can learn more regulation of Function Values from: USB HID Usage Tables 1.12(https://www.usb.org/sites/default/files/documents/hut1_12v2.pdf) 
/*--------------Function Values--------------------*/
#define Original_Value               0x00  
#define Volume_Up                    0x01   
#define Volume_Down                  0x02
#define PC_Mute                      0x04
#define Media_Player_Play_or_Pause   0x08  
#define Open_Brower_Home             0x10
#define Open_Windows_Media_Player    0x20
#define Open_Calculator              0x40
#define Open_My_Computor             0x80             
/*-------------------------------------------------*/

/* USB HID report descriptor. */
//You can learn more detail of USB HID report descriptor from: USB HID Usage Tables 1.12(https://www.usb.org/sites/default/files/documents/hut1_12v2.pdf) 
uint8_t const desc_hid_report[] =
{
  //The first  Byte: The command of purpose. 0xbbbbbbbb  D7~D4:bTag  D3~D2:bTyte  D1~D0:bSize
  //The second Byte: The usage ID. 
  0x05, 0x0c,         //USAGE_PAGE(Consumer Page)
  0x09, 0x01,         //USAGE(Consumer Control)
  0xa1, 0x01,         //COLLECTION(Application)
  0x85, 0x01,         //Report ID(1)
  
  0x09, 0xe9,         //USAGE(Volume Increment)
  0x09, 0xea,         //USAGE(Volume Decrement)
  0x09, 0xe2,         //USAGE(Mute)
  0x09, 0xcd,         //USAGE(Play/Pause)
  0x0a, 0x23, 0x02,   //USAGE(Open Brower Home)          (0x0223)（Little-endian）
  0x0a, 0x83, 0x01,   //USAGE(Open Windows Media Player) (0x0183)（Little-endian）
  0x0a, 0x92, 0x01,   //USAGE(Open Calculator)           (0x0192)（Little-endian）
  0x0a, 0x94, 0x01,   //USAGE(Open My Computor)          (0x0194)（Little-endian）
 
  0x95, 0x08,         //REPORT_COUNT(8)
  0x75, 0x01,         //REPORT_SIZE(1)
  0x81, 0x02,         //INPUT(Data,Var,Abs)

  0x15, 0x00,         //LOGICAL_MINIMUM(0)
  0x25, 0x01,         //LOGICAL_MAXIMUM(1)

  0xc0                //END_COLLECTION 
};

uint8_t   HID_report[1];

Adafruit_USBD_HID usb_hid;

// the setup function runs once when you press reset or power the board
void setup()
{
  /*Init Button*/
  //You can also set other button pin.
  pinMode(BUTTON_1, INPUT_PULLUP);
  pinMode(BUTTON_2, INPUT_PULLUP);
  pinMode(BUTTON_3, INPUT_PULLUP);

  /*Init USB Device*/
  usb_hid.enableOutEndpoint(true);
  usb_hid.setPollInterval(2);
  usb_hid.setReportDescriptor(desc_hid_report, sizeof(desc_hid_report));
  usb_hid.begin();

  /*Init serial port*/
  Serial.begin(115200);

  // wait until device mounted
  while( !USBDevice.mounted() ) 
  delay(1);

  Serial.println("The HID PC Media Controler Example.");
}

void loop()
{ 
  /*You can select other functions from the above function values*/
  
  // PC Mute
  if( 0 == digitalRead(BUTTON_1) )
  {
     delay(100);
     if( 0 == digitalRead(BUTTON_1) )
     {
       HID_report[0] = PC_Mute;    
       usb_hid.sendReport(1, HID_report, 1);
       //The function interface: sendReport(uint8_t report_id, void const *report, uint8_t len)
       delay(50);
       HID_report[0] = Original_Value;
       usb_hid.sendReport(1, HID_report, 1); 
       delay(100);
     }
  }
  
  // Volume Down
  if( 0 == digitalRead(BUTTON_2) )
  {  
     delay(100);
     if( 0 == digitalRead(BUTTON_2) )
     {
       HID_report[0] = Volume_Down;
       usb_hid.sendReport(1, HID_report, 1);
       //The function interface: sendReport(uint8_t report_id, void const *report, uint8_t len)
       delay(50);
       HID_report[0] = Original_Value;
       usb_hid.sendReport(1, HID_report, 1);
       delay(100);
     }
  }

  // Volume Up
  if( 0 == digitalRead(BUTTON_3) )
  { 
     delay(100);
     if( 0 == digitalRead(BUTTON_3) )
     {
       HID_report[0] = Volume_Up;
       usb_hid.sendReport(1, HID_report, 1);
       //The function interface: sendReport(uint8_t report_id, void const *report, uint8_t len)
       delay(50);
       HID_report[0] = Original_Value;
       usb_hid.sendReport(1, HID_report, 1);
       delay(100);
     }
  }
}
