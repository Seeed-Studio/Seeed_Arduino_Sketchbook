#include"LIS3DHTR.h"
#include <LovyanGFX.hpp>
#include <math.h>

#define DARKGREEN24 0x38761DU
#define GREEN24 0x00FF00
#define RED24 0xFF0000U
#define YELLOW24 0xFFFF00U
#define WHITE24 0xFFFFFFU
#define BLACK24 0x000000U
#define ORANGE24 0xF1C232U
#define EARTH24 0xB45F06U
#define SKY24   0x0000FFU
#define DEARTH24 0xA55706U
#define DSKY24   0x0000DBU

#define pi 3.1415

LIS3DHTR<TwoWire> lis;
static LGFX lcd;
static LGFX_Sprite canvas(&lcd);   
static LGFX_Sprite bank(&canvas);  
static LGFX_Sprite pitch(&bank);   
static auto transpalette = 0;           
 
 
void setup() {
  Serial.begin(115200);
//   while (!Serial);
  lis.begin(Wire1);
 
  if (!lis) {
    Serial.println("ERROR");
    while(1);
  }
  lis.setOutputDataRate(LIS3DHTR_DATARATE_25HZ); //Data output rate
  lis.setFullScaleRange(LIS3DHTR_RANGE_2G); //Scale range set to 2g

  // Display set up
  lcd.init();
  lcd.setRotation(1);
  lcd.setBrightness(128);
//set color
  canvas.setColorDepth(16);
  bank.setColorDepth(16);
  pitch.setColorDepth(16);

//keep memory
  canvas.createSprite(180,180);
  bank.createSprite(180, 180);
  pitch.createSprite(140, 140);

// create parts
  canvas.fillScreen(transpalette);
  bank.fillScreen(transpalette);
  pitch.fillScreen(transpalette);
  
//Pitch sprite  
  pitch.fillArc(70,70,70,1,0,180,DEARTH24);
  pitch.fillArc(70,70,70,1,180,360,DSKY24);
  pitch.drawFastHLine(0,70,140,WHITE24);   //0
  pitch.drawFastHLine(30,30,80,WHITE24);   //20
  pitch.drawFastHLine(50,50,40,WHITE24);   //10
  pitch.drawFastHLine(55,60,30,WHITE24);   //5
  pitch.drawFastHLine(55,40,30,WHITE24);   //15

//Bank sprite
  pitch.setPivot(70,70);
  pitch.pushRotateZoom(90,90,0,1,1);

  bank.fillArc(90,90,90,70,0,180,EARTH24);
  bank.fillArc(90,90,90,70,180,360,SKY24);
  bank.fillTriangle(90,20,85,3,95,3,WHITE24);
  bank.drawFastHLine(0,90,20,WHITE24);
  bank.drawFastHLine(160,90,20,WHITE24);
  bank.drawLine(90-90.0*cos(30.0/180.0*pi),90-90.0*sin(30.0/180.0*pi),90-70.0*cos(30.0/180.0*pi),90-70.0*sin(30.0/180.0*pi),WHITE24);
  bank.drawLine(90-90.0*cos(60.0/180.0*pi),90-90.0*sin(60.0/180.0*pi),90-70.0*cos(60.0/180.0*pi),90-70.0*sin(60.0/180.0*pi),WHITE24);
  bank.drawLine(90-80.0*cos(70.0/180.0*pi),90-80.0*sin(70.0/180.0*pi),90-70.0*cos(70.0/180.0*pi),90-70.0*sin(70.0/180.0*pi),WHITE24);
  bank.drawLine(90-80.0*cos(80.0/180.0*pi),90-80.0*sin(80.0/180.0*pi),90-70.0*cos(80.0/180.0*pi),90-70.0*sin(80.0/180.0*pi),WHITE24);
  bank.fillTriangle(40,40,35,31,31,35,WHITE24);
  bank.drawLine(90+90.0*cos(30.0/180.0*pi),90-90.0*sin(30.0/180.0*pi),90+70.0*cos(30.0/180.0*pi),90-70.0*sin(30.0/180.0*pi),WHITE24);
  bank.drawLine(90+90.0*cos(60.0/180.0*pi),90-90.0*sin(60.0/180.0*pi),90+70.0*cos(60.0/180.0*pi),90-70.0*sin(60.0/180.0*pi),WHITE24);
  bank.drawLine(90+80.0*cos(70.0/180.0*pi),90-80.0*sin(70.0/180.0*pi),90+70.0*cos(70.0/180.0*pi),90-70.0*sin(70.0/180.0*pi),WHITE24);
  bank.drawLine(90+80.0*cos(80.0/180.0*pi),90-80.0*sin(80.0/180.0*pi),90+70.0*cos(80.0/180.0*pi),90-70.0*sin(80.0/180.0*pi),WHITE24);
  bank.fillTriangle(139,40,144,31,148,35,WHITE24);
  
  lcd.clear(); 
  bank.setPivot(90,90);
  bank.pushRotateZoom(90,90,0,1,1);
  
  canvas.drawTriangle(90,20,85,30,95,30,ORANGE24);
  canvas.drawCircle(90,90,5,RED24);
  canvas.drawFastHLine(30,90,50,ORANGE24);
  canvas.drawFastHLine(100,90,50,ORANGE24);
  canvas.drawFastVLine(80,90,5,ORANGE24);
  canvas.drawFastVLine(100,90,5,ORANGE24);
  canvas.drawCircle(90,90,70,BLACK24);

  canvas.pushSprite(70,30);
  
}

  int p_plot_x = 159, p_plot_y = 119;
  float px_values=0.0, py_values=0.0, pz_values=0.0;
  float px_angle=0, py_angle=0, pz_angle=0;   
    
