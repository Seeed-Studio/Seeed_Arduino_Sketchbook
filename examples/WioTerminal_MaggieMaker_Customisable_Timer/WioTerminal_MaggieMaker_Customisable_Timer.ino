// MaggieMaker: A Customisable Kitchen Timer
// Author: Jonathan Tan
// Jan 2021
// Written for Seeed Wio Terminal

#include <TFT_eSPI.h>
#include"Free_Fonts.h"

// EDIT THE FOLLOWING TO DECLARE BUTTON PINS FOR OTHER ARDUINO BOARDS
const auto clicker_button = WIO_5S_PRESS;
const auto audio_button = WIO_KEY_C;

// Initialise State Variables
int alarm_state = 1;
int clicker_state = 0;
bool skipped_timer = false;

TFT_eSPI tft;
TFT_eSprite ttext(&tft);
TFT_eSprite alarm_text(&tft);
TFT_eSprite step_number(&tft);
TFT_eSprite title_text(&tft);
TFT_eSprite notes1_text(&tft);
TFT_eSprite notes2_text(&tft);
TFT_eSprite notes3_text(&tft);
TFT_eSprite action_text(&tft);
TFT_eSprite sp(&tft);


void clear_timer() {
    tft.fillRect(0, 200, 400, 40, TFT_BLACK);
    action_text.deleteSprite();
}

void clear_all() {
    tft.fillRect(0, 50, 400, 200, TFT_BLACK);
    tft.fillRect(0, 0, 150, 20, TFT_BLACK);
}

void clear_sprites(String notes2, String notes3) {
    step_number.deleteSprite();
    title_text.deleteSprite();
    notes1_text.deleteSprite();
    action_text.deleteSprite();
    if (notes2.length() != 0) {
        notes2_text.deleteSprite();
    }
    if (notes3.length() != 0) {
        notes3_text.deleteSprite();
    }
}

void alarm() {
    for (int k=0; k<3; k++) {
        for (int i=0; i<3; i++) {
            analogWrite(WIO_BUZZER, 150);
            delay(80);
            analogWrite(WIO_BUZZER, 0);
            delay(80);
        }
        delay(600);
    }
}

void alarm_logo() {
      sp.createSprite(12,13);
      sp.fillTriangle(5,0,5,13,0,6,0xFFFF);
      sp.fillRect(0,4,2,5,0xFFFF);
      sp.drawFastVLine(8,5,3,0xFFFF);
      sp.drawFastVLine(11,5,3,0xFFFF);
      sp.drawFastVLine(10,3,2,0xFFFF);
      sp.drawFastVLine(10,8,2,0xFFFF);
      sp.drawLine(8,1,10,3,0xFFFF);
      sp.drawLine(8,11,10,9,0xFFFF);
      sp.drawPixel(7,4,0xFFFF);
      sp.drawPixel(7,8,0xFFFF);
      sp.pushSprite(300, 5);
}

void cover(int mode, String line1 = "Meet Your", String line2 = "Maggie Maker!") {
    tft.setFreeFont(FSSB18);
    tft.setTextDatum(MC_DATUM);

    if (mode == 0) {
        // Add Toggle Alarm Text
        alarm_text.createSprite(80,10);
        alarm_text.setCursor(0,0);
        alarm_text.setTextColor(TFT_LIGHTGREY, 0);
        alarm_text.drawString("Toggle Alarm", 0,0);
        alarm_text.pushSprite(5,5);
        
        // Add Title Text
        tft.drawString(line1, 160, 100);
        tft.drawString(line2, 160, 140);
        tft.setFreeFont(FSS9);
        tft.drawString("Click to Start", 160, 210);

        // Wait for Input and Allow for Alarm Toggle
        while (clicker_state == 0) {
            if (digitalRead(audio_button) == LOW) {
              if (alarm_state == 1) {
                  alarm_state = 0;
                  tft.fillRect(300, 0, 50, 50, TFT_BLACK);
                  delay(500);
              } else if (alarm_state == 0) {
                  alarm_state = 1;
                  alarm_logo();
                  delay(500);
              }
            }
            if (digitalRead(clicker_button) == LOW) {
              clicker_state = 1;
              delay(500);
              alarm_text.deleteSprite();
            }
        }
    } else if (mode == 1) {
        // Ending Text
        tft.drawString("Enjoy your Food!", 160, 120);
        tft.setFreeFont(FSS9);
        tft.drawString("Made by Jonathan Tan", 160, 210);
        while (clicker_state == 0) {
            if (digitalRead(clicker_button) == LOW) {
              clicker_state = 1;
              delay(500);
            }
        }
    }
    clicker_state = 0;
    clear_all();
}

