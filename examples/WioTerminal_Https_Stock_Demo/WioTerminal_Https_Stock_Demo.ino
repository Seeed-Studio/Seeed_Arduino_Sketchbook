#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include"Free_Fonts.h"
#include"TFT_eSPI.h"

TFT_eSPI tft;
TFT_eSprite spr = TFT_eSprite(&tft);  // Sprite

const char* ssid     = "SSID";
const char* password = "PASSWORD";

// This is an stock API from https://iexcloud.io/, please replace with your own token.
// For more api usage, please visit their api docs: https://iexcloud.io/docs/api/

char HTTPS_SERVER[] = "sandbox.iexapis.com";
char HTTPS_PATH[] = "/stable/stock/market/batch?symbols=aapl,fb,googl,amzn&types=price&range=1d&last=5&token=";
char TOKEN[] = "Tsk_eb711bae94c6489aa91688d4867f8485";

const char* test_root_ca = \
                           "-----BEGIN CERTIFICATE-----\n"
                           "MIIGCDCCA/CgAwIBAgIQKy5u6tl1NmwUim7bo3yMBzANBgkqhkiG9w0BAQwFADCB\n"
                           "hTELMAkGA1UEBhMCR0IxGzAZBgNVBAgTEkdyZWF0ZXIgTWFuY2hlc3RlcjEQMA4G\n"
                           "A1UEBxMHU2FsZm9yZDEaMBgGA1UEChMRQ09NT0RPIENBIExpbWl0ZWQxKzApBgNV\n"
                           "BAMTIkNPTU9ETyBSU0EgQ2VydGlmaWNhdGlvbiBBdXRob3JpdHkwHhcNMTQwMjEy\n"
                           "MDAwMDAwWhcNMjkwMjExMjM1OTU5WjCBkDELMAkGA1UEBhMCR0IxGzAZBgNVBAgT\n"
                           "EkdyZWF0ZXIgTWFuY2hlc3RlcjEQMA4GA1UEBxMHU2FsZm9yZDEaMBgGA1UEChMR\n"
                           "Q09NT0RPIENBIExpbWl0ZWQxNjA0BgNVBAMTLUNPTU9ETyBSU0EgRG9tYWluIFZh\n"
                           "bGlkYXRpb24gU2VjdXJlIFNlcnZlciBDQTCCASIwDQYJKoZIhvcNAQEBBQADggEP\n"
                           "ADCCAQoCggEBAI7CAhnhoFmk6zg1jSz9AdDTScBkxwtiBUUWOqigwAwCfx3M28Sh\n"
                           "bXcDow+G+eMGnD4LgYqbSRutA776S9uMIO3Vzl5ljj4Nr0zCsLdFXlIvNN5IJGS0\n"
                           "Qa4Al/e+Z96e0HqnU4A7fK31llVvl0cKfIWLIpeNs4TgllfQcBhglo/uLQeTnaG6\n"
                           "ytHNe+nEKpooIZFNb5JPJaXyejXdJtxGpdCsWTWM/06RQ1A/WZMebFEh7lgUq/51\n"
                           "UHg+TLAchhP6a5i84DuUHoVS3AOTJBhuyydRReZw3iVDpA3hSqXttn7IzW3uLh0n\n"
                           "c13cRTCAquOyQQuvvUSH2rnlG51/ruWFgqUCAwEAAaOCAWUwggFhMB8GA1UdIwQY\n"
                           "MBaAFLuvfgI9+qbxPISOre44mOzZMjLUMB0GA1UdDgQWBBSQr2o6lFoL2JDqElZz\n"
                           "30O0Oija5zAOBgNVHQ8BAf8EBAMCAYYwEgYDVR0TAQH/BAgwBgEB/wIBADAdBgNV\n"
                           "HSUEFjAUBggrBgEFBQcDAQYIKwYBBQUHAwIwGwYDVR0gBBQwEjAGBgRVHSAAMAgG\n"
                           "BmeBDAECATBMBgNVHR8ERTBDMEGgP6A9hjtodHRwOi8vY3JsLmNvbW9kb2NhLmNv\n"
                           "bS9DT01PRE9SU0FDZXJ0aWZpY2F0aW9uQXV0aG9yaXR5LmNybDBxBggrBgEFBQcB\n"
                           "AQRlMGMwOwYIKwYBBQUHMAKGL2h0dHA6Ly9jcnQuY29tb2RvY2EuY29tL0NPTU9E\n"
                           "T1JTQUFkZFRydXN0Q0EuY3J0MCQGCCsGAQUFBzABhhhodHRwOi8vb2NzcC5jb21v\n"
                           "ZG9jYS5jb20wDQYJKoZIhvcNAQEMBQADggIBAE4rdk+SHGI2ibp3wScF9BzWRJ2p\n"
                           "mj6q1WZmAT7qSeaiNbz69t2Vjpk1mA42GHWx3d1Qcnyu3HeIzg/3kCDKo2cuH1Z/\n"
                           "e+FE6kKVxF0NAVBGFfKBiVlsit2M8RKhjTpCipj4SzR7JzsItG8kO3KdY3RYPBps\n"
                           "P0/HEZrIqPW1N+8QRcZs2eBelSaz662jue5/DJpmNXMyYE7l3YphLG5SEXdoltMY\n"
                           "dVEVABt0iN3hxzgEQyjpFv3ZBdRdRydg1vs4O2xyopT4Qhrf7W8GjEXCBgCq5Ojc\n"
                           "2bXhc3js9iPc0d1sjhqPpepUfJa3w/5Vjo1JXvxku88+vZbrac2/4EjxYoIQ5QxG\n"
                           "V/Iz2tDIY+3GH5QFlkoakdH368+PUq4NCNk+qKBR6cGHdNXJ93SrLlP7u3r7l+L4\n"
                           "HyaPs9Kg4DdbKDsx5Q5XLVq4rXmsXiBmGqW5prU5wfWYQ//u+aen/e7KJD2AFsQX\n"
                           "j4rBYKEMrltDR5FL1ZoXX/nUh8HCjLfn4g8wGTeGrODcQgPmlKidrv0PJFGUzpII\n"
                           "0fxQ8ANAe4hZ7Q7drNJ3gjTcBpUC2JD5Leo31Rpg0Gcg19hCC0Wvgmje3WYkN5Ap\n"
                           "lBlGGSW4gNfL1IYoakRwJiNiqZ+Gb7+6kHDSVneFeO/qJakXzlByjAA6quPbYzSf\n"
                           "+AZxAeKCINT+b72x\n"
                           "-----END CERTIFICATE-----\n";

