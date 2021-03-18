#include "get_hist_weather.h"

CircularBuffer<float, 72> stack;

const char* ssid = "";
const char* password =  "";
const char* apiKey = ""; 
extern const char* location = "Shenzhen,CN"; 
char server[] = "api.openweathermap.org";  

int status = WL_IDLE_STATUS; 
uint32_t timestamp;
float latitude;
float longitude;

// Use WiFiClient class to create TCP connections
WiFiClient client;

bool setupWIFI() {
 
    // Set WiFi to station mode and disconnect from an AP if it was previously connected
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    delay(100);

    WiFi.begin(ssid, password);
 
    while (WiFi.status() != WL_CONNECTED) {
        delay(250);
         LogTFT("Connecting to WiFi...\n");
    }
    LogTFT("Connected to the WiFi network\n");
    Serial.print(F("IP Address: "));
    Serial.println(WiFi.localIP()); // prints out the device's IP address
    
    return true;
}

DynamicJsonDocument get_data(char request[], uint8_t kb) {
  
 while (true)
 {  
 Serial.println(F("\nStarting connection to server...")); 
 // if you get a connection, report back via serial: 
 if (!client.connect(server, 80)) {
    LogTFT("Connection failed\n");
    delay(50);
    continue;
  }
  
    LogTFT("Connected to server\n"); 
   // Make a HTTP request: 
   client.print(request);
   if (client.println() == 0) {
    LogTFT("Failed to send request\n");
    client.stop();
    delay(50);
    continue;
   }

  DynamicJsonDocument doc(1024*kb);

  // Parse JSON object
  DeserializationError error = deserializeJson(doc, client);
  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
    delay(50);
    continue;
  }
  LogTFT("Parsing OK\n");
  //client.stop();
  return doc; 
} 
}

bool getCurrent() { 
 char request[1024];
 int n = sprintf (request, "GET /data/2.5/forecast?q=%s&appid=%s&cnt=1&units=metric\nHost: api.openweathermap.org\nConnection: close\n", location, apiKey);
 DynamicJsonDocument doc = get_data(request, 2);

 //get the data from the json tree 
 timestamp = doc["list"][0]["dt"];
 latitude = doc["city"]["coord"]["lat"];
 longitude = doc["city"]["coord"]["lon"];   
 Serial.println(timestamp);
 Serial.println(latitude);
 Serial.println(longitude);
 doc.clear();
 return true; 
 }

bool getHistorical() {
 char request[1024];
 bool buf_full = false;
 uint32_t day_unix = 0;
 
 while (true)
 {
  
 int n = sprintf (request, "GET /data/2.5/onecall/timemachine?lat=%f&lon=%f&dt=%d&appid=%s&units=metric\nHost: api.openweathermap.org\nConnection: close\n", latitude, longitude, timestamp-3600*8-day_unix, apiKey);
 DynamicJsonDocument doc = get_data(request, 10);

 Serial.println(doc["hourly"].size());
 for (uint8_t i = doc["hourly"].size()-1; i > 0; i = i - 1) {
   if (!stack.available()) { buf_full = true; break; }
   stack.push(float(doc["hourly"][i]["temp"]) / 60.0);
   stack.push(float(doc["hourly"][i]["humidity"]) / 100.0);
   stack.push(float(doc["hourly"][i]["pressure"]) / 1000.0);
  }

  day_unix += 86400;
  if (buf_full) { break; }
  doc.clear();
 }

 return true;
} 
