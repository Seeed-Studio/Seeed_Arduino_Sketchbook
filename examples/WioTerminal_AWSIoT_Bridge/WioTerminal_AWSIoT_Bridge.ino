 /*********************************************************************************
   MQTT Bridge to AWS Example
 
   MQTT Client for MQTT AWS Bridge https://github.com/dnavarrom/aws_mosquitto_broker

   Modifier: Anson He (Seeed Studio)
   Modified: To support Wio Terminal to use Wi-Fi to connect to local broker to AWS IoT. Also, update to ArduinoJson v6 API.
 
   Tested on: Arduino Uno / Mega 2560 + Ethernet Shield
    
   Written by Diego Navarro M
   Using PubSubClient library https://github.com/knolleary/pubsubclient
   Using ArduinoJson Library https://bblanchon.github.io/ArduinoJson/
   Using SoftReset Library https://github.com/WickedDevice/SoftReset
   MIT license, all text above must be included in any redistribution
 ***********************************************************************************/

#include <rpcWiFi.h>
#include <PubSubClient.h> 
#include <ArduinoJson.h>

//Comment to disable debug output
#define GENERAL_DEBUG

#ifdef GENERAL_DEBUG
#define DEBUG_PRINT(string) (Serial.println(string))
#endif

#ifndef GENERAL_DEBUG
#define DEBUG_PRINT(String)
#endif

#define SEND_BUTTON  WIO_KEY_C // Button to send message
int lastState = 1;             // State variable

/************************* Wi-Fi Client Setup *****************************/

const char* ssid     = "se.101";     // your network SSID
const char* password = "qqqqqqqq9";  // your network password

char myIPAddress[20];

/************************* MQTT Client Config ********************************/

const char* mqttserver = "10.0.0.62";                  // Local Broker
const int mqttport = 1883;                             // MQTT port
String subscriptionTopic = "awsiot_to_localgateway";   // Get messages from AWS
String publishTopic = "localgateway_to_awsiot";        // Send messages to AWS 


/************************* MQTT Connection Monitoring ************************/

long connecionMonitoringFrequency = 10000UL;  // (1000 = 1 sec)
unsigned long lastConnectionCheck = 0;
int connectionAttempt = 0;
const int maxconnectionAttempt = 5;           // Reset after x connection attempt
const int maxConnectionCycles = 100;          // Security Reset (Soft Reet)
const int maxCiclosDiarios = 5000;            // Full Reset
int ConnectionCyclesCount = 0;
int totalCyclesCount = 0;


/********************* MQTT Broker Callback Function **************************

 /*!
    @brief  Process MQTT Messages (subscription)
    @param  topic
            the topic the message arrived on (const char[])
    @param  payload
            the message payload (byte array)
    @param  length
            the length of the message payload (unsigned int)
    @return nothing.
*/

void callback(char* topic, byte* payload, unsigned int length) {

  /*
  Internally, the client uses the same buffer for both inbound and outbound messages. After the callback function returns, 
  or if a call to either publish or subscribe is made from within the callback function, the topic and payload values passed 
  to the function will be overwritten. The application should create its own copy of the values if they are required beyond this.
  */
  
  // Allocate the correct amount of memory for the payload copy
  byte* p = (byte*)malloc(length);
  // Copy the payload to the new buffer
  memcpy(p,payload,length);
  
  char* chararray;
  chararray = (char*)p;

  ProcessPayload(chararray);

  free(p);
}

/************************** PubSubClient Declaration *******************************/

WiFiClient client;
PubSubClient mqttclient(mqttserver,mqttport,callback, client);   //Callback function must be declared before this line

/************************** Sketch Code ********************************************/

void setup() {

  #if defined(GENERAL_DEBUG)
  Serial.begin(9600);
  #endif

  // Initialise button
  pinMode(SEND_BUTTON, INPUT_PULLUP);

  // Configure mqttClient
  mqttclient.setServer(mqttserver, mqttport);
  mqttclient.setCallback(callback);

  // Connect to WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.println("Connecting to WiFi..");
    }
  Serial.println("Connected to the WiFi network");
  
  DEBUG_PRINT("<SETUP> : Board IP = " + (String)getIpReadable(WiFi.localIP()));

  // Connect to Broker
  if (reconnect()) {
    DEBUG_PRINT("<SETUP> : Connected to broker");
  }
  else {
    DEBUG_PRINT("<SETUP> : Initial Connection Failed");    
  }
  DEBUG_PRINT("<SETUP> : End Config..");
}

