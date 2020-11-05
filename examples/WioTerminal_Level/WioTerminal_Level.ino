#include"LIS3DHTR.h"
#include "TFT_eSPI.h"
#include <math.h>

LIS3DHTR<TwoWire> lis;
TFT_eSPI tft;

void disp_sight() {
  tft.drawCircle(159,119,90,TFT_WHITE);
  tft.drawCircle(159,119,60,TFT_WHITE);
  tft.drawCircle(159,119,30,TFT_WHITE);
  tft.drawLine(159,29,159,209,TFT_WHITE);
  tft.drawLine(69,119,249,119,TFT_WHITE);
  tft.fillCircle(159,119,3,TFT_RED);  

  tft.fillRoundRect(56,215,206,20,10,TFT_DARKGREEN);
  tft.fillRoundRect(295,16,20,206,10,TFT_DARKGREEN);

}
 
void setup() {
  Serial.begin(115200);
//   while (!Serial);
  pinMode(WIO_5S_UP, INPUT_PULLUP);
  pinMode(WIO_5S_DOWN, INPUT_PULLUP);

  lis.begin(Wire1);
 
  if (!lis) {
    Serial.println("ERROR");
    while(1);
  }
  lis.setOutputDataRate(LIS3DHTR_DATARATE_25HZ); //Data output rate
  lis.setFullScaleRange(LIS3DHTR_RANGE_2G); //Scale range set to 2g

  // Display set up
  tft.begin();
  tft.setRotation(3);
  tft.setTextColor(TFT_YELLOW);
  tft.setTextSize(2);
  tft.fillScreen(TFT_BLACK); 
  disp_sight();
  tft.drawString("Angle", 5, 20);
  tft.drawString("X: ", 10, 40);
  tft.drawString("Y: ", 10, 60);
  tft.drawString("Accel.", 5, 120);
  tft.drawString("X: ", 10, 140);
  tft.drawString("Y: ", 10, 160);
  tft.drawString("Z: ", 10, 180);
  tft.drawString("Scale", 238, 20);
}

  int p_plot_x = 159, p_plot_y = 119;
  int scale = 1, p_scale = 5;
   
void loop() {
  float x_values, y_values, z_values;
  int plot_x, plot_y;
  x_values = lis.getAccelerationX();
  y_values = lis.getAccelerationY();
  z_values = lis.getAccelerationZ();
/* 
  Serial.print("X: "); Serial.print(x_values);
  Serial.print(" Y: "); Serial.print(y_values);
  Serial.print(" Z: "); Serial.print(z_values);
  Serial.println();
*/
  tft.fillRect(50,40,72,35,TFT_BLACK);
  tft.fillRect(50,140,72,55,TFT_BLACK);
  tft.drawString(String(180.0/3.1415*asin(x_values)), 50, 40);
  tft.drawString(String(180.0/3.1415*asin(y_values)), 50, 60);
  tft.drawString(String(x_values), 50, 140);
  tft.drawString(String(y_values), 50, 160);
  tft.drawString(String(z_values), 50, 180);
  if (scale != p_scale) {
  tft.fillRect(260,41,25,20,TFT_BLACK);
  tft.drawString(String(scale), 260, 41);
  }

  tft.fillCircle(p_plot_x,p_plot_y,10,TFT_BLACK);
  tft.fillRect(p_plot_x+3,215,6,20,TFT_DARKGREEN);
  tft.fillRect(295,p_plot_y-3,20,6,TFT_DARKGREEN);
  disp_sight();
  plot_x = 159 + scale*(90.0/3.1415 * 2 * asin(y_values));
  if (plot_x > 249) plot_x = 249;
  if (plot_x < 69) plot_x = 69;
  plot_y = 119 - scale*(90.0/3.1415 * 2 * asin(x_values));
  if (plot_y > 209) plot_y = 209;
  if (plot_y < 29) plot_y = 29;
  tft.fillCircle(plot_x,plot_y,10,TFT_YELLOW);
  tft.fillRect(plot_x-3,215,6,20,TFT_WHITE);
  tft.drawLine(plot_x,215,plot_x,235,TFT_BLACK);
  tft.fillRect(295,plot_y-3,20,6,TFT_WHITE);
  tft.drawLine(295,plot_y,315,plot_y,TFT_BLACK);

   if (digitalRead(WIO_5S_UP) == LOW) {
    scale++;
    if (scale > 5) scale = 5;
    while(digitalRead(WIO_5S_UP) == LOW);
   }
   if (digitalRead(WIO_5S_DOWN) == LOW) {
    scale--;
    if (scale <1) scale = 1;
    while(digitalRead(WIO_5S_DOWN) == LOW);
   }

  p_plot_x = plot_x;
  p_plot_y = plot_y;
  
  delay(50);
}