void page(int step_no, String title, int duration, String notes1, String notes2 = "", String notes3 = "") {

    // Display the Text Instructions
    step_number.createSprite(200, 15);
    step_number.setCursor(0,0);
    step_number.setTextColor(TFT_LIGHTGREY, 0);
    step_number.setTextSize(2);
    step_number.printf("STEP %d", step_no);
    step_number.pushSprite(10,50);

    title_text.createSprite(320, 80);
    title_text.setCursor(0,0);
    title_text.setFreeFont(FSSB18);
    title_text.drawString(title, 0 ,0);
    title_text.pushSprite(8,75);

    notes1_text.createSprite(280, 20);
    notes1_text.setCursor(0,0);
    notes1_text.setFreeFont(FSS9);
    notes1_text.drawString(notes1, 0, 0);
    notes1_text.pushSprite(10,120);

    if (notes2.length() != 0) {
        notes2_text.createSprite(280, 20);
        notes2_text.setCursor(0,0);
        notes2_text.setFreeFont(FSS9);
        notes2_text.drawString(notes2, 0, 0);
        notes2_text.pushSprite(10,140);
    }

    if (notes3.length() != 0) {
        notes3_text.createSprite(280, 20);
        notes3_text.setCursor(0,0);
        notes3_text.setFreeFont(FSS9);
        notes3_text.drawString(notes3, 0, 0);
        notes3_text.pushSprite(10,160);
    }

    // Engage timer page if selected
    if (duration != 0) {
      
        // First let the user understand the instruction.
        while (clicker_state == 0) {
            action_text.createSprite(320, 30);
            action_text.setCursor(0,0);
            action_text.setFreeFont(FSS9);
            action_text.drawString("Start the timer when you're ready.", 0, 0);
            action_text.pushSprite(10,210);
            if (digitalRead(clicker_button) == LOW) clicker_state = 1;
        }
        clicker_state = 0;
        clear_timer();
        delay(300);

        // Timer starts running.
        ttext.createSprite(110,30);
        tft.drawRoundRect(10, 210, 200, 10, 4, TFT_WHITE);
        long start_millis = millis();
        int progress, t_min, t_sec;
        long seconds_elapsed = 0;
        long seconds_remain;
        
        while(seconds_elapsed<duration && clicker_state == 0) {
            seconds_elapsed = (millis() - start_millis)/1000;
            seconds_remain = duration - seconds_elapsed;
            t_min = seconds_remain/60;
            t_sec = seconds_remain%60;
      
            ttext.setCursor(0, 0);
            ttext.setTextColor(0xFFE0, 0); 
            ttext.setTextSize(3);
            ttext.printf("%02d:%02d", t_min, t_sec);
            ttext.pushSprite(220,200);
      
            progress = 200*seconds_elapsed/duration;
            tft.fillRoundRect(10, 210, progress, 10, 4, TFT_WHITE);
            if (digitalRead(clicker_button) == LOW) {
              clicker_state = 1;
              skipped_timer = true;
            }
        }
        clicker_state = 0;
        
        if (skipped_timer == false && alarm_state == 1) {
            alarm();
        } else if (skipped_timer == false && alarm_state == 0) {
            delay(2000);
        } else {
            skipped_timer = false;
            delay(500);
        }
        
        clear_timer();

        // Await user instruction to proceed.
        while (clicker_state == 0) {
            action_text.createSprite(320, 30);
            action_text.setCursor(0,0);
            action_text.setFreeFont(FSS9);
            action_text.drawString("Timer done! Click to continue.", 0, 0);
            action_text.pushSprite(10,210);

            if (digitalRead(clicker_button) == LOW){
                clicker_state = 1;
                delay(500);
            }
        }
        clicker_state = 0;
        clear_all();

    } else {

        // User will proceed when task is completed.
        while (clicker_state == 0) {
            action_text.createSprite(320, 30);
            action_text.setCursor(0,0);
            action_text.setFreeFont(FSS9);
            action_text.drawString("When you're ready, click to continue.", 0, 0);
            action_text.pushSprite(10,210);
            if (digitalRead(clicker_button) == LOW) {
              clicker_state = 1;
              delay(500);
            }
        }
        clicker_state = 0;
        clear_all();
    }

    clear_sprites(notes2, notes3);
    delay(100);

}


void setup() {
      pinMode(clicker_button, INPUT_PULLUP);
      pinMode(audio_button, INPUT_PULLUP);
      pinMode(WIO_BUZZER, OUTPUT);
  
      tft.init();
      tft.setRotation(3);
      tft.fillScreen(TFT_BLACK);
      alarm_logo();
}

void loop() {
      cover(0);
      page(1, "Prepare!", 0, "Get your favourite soup instant", "noodles and an egg.");
      page(2, "Boil the Water!", 0, "Heat pot at medium-high heat", "until water is bubbling.");
      page(3, "Noodles In!", 120, "Submerge all the way and", "separate slightly with chopsticks.");
      page(4, "Enter Soup Base!", 45, "Add however much you want,", "make sure it's mixed well!");
      page(5, "Add the Egg!", 60, "Remove the pot from heat and", "cover the egg with noodles.");
      page(6, "Say Cheese~", 0, "Add one slice of cheese", "on top of the noodles.");
      cover(1);
}