// You can use x.509 client certificates if you want

WiFiClientSecure client;

void setup() {
  //Initialize serial and wait for port to open:
  Serial.begin(115200);
  delay(100);

  Serial.print("Attempting to connect to SSID: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  tft.begin();
  tft.setRotation(3);
  tft.fillScreen(TFT_WHITE);
  tft.setFreeFont(FMB12);
  tft.setTextColor(TFT_BLACK);
  tft.setCursor((320 - tft.textWidth("Connecting to Wi-Fi..")) / 2, 120);
  tft.print("Connecting to Wi-Fi..");

  // attempt to connect to Wifi network:
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    // wait 1 second for re-trying
    delay(1000);
  }

  Serial.print("Connected to ");
  Serial.println(ssid);

  tft.fillScreen(TFT_WHITE);
  tft.setCursor((320 - tft.textWidth("Connected!")) / 2, 120);
  tft.print("Connected!");

  tft.setRotation(3);

  tft.fillScreen(TFT_BLACK);
  tft.setFreeFont(FF17);
  tft.setTextColor(tft.color565(250,211,207));
  tft.drawString("Live Stock Value",20,15);

  tft.fillRoundRect(5, 45, 150, 90, 5, tft.color565(36,112,160));
  tft.fillRoundRect(5, 145, 150, 90, 5, tft.color565(36,112,160));
  tft.fillRoundRect(165, 45, 150, 90, 5, tft.color565(36,112,160));
  tft.fillRoundRect(165, 145, 150, 90, 5, tft.color565(36,112,160));

  tft.setFreeFont(FM9);
  tft.setTextColor(tft.color565(250,211,207));
  tft.drawString("APPLE", 40, 50);
  tft.drawString("GOOGLE",210, 50);
  tft.drawString("FACEBOOK",40, 150);
  tft.drawString("AMAZON",210, 150);

}

void loop()
{
  getSum();
  delay(50);
}

void getSum() {
  String data;
  client.setCACert(test_root_ca);

  Serial.println("\nStarting connection to server...");
  if (!client.connect(HTTPS_SERVER, 443)) {
    Serial.println("Connection failed!");
  } else {
    Serial.println("Connected to server!");
    // Make a HTTP request:
    client.print("GET ");
    client.print(HTTPS_PATH);
    client.print(TOKEN);
    client.println(" HTTP/1.1");
    client.print("Host: ");
    client.println(HTTPS_SERVER);
    client.println("Connection: close");
    client.println();
    delay(50); 
    
    while (client.connected()) {
      String line = client.readStringUntil('\n');
      if (line == "\r") {
        Serial.println("headers received");
        break;
      }
    }

    while (client.available())
    {
      String line = client.readStringUntil('\r');
      data = line;
    }
    client.stop();
    Serial.println("closing connection");
  }
      //ArduinoJson to parse data, plesae check ArduinoJson for more info
    const size_t capacity = 4*JSON_OBJECT_SIZE(1) + JSON_OBJECT_SIZE(4) + 50;
    DynamicJsonDocument doc(capacity);
    deserializeJson(doc, data);

    float AAPL_price = doc["AAPL"]["price"];
    float FB_price = doc["FB"]["price"]; 
    float GOOGL_price = doc["GOOGL"]["price"];
    float AMZN_price = doc["AMZN"]["price"];

   // Sprite buffer
    spr.createSprite(100, 30);
    spr.fillSprite(tft.color565(36,112,160));
    spr.setFreeFont(FMB12);
    spr.setTextColor(TFT_WHITE);
    spr.drawFloat(AAPL_price, 2, 0, 0); //Apple
    spr.pushSprite(35, 80);
    spr.deleteSprite();

   // Sprite buffer
    spr.createSprite(100, 30);
    spr.fillSprite(tft.color565(36,112,160));
    spr.setFreeFont(FMB12);
    spr.setTextColor(TFT_WHITE);
    spr.drawFloat(GOOGL_price, 2, 0, 0); //Google
    spr.pushSprite(200, 80);
    spr.deleteSprite();
    
   // Sprite buffer
    spr.createSprite(100, 30);
    spr.fillSprite(tft.color565(36,112,160));
    spr.setFreeFont(FMB12);
    spr.setTextColor(TFT_WHITE);
    spr.drawFloat(FB_price, 2, 0, 0); //fb
    spr.pushSprite(35, 180);
    spr.deleteSprite();

    // Sprite buffer
    spr.createSprite(100, 30);
    spr.fillSprite(tft.color565(36,112,160));
    spr.setFreeFont(FMB12);
    spr.setTextColor(TFT_WHITE);
    spr.drawFloat(AMZN_price, 2, 0, 0); //amazon
    spr.pushSprite(200, 180);
    spr.deleteSprite();

}
