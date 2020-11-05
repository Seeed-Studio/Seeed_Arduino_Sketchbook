#include <LovyanGFX.hpp>
#include <Servo.h>
Servo myservo;

//int outpin=D2;

int counter = 0;
int i = 0;
static LGFX lcd;
static LGFX_Sprite sprite(&lcd);

int pos = 0;

void drawEye(int x, int y) {
  sprite.clear();
  sprite.fillCircle(65,65,60,0xFFFF);
  //sprite.drawEllipse(65,65,60,30,0xFFFF);
  //sprite.drawFastHLine(5,62,120,0xFF);
  sprite.fillCircle(x,y,15,0);
  sprite.fillArc(65,65,60,1,180,360,0xED6E);
  sprite.setPivot(65,65);
  sprite.pushRotateZoom(80,70,0,1,0.5);
  sprite.pushRotateZoom(240,70,0,1,0.5);
}

void sleepyEyes() {
  int p_rect[4][4] = {{9,65,112,22},{22,65,86,41},{40,65,49,55},{63,65,4,59}};
  sprite.clear();
  sprite.fillCircle(65,65,60,0xFFFF);
  sprite.fillCircle(65,70,10,0);
  //sprite.fillArc(65,65,60,1,180,360,0xED6E);
  sprite.setPivot(65,65);
  for (int i=0;i<4;i++) {
    sprite.fillArc(65,65,60,1,180,360,0xED6E);
    sprite.fillArc(65,65,60,1,0,22*(i+1),0xED6E);
    sprite.fillArc(65,65,60,1,180-(22*(i+1)),180,0xED6E);
    sprite.fillRect(p_rect[i][0],p_rect[i][1],p_rect[i][2],p_rect[i][3],0xED6E);
    sprite.pushRotateZoom(80,70,0,1,0.5);
    sprite.pushRotateZoom(240,70,0,1,0.5);
    delay(200);
  }
  delay(2000);
  //sprite.pushRotateZoom(80,70,0,1,0.5);
  //sprite.pushRotateZoom(240,70,0,1,0.5);
}


void setup() {
  pinMode(WIO_5S_UP, INPUT_PULLUP);
  pinMode(WIO_5S_DOWN, INPUT_PULLUP);

  myservo.attach(0);
  
  lcd.init();
  lcd.setRotation(3);
  lcd.setBrightness(128);
  lcd.setColorDepth(16);
  lcd.clear();

  sprite.setColorDepth(16);
  sprite.createSprite(130, 130);
  drawEye(50,80);
  lcd.drawArc(160,90,120,119,30,150,0xE800);

}

void loop() {
    int x = random(15, 115);

    for (pos = 0; pos <= 60; pos += 1) {
      int l = pos + 20;
      drawEye(l,80);
      myservo.write(pos);
      delay(80);
  }

    for (pos = 60; pos >= 0; pos -= 1) {
      int l = pos;
      drawEye(l,80);
      myservo.write(pos);
      delay(80);
    }    

  delay(3000);

    sleepyEyes();


}
