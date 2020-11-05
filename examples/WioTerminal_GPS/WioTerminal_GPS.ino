#include <SoftwareSerial.h>
#include <TinyGPS++.h>
#include "TFT_eSPI.h"
#include <SPI.h>
#include <Seeed_FS.h>
#include "SD/Seeed_SD.h"
 
SoftwareSerial mySerial(2, 3); // RX, TX
TinyGPSPlus gps;
TFT_eSPI tft;

TinyGPSCustom ExtLat(gps, "GPGGA", 3);  //N for Latitude
TinyGPSCustom ExtLng(gps, "GPGGA", 5);  //E for Longitude

const float pi = 3.1415;
int menu = 0, p_menu = 3;
int logging = 0, sat_n = 0;
double dist_LAT = 34.9722899, dist_LONG = 138.3868869;

String p_hour, p_lat, p_lng, p_alt, p_sat, p_date;
String p_my_speed, p_my_course, p_dist_dTo, p_dist_cTo;
String pdist_LAT, pdist_LONG;
String sdist_LAT = String(dist_LAT);
String sdist_LONG = String(dist_LONG);

//for Satellites position  ****************************
static const int MAX_SATELLITES = 40;

TinyGPSCustom totalGPGSVMessages(gps, "GPGSV", 1); // $GPGSV sentence, first element
TinyGPSCustom messageNumber(gps, "GPGSV", 2);      // $GPGSV sentence, second element
TinyGPSCustom satsInView(gps, "GPGSV", 3);         // $GPGSV sentence, third element
TinyGPSCustom satNumber[4]; // to be initialized later
TinyGPSCustom elevation[4];
TinyGPSCustom azimuth[4];
TinyGPSCustom snr[4];

struct
{
  bool active;
  int elevation;
  int azimuth;
  int snr;
  int dsp;
} sats[MAX_SATELLITES];

int Log_f = 0;
String N_date, hour0;
unsigned long p_time;
int N_y, N_m, N_d;
String hr1, min1, sec1;
String F_name;
bool sd;

// ************************

