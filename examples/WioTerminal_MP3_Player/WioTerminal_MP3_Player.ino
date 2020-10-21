#include "KT403A_Player.h"
#include"TFT_eSPI.h"
#include "Seeed_FS.h" //Including SD card library
#include"RawImage.h"  //Including image processing library

TFT_eSPI tft;
#ifdef ARDUINO_SAMD_VARIANT_COMPLIANCE
    #define COMSerial Serial1
    #define ShowSerial SerialUSB

    KT403A<Uart> Mp3Player;
#endif

int menu = 0, p_menu = 4;
int plmode = 0, p_plmode = 5;
int p_status = 0;
int idx_folder = 1, idx_file = 1;
String mode_S[] = {"Loop_All", "Loop_Folder", "Play_Root" , "Play MP3", "Play_File"};
int Eq = 0;
String Eq_S[] = {"NORMAL", "POP", "ROCK" , "JAZZ", "CLASSIC", "BASS"};
EQUALIZER Eq_e[] = {NORMAL, POP, ROCK , JAZZ, CLASSIC, BASS};

void disp_title() {  
  tft.fillScreen(TFT_BLACK);
  
  if (menu == 0) {
    drawImage<uint8_t>("wio-chan8.bmp", 21, 32);
    tft.setTextColor(TFT_YELLOW);
    tft.drawString("MP3 Player",105, 5);
    tft.drawString("Play : ",5, 35);
    tft.drawString(mode_S[plmode], 89, 35);
    tft.drawString("Equalizer : ",5, 65);
    tft.drawString(Eq_S[Eq], 145, 65);
    //tft.fillTriangle(140,100,180,120,140,140,TFT_YELLOW);
  }
  
  if (menu == 1) {
    tft.drawString("Setting", 117, 5);
    tft.drawString("Play : ", 5, 37);
    tft.drawString(mode_S[plmode], 89, 37);
        
    tft.drawString("Equalizer : ", 5, 120);
    tft.drawString(Eq_S[Eq], 149, 120);

    if( (plmode == 1) or (plmode == 4) ) {
       tft.drawString("Folder# : ", 29, 60);
       tft.drawString(String(idx_folder), 150, 60);
    }
    if( (plmode == 2) or (plmode == 3) or (plmode == 4) ) {
       tft.drawString("File# : ", 52, 85);
       tft.drawString(String(idx_file), 150, 85);
    }
    
  }   //  end of menu 1
}   //  endo of disp_title()

