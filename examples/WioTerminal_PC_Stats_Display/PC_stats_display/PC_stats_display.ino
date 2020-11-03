//Libraries 
#include "TFT_eSPI.h" //TFT LCD library 
#include "Free_Fonts.h" //include the font library

//Initializations
TFT_eSPI tft; //Initializing TFT LCD
TFT_eSprite spr = TFT_eSprite(&tft); //Initializing buffer

void setup() {
  Serial.begin(9600); //start serial communication 
  tft.begin(); //Start TFT LCD
  tft.setRotation(3); //Set LCD rotation
  spr.createSprite(TFT_HEIGHT,TFT_WIDTH); //Create buffer
}

//initialize data types for variables 
String serialReceive;  
String CPUstat;
String cpufreq;
String UsedMemStr;
String FreeMemStr;
String hddUsed;
String hddFree;
String ssdUsed;
String ssdFree;
String GPUstat;
String GPUused;

void loop() {
  if(Serial.available() > 0) { //check whether if any data is available in serial buffer
    serialReceive = Serial.readString(); //read it as string and put into serialReceive variable
  }
  
  CPUstat = serialReceive.substring(0, 5); //split the received long string to substrings. (5 characters/information)
  cpufreq = serialReceive.substring(5, 10);
  UsedMemStr = serialReceive.substring(10, 15);
  FreeMemStr = serialReceive.substring(15, 20);
  ssdUsed = serialReceive.substring(20, 25);
  ssdFree = serialReceive.substring(25, 30);
  hddUsed = serialReceive.substring(30, 35);
  hddFree = serialReceive.substring(35, 40);
  GPUstat = serialReceive.substring(40, 45);
  GPUused = serialReceive.substring(45, 50);
  
  spr.fillSprite(tft.color565(0,255,255)); //Fill background with white color

  spr.drawFastHLine(0,84,320,TFT_BLACK); //Draw horizontal line
  spr.drawFastHLine(0,143,320,TFT_BLACK); 

  //CPU name 
  spr.setTextColor(TFT_BLACK); //set text color
  spr.setFreeFont(FSSB12); //set font 
  spr.drawString("RYZEN 3700X",5,5); //draw string 

  //CPU stats 
  spr.setFreeFont(FSS12);
  spr.drawString("Util: ",10,35);
  spr.drawString(CPUstat,70,35); 
  spr.drawString("%",120,35);
  spr.drawString("Speed: ",10,60);
  spr.drawString(cpufreq,75,60);
  spr.drawString("GHz",118,60);

  //GPU name
  spr.setFreeFont(FSSB12);
  spr.drawString("GTX 1660S",180,5);

  //GPU stats 
  spr.setFreeFont(FSS12);
  spr.drawString("Util: ",170,35);
  spr.drawString(GPUstat,235,35);
  spr.drawString("%",275,35);
  spr.drawString("Used: ",170,60);
  spr.drawString(GPUused,230,60);
  spr.drawString("GB",280,60);

  //RAM name
  spr.setFreeFont(FSSB12);
  spr.drawString("32GB DDR4",10,90);

  //RAM stats
  spr.setFreeFont(FSS12);
  spr.drawString("Used: ",10,120);
  spr.drawString(UsedMemStr,70,120);
  spr.drawString("GB",120,120);
  spr.drawString("Free: ",170,120);
  spr.drawString(FreeMemStr,230,120);
  spr.drawString("GB",280,120);
  
  //SSD and HDD names
  spr.setFreeFont(FSSB12);
  spr.drawString("500GB SSD",10,150);
  spr.drawString("1TB HDD",170,150);

  //SSD stats
  spr.setFreeFont(FSS12);
  spr.drawString("Used: ",10,180);
  spr.drawString(ssdUsed,70,180);
  spr.drawString("GB",125,180);
  spr.drawString("Free: ",10,210);
  spr.drawString(ssdFree,70,210);
  spr.drawString("GB",125,210);

  spr.drawFastVLine(165,0,84,TFT_BLACK); //draw vertical line
  spr.drawFastVLine(165,143,240,TFT_BLACK);

  //HDD stats
  spr.setFreeFont(FSS12);
  spr.drawString("Used: ",170,180);
  spr.drawString(hddUsed,225,180);
  spr.drawString("GB",280,180);
  spr.drawString("Free: ",170,210);
  spr.drawString(hddFree,225,210);
  spr.drawString("GB",280,210);

  spr.pushSprite(0,0); //Push to LCD
  delay(50);

}