void loop() {

  // Check connection Status
  if (millis() - lastConnectionCheck > connecionMonitoringFrequency) {
    lastConnectionCheck = millis();
    CheckConnection();
  }

  // Send a message via pressing a button
  int state = digitalRead(SEND_BUTTON);
  if (state == 0) {
    delay(100); // Debounce
    if (state != lastState) {
      Publicar(publishTopic,"{\"message\": \"SENDING FROM WIO TERMINAL\"}");
      DEBUG_PRINT("MESSAGE SENT");
    }
    lastState = !lastState;
    delay(50);
  }

  mqttclient.loop();  //end cycle
}

/***************************** Payload Processing function ************************/

 /*!
    @brief  Process the Broker Payload
    @param  chararray
            Payload from Callback Function
    @return nothing
*/

void ProcessPayload(char* chararray) {

  size_t charSize;
  
  // Extract Json
  DynamicJsonDocument jsonBuffer(200);
  
  auto error = deserializeJson(jsonBuffer, chararray);
  JsonObject root = jsonBuffer.as<JsonObject>();
  
  if (error)
  {
    DEBUG_PRINT("<DecodeJson> parseObject() failed");
    return;
  }

  for (JsonObject::iterator it=root.begin(); it!=root.end(); ++it) {
    DEBUG_PRINT(it->key().c_str());
    DEBUG_PRINT(it->value().as<char*>());
}

  delay(1000);

  // Publish ACK
  Publicar(publishTopic,"{\"message\": \"OK\"}");
}

 /*!
    @brief  Publish to MQTT Broker
    @param  topic
            the topic the message to publish to
    @param  value
            Message to send
    @return Return true if message sent successfully
*/

bool Publicar(String topic, String value) 
{
  
  bool success = false;
  char cpytopic [50];
  char message [50];
  value.toCharArray(message, value.length() + 1);
  topic.toCharArray(cpytopic,topic.length() + 1);
  success = mqttclient.publish(cpytopic, message);
  
  return success;  
}




/***************************** Broker Connection Functions ************************/


 /*!
    @brief  Check Wi-Fi Connection
    @return Nothing
*/

void CheckConnection()
{
  ConnectionCyclesCount++;
  totalCyclesCount++;

  //Restart Wi-Fi after x connection cycles
  if (ConnectionCyclesCount > maxConnectionCycles)
  {
    DEBUG_PRINT("<CheckConnection> : Restart Wi-Fi..");
    client.stop();
    WiFi.begin(ssid, password);
    ConnectionCyclesCount = 0;
  }
  else
  {
    // Daily Softreset
    if (totalCyclesCount > maxCiclosDiarios) {
      DEBUG_PRINT("<CheckConnection> : Reset Device..");
      totalCyclesCount = 0;    
      delay(1000);
      NVIC_SystemReset(); // SAMD CMSIS function to reset
    }
    else
    {
      //Check MQTT Connection
      if (!mqttclient.connected()) {
        if (!reconnect()) {
          DEBUG_PRINT("<CheckConnection> : Disconnected.. connection attempt #: " + (String)connectionAttempt);
          connectionAttempt++;
          if (connectionAttempt > maxconnectionAttempt)
          {
            connectionAttempt = 0;
            DEBUG_PRINT("<CheckConnection> : Restart Wi-Fi!");
            client.stop();
            WiFi.begin(ssid, password);
            delay(1000);
          }
        }
        else
        {
          connectionAttempt = 0;
          DEBUG_PRINT("<CheckConnection> : Reconnected!");
        }
      }
      else
      {
          DEBUG_PRINT("<CheckConnection> : Connected!");
      }
    }
  }
}

 /*!
    @brief  Reconnect to MQTT Broker
    @return Return true if reconnected
*/

boolean reconnect() {
  if (mqttclient.connect("arduinoClient")) {

    char topicConnection [50];
    
    subscriptionTopic.toCharArray(topicConnection,25);
    mqttclient.subscribe(topicConnection);
  }
  
  return mqttclient.connected();
}



/******************************* Generic Helpers *********************************/

 /*!
    @brief  Convert IP address to readable string
*/

char* getIpReadable(IPAddress ipAddress)
{

  unsigned char octet[4]  = {
    0, 0, 0, 0    };
  for (int i = 0; i < 4; i++)
  {
    octet[i] = ( ipAddress >> (i * 8) ) & 0xFF;
  }

  sprintf(myIPAddress, "%d.%d.%d.%d\0", octet[0], octet[1], octet[2], octet[3]);

  return myIPAddress;
}

 /*!
    @brief  Decode Json message
*/

void DecodeJson(char json[]) {
  DynamicJsonDocument jsonBuffer(200);

  auto error = deserializeJson(jsonBuffer, json);
  JsonObject root = jsonBuffer.as<JsonObject>();
  
  if (error)
  {
    DEBUG_PRINT("<DecodeJson> parseObject() failed");
    return;
  }
  
  const char* message = root["message"];
  long data = root["value"];
}
