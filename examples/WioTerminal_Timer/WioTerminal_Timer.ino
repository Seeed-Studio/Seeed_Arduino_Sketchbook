#include <LovyanGFX.hpp>

static LGFX lcd;                 
static LGFX_Sprite ttext(&lcd); 
static LGFX_Sprite arc(&lcd); 
static LGFX_Sprite sp(&lcd); 
static auto transpalette = 0;           
 
unsigned int set_time = 180, pset_time = 0;
unsigned int rm_time;
int c_status = 0;
unsigned long p_millis, c_millis;
int c_pos = 0, p_pos = 0;
int sound = 1;
int pos[] = {91,123,167,199};
int inc[] = {600,60,10,1};

void disp_time(int d_min, int d_sec){   //  Display Timer
      arc.pushRotateZoom(160,120,0,1.5,1);
      ttext.setCursor(0, 0);
      ttext.setTextColor(0xFFE0, 0); 
      ttext.printf("%02d:%02d", d_min, d_sec); 
      ttext.pushRotateZoom(160,115,0,1,1);

}

void pipipi() {   //    Alarm sound
  int j;
  while (digitalRead(WIO_KEY_C) == HIGH) {
    if (sound == 1) {
      j++;
      if (j>3)j=1;
        for (int i=0; i<3; i++){
          digitalWrite(WIO_BUZZER, 1);
          delay(80);
          digitalWrite(WIO_BUZZER, 0);
          delay(80);
          Serial.print("11111");
        }
      delay(100);
    }
  }
}

void setup() {
    Serial.begin(115200);
    pinMode(WIO_KEY_A, INPUT_PULLUP);
    pinMode(WIO_KEY_B, INPUT_PULLUP);
    pinMode(WIO_KEY_C, INPUT_PULLUP);
    pinMode(WIO_5S_UP, INPUT_PULLUP);
    pinMode(WIO_5S_DOWN, INPUT_PULLUP);
    pinMode(WIO_5S_LEFT, INPUT_PULLUP);
    pinMode(WIO_5S_RIGHT, INPUT_PULLUP);
    pinMode(WIO_5S_PRESS, INPUT_PULLUP);
    pinMode(WIO_BUZZER, OUTPUT);

    lcd.init();
    lcd.setRotation(1);
    lcd.setBrightness(255);
    lcd.setColorDepth(16);
    lcd.clear();
    
//    lcd.setTextColor(TFT_YELLOW);
    ttext.setFont(&fonts::Font7);
    lcd.fillScreen(TFT_BLACK);

    arc.setColorDepth(16);
    ttext.setColorDepth(16);
    sp.setColorDepth(16);
    arc.createSprite(240,160);
    ttext.createSprite(140,60);    
    sp.createSprite(12,13);
    arc.fillScreen(transpalette);
    ttext.fillScreen(transpalette);
    sp.fillScreen(transpalette);
    arc.fillArc(80,80,60,80,0,360,0xC424);
    arc.setPivot(80,80);
    ttext.setPivot(70,15);
    // Speakericon
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
    sp.pushSprite(&lcd, 300, 5);
    lcd.setTextColor(0xFFE0, 0);
    pinMode(WIO_BUZZER, OUTPUT);     
}
 
void loop() {
  int t_min, t_sec;

  if (digitalRead(WIO_5S_PRESS) == LOW) {   //  5S Press button
    if (c_status == 0) {
      lcd.drawFastHLine(pos[c_pos],148,30,0);
      c_status = 1;                         //  wait -> start
      rm_time = set_time;
    }
    else if (c_status == 1) c_status = 2;   //  start -> pause
    else if (c_status == 2) c_status = 1;    //  pause -> resume
    else c_status = 3;                      //  time up
    
    while(digitalRead(WIO_5S_PRESS) == LOW);
  }
  
  if (digitalRead(WIO_KEY_C) == LOW)  {   //  C button to Reset
    c_status = 0;
    arc.fillArc(80,80,60,80,0,360,0xC424);
    t_min = set_time/60;
    t_sec = set_time%60;
    disp_time(t_min, t_sec);   
    lcd.drawFastHLine(pos[c_pos],148,30,0xFFFF);       
    while(digitalRead(WIO_KEY_C) == LOW);
  }

  if (digitalRead(WIO_KEY_A) == LOW)  {   //  A button to Alarm sound on/off 
    if(sound == 1) {
      sound = 0;
      lcd.fillRect(300,5,15,15,0);
    }
    else {
      sound = 1;
      sp.pushSprite(&lcd, 300, 5);
    }
    while(digitalRead(WIO_KEY_A) == LOW);
  }

  if (c_status == 0 ) {                       //  set up timer
    if (digitalRead(WIO_5S_RIGHT) == LOW) {   // move set up cursor next
      lcd.drawFastHLine(pos[c_pos],148,30,0);
      c_pos++;
      if (c_pos > 3) c_pos = 0;
      lcd.drawFastHLine(pos[c_pos],148,30,0xFFFF);      
      while(digitalRead(WIO_5S_RIGHT) == LOW);      
    }
    if (digitalRead(WIO_5S_LEFT) == LOW) {    // move set up cursor previous
      lcd.drawFastHLine(pos[c_pos],148,30,0);
      c_pos--;
      if (c_pos < 0) c_pos = 3;
      lcd.drawFastHLine(pos[c_pos],148,30,0xFFFF);      
      while(digitalRead(WIO_5S_LEFT) == LOW);      
    }

    if (digitalRead(WIO_5S_UP) == LOW) {    // increase figure on cursor
      set_time += inc[c_pos];
      if (set_time > 5999) set_time = 5999;
      while(digitalRead(WIO_5S_UP) == LOW);      
    }

    if (digitalRead(WIO_5S_DOWN) == LOW) {    // decrease figure on cursor
      set_time -= inc[c_pos];
      if (set_time > 5999) {
        set_time += inc[c_pos];
        set_time = set_time%inc[c_pos];
      }
      while(digitalRead(WIO_5S_DOWN) == LOW);      
    }

    if (set_time != pset_time) {
    t_min = set_time/60;
    t_sec = set_time%60;
    disp_time(t_min, t_sec);
    lcd.drawFastHLine(pos[c_pos],148,30,0xFFFF);      
    }
    pset_time = set_time;
  }   //  end of c_status 0


  if (c_status == 1) {    //  count down timer
    c_millis = millis();
    if( c_millis - p_millis >= 1000 ) {
      p_millis = c_millis;
      rm_time--;
       if (set_time > 5999) rm_time = 0;
       
      arc.clear();
      if((360*rm_time)/set_time < 90) arc.fillArc(80,80,60,80,270,270+360*rm_time/set_time,0xC424);
      else arc.fillArc(80,80,60,80,270,(360*rm_time)/set_time-90,0xC424);
      arc.pushRotateZoom(160,120,0,1.5,1);

      t_min = rm_time/60;
      t_sec = rm_time%60;
      disp_time(t_min, t_sec);    
    
    }   //end of  millis
    if (rm_time < 1) c_status = 3;
  }   // end of c_status 1

  if (c_status == 3) {    //  timer reset
    pipipi();
    c_status = 0;
  }
}   //  end of loop