void loop() {
  float x_values, y_values, z_values, x_angle, y_angle, z_angle;
  x_values = lis.getAccelerationX();
  y_values = lis.getAccelerationY();
  z_values = lis.getAccelerationZ();

  x_angle = 180.0/3.1415*asin(x_values);
  y_angle = 180.0/3.1415*asin(y_values);
  z_angle = 180.0/3.1415*asin(z_values);
/* 
  Serial.print("X: "); Serial.print(x_values);
  Serial.print(" Y: "); Serial.print(y_values);
  Serial.print(" Z: "); Serial.print(z_values);
  Serial.println();
*/

//pitch sprite
  pitch.fillRect(0,0,140,70+2*z_angle,DSKY24);
  pitch.fillRect(0,70+2*z_angle,140,140,DEARTH24);
  pitch.drawFastHLine(0,70+2*z_angle,140,WHITE24);   //0
  pitch.drawFastHLine(30,30+2*z_angle,80,WHITE24);   //20
  pitch.drawFastHLine(50,50+2*z_angle,40,WHITE24);   //10
  pitch.drawFastHLine(55,60+2*z_angle,30,WHITE24);   //5
  pitch.drawFastHLine(55,40+2*z_angle,30,WHITE24);   //15
  pitch.drawFastHLine(30,110+2*z_angle,80,BLACK24);   //20
  pitch.drawFastHLine(50,90+2*z_angle,40,BLACK24);   //10
  pitch.drawFastHLine(55,80+2*z_angle,30,BLACK24);   //5
  pitch.drawFastHLine(55,100+2*z_angle,30,BLACK24);   //15
  //pitch.fillArc(70,70,70,90,0,359,BLACK24);
  pitch.fillTriangle(0,0,41,0,0,41,BLACK24);   
  pitch.fillTriangle(140,0,99,0,140,41,BLACK24);  
  pitch.fillTriangle(140,140,140,99,99,140,BLACK24);  
  pitch.fillTriangle(0,140,41,140,0,99,BLACK24);   
  pitch.setTextSize(1);
  pitch.drawNumber(20,20,27+2*z_angle);  pitch.drawNumber(20,110,27+2*z_angle);
  pitch.drawNumber(10,20,47+2*z_angle);  pitch.drawNumber(10,110,47+2*z_angle);
  pitch.drawNumber(20,20,107+2*z_angle);  pitch.drawNumber(20,110,107+2*z_angle);
  pitch.drawNumber(10,20,87+2*z_angle);  pitch.drawNumber(10,110,87+2*z_angle);
  
//Bank sprite
  pitch.setPivot(70,70);
  pitch.pushRotateZoom(90,90,0,1,1);

  bank.fillArc(90,90,90,70,0,180,EARTH24);
  bank.fillArc(90,90,90,70,180,360,SKY24);
  bank.fillTriangle(90,20,85,3,95,3,WHITE24);
  bank.drawFastHLine(0,90,20,WHITE24);
  bank.drawFastHLine(160,90,20,WHITE24);
  bank.drawLine(90-90.0*cos(30.0/180.0*pi),90-90.0*sin(30.0/180.0*pi),90-70.0*cos(30.0/180.0*pi),90-70.0*sin(30.0/180.0*pi),WHITE24);
  bank.drawLine(90-90.0*cos(60.0/180.0*pi),90-90.0*sin(60.0/180.0*pi),90-70.0*cos(60.0/180.0*pi),90-70.0*sin(60.0/180.0*pi),WHITE24);
  bank.drawLine(90-80.0*cos(70.0/180.0*pi),90-80.0*sin(70.0/180.0*pi),90-70.0*cos(70.0/180.0*pi),90-70.0*sin(70.0/180.0*pi),WHITE24);
  bank.drawLine(90-80.0*cos(80.0/180.0*pi),90-80.0*sin(80.0/180.0*pi),90-70.0*cos(80.0/180.0*pi),90-70.0*sin(80.0/180.0*pi),WHITE24);
  bank.fillTriangle(40,40,35,31,31,35,WHITE24);
  bank.drawLine(90+90.0*cos(30.0/180.0*pi),90-90.0*sin(30.0/180.0*pi),90+70.0*cos(30.0/180.0*pi),90-70.0*sin(30.0/180.0*pi),WHITE24);
  bank.drawLine(90+90.0*cos(60.0/180.0*pi),90-90.0*sin(60.0/180.0*pi),90+70.0*cos(60.0/180.0*pi),90-70.0*sin(60.0/180.0*pi),WHITE24);
  bank.drawLine(90+80.0*cos(70.0/180.0*pi),90-80.0*sin(70.0/180.0*pi),90+70.0*cos(70.0/180.0*pi),90-70.0*sin(70.0/180.0*pi),WHITE24);
  bank.drawLine(90+80.0*cos(80.0/180.0*pi),90-80.0*sin(80.0/180.0*pi),90+70.0*cos(80.0/180.0*pi),90-70.0*sin(80.0/180.0*pi),WHITE24);
  bank.fillTriangle(139,40,144,31,148,35,WHITE24);

  bank.setPivot(90,90);
  pitch.setPivot(70,70);

  bank.pushRotateZoom(90,90,y_angle,1,1);
  
  canvas.drawTriangle(90,20,85,30,95,30,ORANGE24);
  canvas.drawCircle(90,90,5,RED24);
  canvas.drawFastHLine(30,90,50,ORANGE24);
  canvas.drawFastHLine(100,90,50,ORANGE24);
  canvas.drawFastVLine(80,90,5,ORANGE24);
  canvas.drawFastVLine(100,90,5,ORANGE24);
  canvas.drawCircle(90,90,70,BLACK24);

  canvas.pushSprite(70,30);

  lcd.drawString("Pitch", 20, 35);
  lcd.drawString("Bank", 20, 65);
  lcd.drawString("Accel.", 250, 35);
  lcd.drawString("X", 260, 50);
  lcd.drawString("Y", 260, 70);
  lcd.drawString("Z", 260, 90);
 
  lcd.setTextColor(BLACK24,BLACK24);
  lcd.drawString(String(pz_angle), 30, 50);
  lcd.drawString(String(py_angle), 30, 80);
  lcd.drawString(String(px_values), 290, 50);
  lcd.drawString(String(py_values), 290, 70);
  lcd.drawString(String(pz_values), 290, 90);
  lcd.setTextColor(YELLOW24,BLACK24);
  lcd.drawString(String(z_angle), 30, 50);
  lcd.drawString(String(y_angle), 30, 80);
  lcd.drawString(String(x_values), 280, 50);
  lcd.drawString(String(y_values), 280, 70);
  lcd.drawString(String(z_values), 280, 90);

  px_values = x_values;
  py_values = y_values;
  pz_values = z_values;
  px_angle = x_angle;
  py_angle = y_angle;
  pz_angle = z_angle;
  
  delay(50);
}