void disp_title(){
  if (menu == 0) {
    tft.fillScreen(TFT_BLACK); 
    tft.setTextSize(3);
    tft.drawString("GPS",120,3);
    tft.setTextSize(2);

    tft.drawString("Date",30,42);
    tft.drawString("Time",30,74);
    tft.drawString("LAT",30,106);
    tft.drawString("LONG",30,138);
    tft.drawString("ALT",30,170);
    tft.drawString("Satellites",30,202);
    if(sd != true) {
        tft.drawChar(295,223,'S',TFT_WHITE, TFT_RED,2);
        tft.drawChar(307,223,'D',TFT_WHITE, TFT_RED,2);     
    }
    p_hour = " "; p_lat = " "; p_lng = " "; p_alt = " "; p_sat = " "; p_date = " ";
    if(Log_f == 1)
        tft.drawChar(295,3,'L',TFT_YELLOW, TFT_BLACK,2);
  }
  else if(menu == 1) {
    tft.fillScreen(TFT_BLACK); 
    tft.setTextSize(3);
    tft.drawString("GPS",120,3);
    tft.setTextSize(2);

    tft.drawString("Speed",30,64);
    tft.drawString("Course",30,88);
    tft.drawString("Destination",30,112);
    tft.drawString("LAT",40,136);
    tft.drawString("LONG",40,160);
    tft.drawString("Distance",40,184);
    tft.drawString("Course",40,208);
    tft.fillRect(130,136,120,16,TFT_BLACK);
    tft.drawString(sdist_LAT,130,136);
    tft.fillRect(130,160,120,16,TFT_BLACK);
    tft.drawString(sdist_LONG,130,160);
    if(sd != true) {
        tft.drawChar(295,223,'S',TFT_WHITE, TFT_RED,2);
        tft.drawChar(307,223,'D',TFT_WHITE, TFT_RED,2);     
    }

    p_my_speed = " "; p_my_course = " "; p_dist_dTo = " "; p_dist_cTo = " ";
    if(Log_f == 1)
        tft.drawChar(295,3,'L',TFT_YELLOW, TFT_BLACK,2);    
  }

  else if(menu == 2) {
    tft.fillScreen(TFT_BLACK); 
    tft.setTextSize(2);

    tft.drawCircle(160,120,120,TFT_WHITE);
    tft.drawCircle(160,120,80,TFT_WHITE);
    tft.drawCircle(160,120,60,TFT_WHITE);
    tft.drawCircle(160,120,40,TFT_WHITE);
    tft.drawLine(160,1,160,239,TFT_WHITE);
    tft.drawLine(40,120,280,120,TFT_WHITE);
    tft.fillCircle(160,120,3,TFT_RED);
    tft.drawChar(153,0,'N',TFT_ORANGE, TFT_BLACK,3);
    tft.drawChar(153,219,'S',TFT_ORANGE, TFT_BLACK,3);
    tft.drawChar(24,111,'W',TFT_ORANGE, TFT_BLACK,3);
    tft.drawChar(280,111,'E',TFT_ORANGE, TFT_BLACK,3);
    if(sd != true) {
        tft.drawChar(295,223,'S',TFT_WHITE, TFT_RED,2);
        tft.drawChar(307,223,'D',TFT_WHITE, TFT_RED,2);     
    }

    for (int i=0; i<MAX_SATELLITES; ++i)
        sats[i-1].dsp = 0;
        if(Log_f == 1)
            tft.drawChar(295,3,'L',TFT_YELLOW, TFT_BLACK,2);
    }

}
//  ===================================================================
void setup() {
  // Open serial communications and wait for port to open:
    Serial.begin(57600);
    mySerial.begin(9600);

    pinMode(WIO_KEY_A, INPUT_PULLUP);
    pinMode(WIO_KEY_B, INPUT_PULLUP);
    pinMode(WIO_KEY_C, INPUT_PULLUP);

    Serial.print("Initializing SD card...");
    if (!SD.begin(SDCARD_SS_PIN, SDCARD_SPI)) {
      Serial.println("initialization failed!");
      //while (1);
      sd = false;
      tft.drawChar(295,223,'S',TFT_WHITE, TFT_RED,2);
      tft.drawChar(307,223,'D',TFT_WHITE, TFT_RED,2);     
    }
    else {
        Serial.println("initialization done.");
        sd = true;
    }
    tft.begin();
    tft.setRotation(3);

    tft.fillScreen(TFT_BLACK); //Black background    
    tft.setTextColor(TFT_YELLOW);

    //for satellites position
    // Initialize all the uninitialized TinyGPSCustom objects
    for (int i=0; i<4; ++i)
    {
      satNumber[i].begin(gps, "GPGSV", 4 + 4 * i); // offsets 4, 8, 12, 16
      elevation[i].begin(gps, "GPGSV", 5 + 4 * i); // offsets 5, 9, 13, 17
      azimuth[i].begin(  gps, "GPGSV", 6 + 4 * i); // offsets 6, 10, 14, 18
      snr[i].begin(      gps, "GPGSV", 7 + 4 * i); // offsets 7, 11, 15, 19
    }
}

