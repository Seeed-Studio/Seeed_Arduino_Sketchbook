#include "TFT_eSPI.h"

#include "Free_Fonts.h" //include the header file
volatile byte state_a = LOW;
volatile byte state_b = LOW;
volatile byte state_c = LOW;
volatile byte do_break = 0;
TFT_eSPI tft;

void key_a_input()
{
    state_a = !state_a;
    do_break = 1;
}
void key_b_input()
{
    state_b = !state_b;
    do_break = 1;
}
void key_c_input()
{
    state_c = !state_c;
    do_break = 1;
}

void setup()
{
    Serial.begin(115200);
    //Serial1.begin(115200);
    pinMode(WIO_KEY_A, INPUT_PULLUP);

    pinMode(WIO_KEY_B, INPUT_PULLUP);

    pinMode(WIO_KEY_C, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(WIO_KEY_A), key_a_input, CHANGE);
    attachInterrupt(digitalPinToInterrupt(WIO_KEY_B), key_b_input, CHANGE);
    attachInterrupt(digitalPinToInterrupt(WIO_KEY_C), key_c_input, CHANGE);

    tft.begin();
    tft.setRotation(3);
    tft.fillScreen(TFT_BLACK); //Black background
    tft.setFreeFont(FSSBO12);  //select Free, Sans, Bold, Oblique, 12pt.
}

void loop()
{

    if (state_a == LOW)
    {
        uint32_t baud;

        uint32_t old_baud;
        while (digitalRead(WIO_KEY_A) == LOW)
            ;
        tft.drawString("Burn RTL8720 fw", 70, 80); //prints string at (70,80)
        state_a = 1;
        // initialize both serial ports:
        pinMode(PIN_SERIAL2_RX, OUTPUT);
        pinMode(RTL8720D_CHIP_PU, OUTPUT);
        digitalWrite(PIN_SERIAL2_RX, LOW);
        digitalWrite(RTL8720D_CHIP_PU, LOW);
        delay(500);
        digitalWrite(RTL8720D_CHIP_PU, HIGH);
        delay(500);
        pinMode(PIN_SERIAL2_RX, INPUT);
        Serial.beginWithoutDTR(115200);
        //  Serial.baud
        old_baud = Serial.baud();
        RTL8720D.begin(old_baud);
   
        delay(500);
        while (1)
        {
            baud = Serial.baud();
            if(baud != old_baud)
            {
               RTL8720D.begin(baud);
               old_baud = baud;
            }
            // read from port 1, send to port 0:

            if (Serial.available())
            {

                int inbyte = Serial.read();

                RTL8720D.write(inbyte);

                //Serial1.write(inbyte);
            }

            //   read from port 1, send to port 0:

            if (RTL8720D.available())
            {

                int inbyte = RTL8720D.read();

                Serial.write(inbyte);

                //Serial1.write(inbyte);
            }

            if (do_break == 1)
            {
                do_break = 0;
                break;
            }
        }
    }
    else if (state_b == LOW)
    {
        while (digitalRead(WIO_KEY_B) == LOW)
            ;
        tft.drawString("USB to Serial         ", 70, 80); //prints string at (70,80)
        state_b = 1;
        // put your setup code here, to run once:
        uint32_t baud;

        uint32_t old_baud;

        digitalWrite(RTL8720D_CHIP_PU, LOW);
        delay(500);
        digitalWrite(RTL8720D_CHIP_PU, HIGH);
        delay(500);
        
        Serial.begin(115200);

        baud = Serial.baud();

        old_baud = baud;

        RTL8720D.begin(baud);
        while (1)
        {

            // put your main code here, to run repeatedly:
            baud = Serial.baud();
            if (baud != old_baud)
            {
                RTL8720D.begin(baud);
                while (!Serial)
                    ;
                old_baud = baud;
                //     SerialUSB.println(baud);
            }
            if (Serial.available() > 0)
            {
                char c = Serial.read();
                RTL8720D.write(c);
            }
            if (RTL8720D.available() > 0)
            {
                char c = RTL8720D.read();
                Serial.write(c);
            }
            if (do_break == 1)
            {
                do_break = 0;
                break;
            }
        }
    }
    else if (state_c == LOW)
    {
        while (digitalRead(WIO_KEY_C) == LOW)
            ;
        tft.drawString("C Key pressed", 70, 80); //prints string at (70,80)
        state_c = 1;
         while (1)
        {
            if (do_break == 1)
            {
                do_break = 0;
                break;
            }  
        }

    }
}