void set_play() {
    int set_item = 0;
    int dummy = 0;
    while(menu == 1) {
      if (menu != p_menu)  disp_title();
      p_menu = menu;
      //play mode
      if(set_item == 0) {   // Play mode setting
        dummy = 1;
        tft.drawFastHLine(89,55,180,TFT_WHITE);
        while (dummy){
          if (digitalRead(WIO_5S_RIGHT) == LOW) {
            plmode++;
            if(plmode > 4) plmode = 0;
            disp_title();
            tft.drawFastHLine(89,55,180,TFT_WHITE);
            while(digitalRead(WIO_5S_RIGHT) == LOW){}
          }
          if (digitalRead(WIO_5S_LEFT) == LOW) {
            plmode--;
            if(plmode < 0) plmode = 4;
            disp_title();
            tft.drawFastHLine(89,55,180,TFT_WHITE);
            while(digitalRead(WIO_5S_LEFT) == LOW){}
          }
          if (digitalRead(WIO_5S_DOWN) == LOW) {
            tft.drawFastHLine(89,55,180,TFT_BLACK);
            if( (plmode == 1) or (plmode == 4) ) {
              set_item = 1;
              dummy = 0;
            }
            else if( (plmode == 2) or (plmode == 3) ) {
              set_item = 2;
              dummy = 0;
            }
            else {
              set_item = 3;
              dummy = 0;
            }
            while(digitalRead(WIO_5S_DOWN) == LOW){}
          }
          if (digitalRead(WIO_5S_UP) == LOW) {
            tft.drawFastHLine(89,55,180,TFT_BLACK);
            set_item = 3;
            dummy = 0;
            while(digitalRead(WIO_5S_UP) == LOW){}
          }
          if (digitalRead(WIO_KEY_C) == LOW) {
            menu = 0;
            dummy = 0;
            while(digitalRead(WIO_5S_UP) == LOW){}
          }
        }   //end of while dummy
     }  //end of set item 0
    
      if(set_item == 1) {   // Folder setting
        dummy = 1;
        tft.drawFastHLine(150,76,36,TFT_WHITE);
        while (dummy){
          if (digitalRead(WIO_5S_RIGHT) == LOW) {
            idx_folder++;
            if(idx_folder > 99) idx_folder = 99;
            disp_title();
            tft.drawFastHLine(150,76,36,TFT_WHITE);
            while(digitalRead(WIO_5S_RIGHT) == LOW){}
          }
          if (digitalRead(WIO_5S_LEFT) == LOW) {
            idx_folder--;
            if(idx_folder < 1) idx_folder = 1;
            disp_title();
            tft.drawFastHLine(150,76,36,TFT_WHITE);
            while(digitalRead(WIO_5S_LEFT) == LOW){}
          }
          if (digitalRead(WIO_5S_DOWN) == LOW) {
            tft.drawFastHLine(150,76,36,TFT_BLACK);
            if (plmode == 1)  {
              set_item = 3;
              dummy = 0;
            }
            else {
              set_item = 2;
              dummy = 0;
            }
            while(digitalRead(WIO_5S_DOWN) == LOW){}
          }
          if (digitalRead(WIO_5S_UP) == LOW) {
            tft.drawFastHLine(150,76,36,TFT_BLACK);
            set_item = 0;
            dummy = 0;
            while(digitalRead(WIO_5S_UP) == LOW){}
          }
          if (digitalRead(WIO_KEY_C) == LOW) {
            menu = 0;
            dummy = 0;
            while(digitalRead(WIO_5S_UP) == LOW){}
          }
        }   //end of while dummy
     }  //end of set item 1

      if(set_item == 2) {   // File setting
        dummy = 1;
        tft.drawFastHLine(150,101,36,TFT_WHITE);
        while (dummy){
          if (digitalRead(WIO_5S_RIGHT) == LOW) {
            idx_file++;
            if(idx_folder > 255) idx_folder = 255;
            disp_title();
            tft.drawFastHLine(150,101,36,TFT_WHITE);
            while(digitalRead(WIO_5S_RIGHT) == LOW){}
          }
          if (digitalRead(WIO_5S_LEFT) == LOW) {
            idx_file--;
            if(idx_folder < 1) idx_folder = 1;
            disp_title();
            tft.drawFastHLine(150,101,36,TFT_WHITE);
            while(digitalRead(WIO_5S_LEFT) == LOW){}
          }
          if (digitalRead(WIO_5S_DOWN) == LOW) {
            tft.drawFastHLine(150,101,36,TFT_BLACK);
              set_item = 3;
              dummy = 0;
            while(digitalRead(WIO_5S_DOWN) == LOW){}
          }
          if (digitalRead(WIO_5S_UP) == LOW) {
            tft.drawFastHLine(150,101,36,TFT_BLACK);
           if( (plmode == 2) or (plmode == 3) ) {
             set_item = 0;
             dummy = 0;
           }
           else {
             set_item = 1;
             dummy = 0;
           }
            while(digitalRead(WIO_5S_UP) == LOW){}
          }
          if (digitalRead(WIO_KEY_C) == LOW) {
            menu = 0;
            dummy = 0;
            while(digitalRead(WIO_5S_UP) == LOW){}
          }
        }   //end of while dummy
     }  //end of set item 2

      if(set_item == 3) {   // Equalizer setting
        dummy = 1;
        tft.drawFastHLine(149,136,84,TFT_WHITE);
        while (dummy){
          if (digitalRead(WIO_5S_RIGHT) == LOW) {
            Eq++;
            if(Eq > 5) Eq = 5;
            disp_title();
            tft.drawFastHLine(149,136,84,TFT_WHITE);
            while(digitalRead(WIO_5S_RIGHT) == LOW){}
          }
          if (digitalRead(WIO_5S_LEFT) == LOW) {
            Eq--;
            if(Eq < 0) Eq = 0;
            disp_title();
            tft.drawFastHLine(149,136,84,TFT_WHITE);
            while(digitalRead(WIO_5S_LEFT) == LOW){}
          }
          if (digitalRead(WIO_5S_DOWN) == LOW) {
            tft.drawFastHLine(149,136,84,TFT_BLACK);
              set_item = 0;
              dummy = 0;
            while(digitalRead(WIO_5S_DOWN) == LOW){}
          }
          if (digitalRead(WIO_5S_UP) == LOW) {
            tft.drawFastHLine(149,136,84,TFT_BLACK);
           if( (plmode == 2) or (plmode == 3) or (plmode == 4) ) {
             set_item = 2;
             dummy = 0;
           }
           else if (plmode == 1){
             set_item = 1;
             dummy = 0;
           }
           else {
             set_item = 0;
             dummy = 0;
           }
            while(digitalRead(WIO_5S_UP) == LOW){}
          }
          if (digitalRead(WIO_KEY_C) == LOW) {
            menu = 0;
            dummy = 0;
            while(digitalRead(WIO_5S_UP) == LOW){}
          }
        }   //end of while dummy
     }  //end of set item 3

    }   //end of menu1 
    disp_title();
}   //end of set_play