//  ============================================================================
void loop() { 
    long N_mjd;
    
    while (mySerial.available() > 0) {
      char c = mySerial.read();
      //Serial.print(c);
      gps.encode(c);
    }

    if (((millis()-p_time) >3000) and (Log_f ==1)) {
      File myFile = SD.open(F_name,FILE_APPEND);
      if(myFile) {
            myFile.print(N_date);
            Serial.print(N_date);
            myFile.print(",");
            Serial.print(" , ");
            myFile.print(hour0);
            Serial.print(hour0);
            myFile.print(",");
            Serial.print(" , ");
            myFile.print(gps.location.lat(),6);
            Serial.print(gps.location.lat(),6);
            myFile.print(",");
            Serial.print(" , ");
            myFile.print(gps.location.lng(),6);
            Serial.print(gps.location.lng(),6);
            myFile.print(",");
            Serial.print(" , ");
            myFile.println(gps.altitude.meters());
            Serial.println(gps.altitude.meters());
            myFile.close();
      }
      p_time = millis();
    }
    
    double lat0 = gps.location.lat();
    double lat1 = (lat0 -int(lat0))*60;
    double lat2 = (lat1 - int(lat1))*60;
    String lat3 = String(int(lat0))+':' + String(int(lat1))+':'+String(lat2) + ' ' + String(ExtLat.value());

    double lng0 = gps.location.lng();
    double lng1 = (lng0 -int(lng0))*60;
    double lng2 = (lng1 - int(lng1))*60;
    String lng3 = String(int(lng0))+':' + String(int(lng1))+':'+String(lng2) + ' ' + String(ExtLng.value());

    int hr = gps.time.hour()+9;
    if (hr>24) hr -= 24;
    String hr0 = '0'+String(hr);
     hr1 = hr0.substring(hr0.length()-2); 
    String min0 = '0'+String(gps.time.minute());
     min1 = min0.substring(min0.length()-2); 
    String sec0 = '0'+String(gps.time.second());
     sec1 = sec0.substring(sec0.length()-2);
           hour0 =  hr1+':'+min1+':'+sec1;

//Calculate date 
    if (gps.date.isUpdated()) {
        int y = gps.date.year();
        int m = gps.date.month();
        int d = gps.date.day();
        if(m<3){y--; m+=12;}
        long mjd = int(361.7015*y)+int(y/400)-int(y/100)+int(30.59*(m-2))+d-678912;
        if(gps.time.hour()>14) 
            N_mjd = 58580+(mjd-51412);
        else
            N_mjd = 58579+(mjd-51412);
        long n = N_mjd+678881;
        long a = 4*n+3+3*(4*(n+1)/146097+1);
        long b = 5*((a % 1461)/4)+2;
        N_y = int(a/1461);
        N_m = int(b/153+3);
        N_d = int((b % 153)/5+1);
        if (N_m>12){N_y++; N_m-=12;}
        N_date = String(N_y)+'/'+('0'+String(N_m)).substring(('0'+String(N_m)).length()-2)+'/'+('0'+String(N_d)).substring(('0'+String(N_d)).length()-2);
    }

  if(menu == 0) {   //menu 0: Location -----------------------------
    if(menu != p_menu) {
      disp_title();
      p_menu = menu;
    }
        
    if (N_date != p_date) {    //Date
      tft.fillRect(100,42,120,16,TFT_BLACK);
      tft.drawString(N_date,100,42);
      p_date = N_date;
    }
    
    if (hour0 != p_hour) {    //Time
      tft.fillRect(100,74,120,16,TFT_BLACK);
      tft.drawString(hour0,100,74);
      p_hour = hour0;
    }
    
    if (lat3 != p_lat) {      //Latitude
      tft.fillRect(100,106,132,16,TFT_BLACK);
      tft.drawString(lat3,100,106);
      p_lat = lat3;
    }
    
    if (lng3 != p_lng) {      //Longitude
      tft.fillRect(100,138,156,16,TFT_BLACK);
      tft.drawString(lng3,100,138);
      p_lng = lng3;
    }

    if (String(gps.altitude.meters()) != p_alt) { //Altimeter
      tft.fillRect(100,170,60,16,TFT_BLACK);
      tft.drawString(String(gps.altitude.meters()),100,170);
      p_alt = String(gps.altitude.meters());
    }
    
    if (String(gps.satellites.value()) != p_sat) {  //N of Satellites
      tft.fillRect(160,202,32,16,TFT_BLACK);
      tft.drawString(String(gps.satellites.value()),160,202);
      p_sat = String(gps.satellites.value());
    }
  } // end of menu=0

  else if (menu == 1) {   //menu1: Speed  ---------------------------
      if(menu != p_menu) {
        disp_title();
        p_menu = menu;
      }
      String my_speed = String(gps.speed.mps());
      String my_course = String(gps.course.deg());
      double dist_dTo =
        TinyGPSPlus::distanceBetween(
          gps.location.lat(),
          gps.location.lng(),
          dist_LAT, 
          dist_LONG);
      double dist_cTo =
        TinyGPSPlus::courseTo(
          gps.location.lat(),
          gps.location.lng(),
          dist_LAT, 
          dist_LONG);
      String s_distanceTo = String(dist_dTo);
      String s_courseTo = String(dist_cTo);

      //Display Speed & Course
      if (my_speed != p_my_speed) {  //Speed
        tft.fillRect(130,64,48,16,TFT_BLACK);
        tft.drawString(my_speed,130,64);
        p_my_speed = my_speed;
      }
      
      if (my_course != p_my_course) {  //Course
        tft.fillRect(130,88,72,16,TFT_BLACK);
        tft.drawString(my_course,130,88);
        p_my_course = my_course;
      }
      
      if (sdist_LAT != pdist_LAT) {  //distination latitude
        tft.fillRect(130,136,120,16,TFT_BLACK);
        tft.drawString(sdist_LAT,130,136);
        pdist_LAT = sdist_LAT;
      }
      
      if (sdist_LONG != pdist_LONG) {  //distination longitude
        tft.fillRect(130,160,120,16,TFT_BLACK);
        tft.drawString(sdist_LONG,130,160);
        pdist_LONG = sdist_LONG;
      }
      
      if (s_distanceTo != p_dist_dTo) {  //distance to the mark point
        tft.fillRect(154,184,84,16,TFT_BLACK);
        tft.drawString(s_distanceTo,154,184);
        p_dist_dTo = s_distanceTo;
      }
      
      if (s_courseTo != p_dist_cTo) {  //Course to the mark point
        tft.fillRect(154,208,72,16,TFT_BLACK);
        tft.drawString(s_courseTo,154,208);
        p_dist_cTo = s_courseTo;
      }            
  } //end of menu = 1

  if (menu == 2) {   //menu 2: Satellites  ---------------------------
      if(menu != p_menu) {
        disp_title();
        p_menu = menu;
      }

    if (totalGPGSVMessages.isUpdated())
    {
      for (int i=0; i<4; ++i) {
        int no = atoi(satNumber[i].value());
        if (no >= 1 && no <= MAX_SATELLITES)
        {
          sats[no-1].elevation = atoi(elevation[i].value());
          sats[no-1].azimuth = atoi(azimuth[i].value());
          sats[no-1].snr = atoi(snr[i].value());
          sats[no-1].active = true;
        }
      }

      int totalMessages = atoi(totalGPGSVMessages.value());
      int currentMessage = atoi(messageNumber.value());
      if (totalMessages == currentMessage)
      {
        for (int i=0; i<MAX_SATELLITES; ++i)
        {
          if (sats[i].active)
          {            
            int p_X = 160 + (120*cos((sats[i].elevation/180.0)*pi)) * (sin((sats[i].azimuth)/360.0*2.0*pi));
            int p_Y = 120 - (120*cos((sats[i].elevation/180.0)*pi)) * (cos((sats[i].azimuth)/360.0*2.0*pi));
            tft.fillCircle( p_X, p_Y, 5,TFT_BLUE);
            if (sats[i].dsp == 0) {
                tft.drawString(String(i+1),p_X+3,p_Y+3);
                sats[i].dsp = 1;
            }
          }
        }
               
        for (int i=0; i<MAX_SATELLITES; ++i)
          sats[i].active = false;
      }
    }    
  }   //end of menu = 2
  
//  Button check
  if (digitalRead(WIO_KEY_C) == LOW) {  //  Page(menu) change
      menu++;
      if(menu > 2) menu = 0;
      while(digitalRead(WIO_KEY_C) == LOW){}
  }

  if (digitalRead(WIO_KEY_A) == LOW) {  //  Log to SD card
      if ((Log_f == 0) and (sd == true)) {
          Log_f = 1;
          tft.drawChar(295,3,'L',TFT_YELLOW, TFT_BLACK,2);
          p_time = millis();
          if(N_date !="" and hour0 !="") {
            F_name = String(N_y)+String(N_m)+String(N_d)+"_"+hr1+min1+sec1+".txt";
            Serial.println(F_name);
          }
          
      }
      else if ((Log_f == 1) and (sd == true)){
          Log_f = 0;
          tft.drawChar(295,3,'L',TFT_BLACK, TFT_BLACK,2);
          Serial.println(F_name+" closed");
      }
      while(digitalRead(WIO_KEY_A) == LOW){}
  }
  
  if (digitalRead(WIO_KEY_B) == LOW) {  //  Mark current location
      dist_LAT = gps.location.lat();
      dist_LONG = gps.location.lng();
      tft.drawChar(308,3,'M',TFT_YELLOW, TFT_BLACK,2);
      while(digitalRead(WIO_KEY_B) == LOW){}
  }

}   //  end of loop