void setup() {
/*  
    //Initialise SD card
    if (!SD.begin(SDCARD_SS_PIN, SDCARD_SPI)) {
        while (1);
    }
*/  
    tft.begin();
    tft.setRotation(3);
    tft.fillScreen(TFT_BLACK); 
    tft.setTextSize(2);

    //ShowSerial.begin(9600);
    COMSerial.begin(9600);
    while (!COMSerial);
    Mp3Player.init(COMSerial);

    pinMode(WIO_KEY_A, INPUT_PULLUP);
    pinMode(WIO_KEY_B, INPUT_PULLUP);
    pinMode(WIO_KEY_C, INPUT_PULLUP);
    
    pinMode(WIO_5S_UP, INPUT_PULLUP);
    pinMode(WIO_5S_DOWN, INPUT_PULLUP);
    pinMode(WIO_5S_LEFT, INPUT_PULLUP);
    pinMode(WIO_5S_RIGHT, INPUT_PULLUP);
    pinMode(WIO_5S_PRESS, INPUT_PULLUP);

}

void loop() {
  if (menu != p_menu) {
    disp_title();
  }
  
  if (menu == 0) {
    
             if (digitalRead(WIO_5S_PRESS) == LOW) {
                ShowSerial.println("Press 5s PRESS");
                Mp3Player.pause_or_play();
                while(digitalRead(WIO_5S_PRESS) == LOW){}
             }  
             if (digitalRead(WIO_KEY_A) == LOW) {
                plmode = 0;
                disp_title();
                Mp3Player.setEqualizer(Eq_e[Eq]);
                Mp3Player.loop(1);
                while(digitalRead(WIO_KEY_A) == LOW){}
             }
             if (digitalRead(WIO_KEY_C) == LOW) {
                menu = 1;
                while(digitalRead(WIO_KEY_C) == LOW){}
             }
             if (digitalRead(WIO_KEY_B) == LOW) {
                Mp3Player.play();
                while(digitalRead(WIO_KEY_B) == LOW){}
             }
            if (digitalRead(WIO_5S_RIGHT) == LOW) {
                Mp3Player.next();
                while(digitalRead(WIO_5S_RIGHT) == LOW){}
            }
            if (digitalRead(WIO_5S_LEFT) == LOW) {
                Mp3Player.previous();
                while(digitalRead(WIO_5S_LEFT) == LOW){}
            }
             if (digitalRead(WIO_5S_UP) == LOW) {
                Mp3Player.volumeUp();
                while(digitalRead(WIO_5S_UP) == LOW){}
            }
            if (digitalRead(WIO_5S_DOWN) == LOW) {
                Mp3Player.volumeDown();
                while(digitalRead(WIO_5S_DOWN) == LOW){}
            }
  }
  else if (menu == 1) {
    p_menu = 0;
    set_play();
    switch (plmode) {
      case 0:   // Loop All
        Mp3Player.setEqualizer(Eq_e[Eq]);
        Mp3Player.loop(1);
        break;
      case 1:   //  Loop Folder
        Mp3Player.setEqualizer(Eq_e[Eq]);
        Mp3Player.loopFolder(idx_folder);
        break;
      case 2:   //  Play index folder (Root)
        Mp3Player.setEqualizer(Eq_e[Eq]);
        Mp3Player.playSongIndex(idx_file);
        break;
      case 3:   //  Play index folder (Root)
        Mp3Player.setEqualizer(Eq_e[Eq]);
        Mp3Player.playSongMP3(idx_file);
        break;
      case 4:   //  Play index folder (Root)
        Mp3Player.setEqualizer(Eq_e[Eq]);
        Mp3Player.playSongSpecify(idx_folder,idx_file);
        break;
    } // end of switch
        
   }  //end of menu 1
    p_menu = menu;
    delay(200);
}//end of loop
