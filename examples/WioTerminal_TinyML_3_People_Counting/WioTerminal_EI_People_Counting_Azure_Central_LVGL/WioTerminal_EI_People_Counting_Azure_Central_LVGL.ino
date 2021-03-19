/*
    An example sketch for Edge Impulse trained model inference for People counting with Grove Ultrasonic Ranger & Azure Central Data Upload with LVGL Interface
    (continious inference)

    Copyright (c) 2021 Seeed technology co., ltd.
    Author      : Dmitry Maslov, based on Benjamin Kabe Azure SDK Arduino example & SeeedJp fork
    Create Time : February 2021
    Change Log  :

    The MIT License (MIT)

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in
    all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
    THE SOFTWARE.
*/

#include "Config.h"
#include "Storage.h"
#include "Signature.h"
#include "AzureDpsClient.h"
#include "CliMode.h"

#include <rpcWiFiClientSecure.h>
#include <PubSubClient.h>

#include <WiFiUdp.h>
#include <NTP.h>

#include <az_json.h>
#include <az_result.h>
#include <az_span.h>
#include <az_iot_hub_client.h>

#include <people_counter_raw_inference.h>
#include <Seeed_Arduino_FreeRTOS.h>
#include "Ultrasonic.h"
#include "TFT_eSPI.h"
#include <lvgl.h>

#define MQTT_PACKET_SIZE 1024
#define LVGL_TICK_PERIOD 10

const char* ROOT_CA_BALTIMORE =
"-----BEGIN CERTIFICATE-----\n"
"MIIDdzCCAl+gAwIBAgIEAgAAuTANBgkqhkiG9w0BAQUFADBaMQswCQYDVQQGEwJJ\n"
"RTESMBAGA1UEChMJQmFsdGltb3JlMRMwEQYDVQQLEwpDeWJlclRydXN0MSIwIAYD\n"
"VQQDExlCYWx0aW1vcmUgQ3liZXJUcnVzdCBSb290MB4XDTAwMDUxMjE4NDYwMFoX\n"
"DTI1MDUxMjIzNTkwMFowWjELMAkGA1UEBhMCSUUxEjAQBgNVBAoTCUJhbHRpbW9y\n"
"ZTETMBEGA1UECxMKQ3liZXJUcnVzdDEiMCAGA1UEAxMZQmFsdGltb3JlIEN5YmVy\n"
"VHJ1c3QgUm9vdDCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAKMEuyKr\n"
"mD1X6CZymrV51Cni4eiVgLGw41uOKymaZN+hXe2wCQVt2yguzmKiYv60iNoS6zjr\n"
"IZ3AQSsBUnuId9Mcj8e6uYi1agnnc+gRQKfRzMpijS3ljwumUNKoUMMo6vWrJYeK\n"
"mpYcqWe4PwzV9/lSEy/CG9VwcPCPwBLKBsua4dnKM3p31vjsufFoREJIE9LAwqSu\n"
"XmD+tqYF/LTdB1kC1FkYmGP1pWPgkAx9XbIGevOF6uvUA65ehD5f/xXtabz5OTZy\n"
"dc93Uk3zyZAsuT3lySNTPx8kmCFcB5kpvcY67Oduhjprl3RjM71oGDHweI12v/ye\n"
"jl0qhqdNkNwnGjkCAwEAAaNFMEMwHQYDVR0OBBYEFOWdWTCCR1jMrPoIVDaGezq1\n"
"BE3wMBIGA1UdEwEB/wQIMAYBAf8CAQMwDgYDVR0PAQH/BAQDAgEGMA0GCSqGSIb3\n"
"DQEBBQUAA4IBAQCFDF2O5G9RaEIFoN27TyclhAO992T9Ldcw46QQF+vaKSm2eT92\n"
"9hkTI7gQCvlYpNRhcL0EYWoSihfVCr3FvDB81ukMJY2GQE/szKN+OMY3EU/t3Wgx\n"
"jkzSswF07r51XgdIGn9w/xZchMB5hbgF/X++ZRGjD8ACtPhSNzkE1akxehi/oCr0\n"
"Epn3o0WC4zxe9Z2etciefC7IpJ5OCBRLbf1wbWsaY71k5h+3zvDyny67G7fyUIhz\n"
"ksLi4xaNmjICq44Y3ekQEe5+NauQrz4wlHrQMz2nZQ/1/I6eYs9HRCwBXbsdtTLS\n"
"R9I4LtD+gdwyah617jzV/OeBHRnDJELqYzmp\n"
"-----END CERTIFICATE-----";

WiFiClientSecure wifi_client;
PubSubClient mqtt_client(wifi_client);
WiFiUDP wifi_udp;
NTP ntp(wifi_udp);

std::string HubHost;
std::string DeviceId;

static bool debug_nn = false; // Set this to true to see e.g. features generated from the raw signal
static uint32_t run_inference_every_ms = 500;

static float buffer[EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE] = {0};
static float inference_buffer[EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE];
float distance;
uint8_t axis_num = 1;

int16_t peopleCount = 0;
uint16_t peopleIn = 0;
uint16_t peopleOut = 0;

lv_obj_t *LogOutput;
lv_obj_t *peopleInLabel;
lv_obj_t *peopleOutLabel;
lv_obj_t *peopleNumLabel;

const char *prev_prediction = "none";

TaskHandle_t Handle_aTask;
TaskHandle_t Handle_bTask;
TaskHandle_t Handle_cTask;

Ultrasonic ultrasonic(0);
TFT_eSPI tft;
static lv_disp_buf_t disp_buf;
static lv_color_t buf[LV_HOR_RES_MAX * 10];

#define AZ_RETURN_IF_FAILED(exp) \
  do \
  { \
    az_result const _result = (exp); \
    if (az_result_failed(_result)) \
    { \
      return _result; \
    } \
  } while (0)

////////////////////////////////////////////////////////////////////////////////
// 

#define DLM "\r\n"

static String StringVFormat(const char* format, va_list arg)
{
    const int len = vsnprintf(nullptr, 0, format, arg);
    char str[len + 1];
    vsnprintf(str, sizeof(str), format, arg);

    return String{str};
}

static void Abort(const char* format, ...)
{
    va_list arg;
    va_start(arg, format);
    String str{ StringVFormat(format, arg) };
    va_end(arg);

    Serial.printf("ABORT: %s" DLM, str.c_str());

    while (true) {}
}

static void Log(const char* format, ...)
{
    va_list arg;
    va_start(arg, format);
    String str{StringVFormat(format, arg)};
    va_end(arg);

    Serial.print(str);
    
}

/* Display flushing */
void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p)
{
  uint16_t c;

  tft.startWrite(); /* Start new TFT transaction */
  tft.setAddrWindow(area->x1, area->y1, (area->x2 - area->x1 + 1), (area->y2 - area->y1 + 1)); /* set the working window */
  for (int y = area->y1; y <= area->y2; y++) {
    for (int x = area->x1; x <= area->x2; x++) {
      c = color_p->full;
      tft.writeColor(c, 1);
      color_p++;
    }
  }
  tft.endWrite(); /* terminate TFT transaction */
  lv_disp_flush_ready(disp); /* tell lvgl that flushing is done */
}


////////////////////////////////////////////////////////////////////////////////
// Display

static void DisplayPrintf(const char* format, ...)
{
    va_list arg;
    va_start(arg, format);
    String str{StringVFormat(format, arg)};
    va_end(arg);

    Log("%s\n", str.c_str());
    lv_label_set_text(LogOutput, str.c_str());
    lv_task_handler();

}

////////////////////////////////////////////////////////////////////////////////
// Interface init

void lv_buttons(void)
{
    lv_obj_t *peopleInDisplay = lv_btn_create(lv_scr_act(), NULL);     /*Add a button the current screen*/
    lv_obj_set_pos(peopleInDisplay, 20, 60);                            /*Set its position*/
    lv_obj_set_size(peopleInDisplay, 120, 50);                          /*Set its size*/
    peopleInLabel = lv_label_create(peopleInDisplay, NULL);          /*Add a label to the button*/
    lv_label_set_text(peopleInLabel, "0");                     /*Set the labels text*/

    lv_obj_t *peopleOutDisplay = lv_btn_create(lv_scr_act(), NULL);     /*Add a button the current screen*/
    lv_obj_set_pos(peopleOutDisplay, 180, 60);                            /*Set its position*/
    lv_obj_set_size(peopleOutDisplay, 120, 50);                          /*Set its size*/
    peopleOutLabel = lv_label_create(peopleOutDisplay, NULL);          /*Add a label to the button*/
    lv_label_set_text(peopleOutLabel, "0");                     /*Set the labels text*/

    lv_obj_t *peopleNumDisplay = lv_btn_create(lv_scr_act(), NULL);     /*Add a button the current screen*/
    lv_obj_set_pos(peopleNumDisplay, 90, 160);                            /*Set its position*/
    lv_obj_set_size(peopleNumDisplay, 140, 70);                          /*Set its size*/
    peopleNumLabel = lv_label_create(peopleNumDisplay, NULL);          /*Add a label to the button*/
    lv_label_set_text(peopleNumLabel, "0");                     /*Set the labels text*/

    LogOutput = lv_label_create(lv_scr_act(), NULL);
    lv_label_set_long_mode(LogOutput, LV_LABEL_LONG_BREAK);     /*Break the long lines*/
    lv_label_set_recolor(LogOutput, true);                      /*Enable re-coloring by commands in the text*/
    lv_label_set_align(LogOutput, LV_LABEL_ALIGN_LEFT);       /*Center aligned lines*/
    lv_obj_set_width(LogOutput, 320);
    lv_obj_align(LogOutput, NULL, LV_ALIGN_IN_TOP_LEFT, 20, 10);

}

////////////////////////////////////////////////////////////////////////////////
// Azure IoT DPS

static AzureDpsClient DpsClient;
static unsigned long DpsPublishTimeOfQueryStatus = 0;

static void MqttSubscribeCallbackDPS(char* topic, byte* payload, unsigned int length);

static int RegisterDeviceToDPS(const std::string& endpoint, const std::string& idScope, const std::string& registrationId, const std::string& symmetricKey, const uint64_t& expirationEpochTime, std::string* hubHost, std::string* deviceId)
{
    std::string endpointAndPort{ endpoint };
    endpointAndPort += ":";
    endpointAndPort += std::to_string(8883);

    if (DpsClient.Init(endpointAndPort, idScope, registrationId) != 0) return -1;

    const std::string mqttClientId = DpsClient.GetMqttClientId();
    const std::string mqttUsername = DpsClient.GetMqttUsername();

    const std::vector<uint8_t> signature = DpsClient.GetSignature(expirationEpochTime);
    const std::string encryptedSignature = GenerateEncryptedSignature(symmetricKey, signature);
    const std::string mqttPassword = DpsClient.GetMqttPassword(encryptedSignature, expirationEpochTime);

    const std::string registerPublishTopic = DpsClient.GetRegisterPublishTopic();
    const std::string registerSubscribeTopic = DpsClient.GetRegisterSubscribeTopic();

    Log("DPS:" DLM);
    Log(" Endpoint = %s" DLM, endpoint.c_str());
    Log(" Id scope = %s" DLM, idScope.c_str());
    Log(" Registration id = %s" DLM, registrationId.c_str());
    Log(" MQTT client id = %s" DLM, mqttClientId.c_str());
    Log(" MQTT username = %s" DLM, mqttUsername.c_str());
    //Log(" MQTT password = %s" DLM, mqttPassword.c_str());

    wifi_client.setCACert(ROOT_CA_BALTIMORE);
    mqtt_client.setBufferSize(MQTT_PACKET_SIZE);
    mqtt_client.setServer(endpoint.c_str(), 8883);
    mqtt_client.setCallback(MqttSubscribeCallbackDPS);
    DisplayPrintf("Connecting to Azure IoT Hub DPS...");
    if (!mqtt_client.connect(mqttClientId.c_str(), mqttUsername.c_str(), mqttPassword.c_str())) return -2;

    mqtt_client.subscribe(registerSubscribeTopic.c_str());
    mqtt_client.publish(registerPublishTopic.c_str(), "{payload:{\"modelId\":\"" IOT_CONFIG_MODEL_ID "\"}}");

    while (!DpsClient.IsRegisterOperationCompleted())
    {
        mqtt_client.loop();
        if (DpsPublishTimeOfQueryStatus > 0 && millis() >= DpsPublishTimeOfQueryStatus)
        {
            mqtt_client.publish(DpsClient.GetQueryStatusPublishTopic().c_str(), "");
            Log("Client sent operation query message" DLM);
            DpsPublishTimeOfQueryStatus = 0;
        }
    }

    if (!DpsClient.IsAssigned()) return -3;

    mqtt_client.disconnect();

    *hubHost = DpsClient.GetHubHost();
    *deviceId = DpsClient.GetDeviceId();

    Log("Device provisioned:" DLM);
    Log(" Hub host = %s" DLM, hubHost->c_str());
    Log(" Device id = %s" DLM, deviceId->c_str());

    return 0;
}

static void MqttSubscribeCallbackDPS(char* topic, byte* payload, unsigned int length)
{
    Log("Subscribe:" DLM " %s" DLM " %.*s" DLM, topic, length, (const char*)payload);

    if (DpsClient.RegisterSubscribeWork(topic, std::vector<uint8_t>(payload, payload + length)) != 0)
    {
        Log("Failed to parse topic and/or payload" DLM);
        return;
    }

    if (!DpsClient.IsRegisterOperationCompleted())
    {
        const int waitSeconds = DpsClient.GetWaitBeforeQueryStatusSeconds();
        Log("Querying after %u  seconds..." DLM, waitSeconds);

        DpsPublishTimeOfQueryStatus = millis() + waitSeconds * 1000;
    }
}

////////////////////////////////////////////////////////////////////////////////
// Azure IoT Hub

static az_iot_hub_client HubClient;

static int SendCommandResponse(az_iot_hub_client_method_request* request, uint16_t status, az_span response);
static void MqttSubscribeCallbackHub(char* topic, byte* payload, unsigned int length);

static int ConnectToHub(az_iot_hub_client* iot_hub_client, const std::string& host, const std::string& deviceId, const std::string& symmetricKey, const uint64_t& expirationEpochTime)
{
    static std::string deviceIdCache;
    deviceIdCache = deviceId;

    const az_span hostSpan{ az_span_create((uint8_t*)&host[0], host.size()) };
    const az_span deviceIdSpan{ az_span_create((uint8_t*)&deviceIdCache[0], deviceIdCache.size()) };
    az_iot_hub_client_options options = az_iot_hub_client_options_default();
    options.model_id = AZ_SPAN_LITERAL_FROM_STR(IOT_CONFIG_MODEL_ID);
    if (az_result_failed(az_iot_hub_client_init(iot_hub_client, hostSpan, deviceIdSpan, &options))) return -1;

    char mqttClientId[128];
    size_t client_id_length;
    if (az_result_failed(az_iot_hub_client_get_client_id(iot_hub_client, mqttClientId, sizeof(mqttClientId), &client_id_length))) return -4;

    char mqttUsername[256];
    if (az_result_failed(az_iot_hub_client_get_user_name(iot_hub_client, mqttUsername, sizeof(mqttUsername), NULL))) return -5;

    char mqttPassword[300];
    uint8_t signatureBuf[256];
    az_span signatureSpan = az_span_create(signatureBuf, sizeof(signatureBuf));
    az_span signatureValidSpan;
    if (az_result_failed(az_iot_hub_client_sas_get_signature(iot_hub_client, expirationEpochTime, signatureSpan, &signatureValidSpan))) return -2;
    const std::vector<uint8_t> signature(az_span_ptr(signatureValidSpan), az_span_ptr(signatureValidSpan) + az_span_size(signatureValidSpan));
    const std::string encryptedSignature = GenerateEncryptedSignature(symmetricKey, signature);
    az_span encryptedSignatureSpan = az_span_create((uint8_t*)&encryptedSignature[0], encryptedSignature.size());
    if (az_result_failed(az_iot_hub_client_sas_get_password(iot_hub_client, expirationEpochTime, encryptedSignatureSpan, AZ_SPAN_EMPTY, mqttPassword, sizeof(mqttPassword), NULL))) return -3;

    Log("Hub:" DLM);
    Log(" Host = %s" DLM, host.c_str());
    Log(" Device id = %s" DLM, deviceIdCache.c_str());
    Log(" MQTT client id = %s" DLM, mqttClientId);
    Log(" MQTT username = %s" DLM, mqttUsername);
    //Log(" MQTT password = %s" DLM, mqttPassword);

    wifi_client.setCACert(ROOT_CA_BALTIMORE);
    mqtt_client.setBufferSize(MQTT_PACKET_SIZE);
    mqtt_client.setServer(host.c_str(), 8883);
    mqtt_client.setCallback(MqttSubscribeCallbackHub);

    if (!mqtt_client.connect(mqttClientId, mqttUsername, mqttPassword)) return -6;

    mqtt_client.subscribe(AZ_IOT_HUB_CLIENT_METHODS_SUBSCRIBE_TOPIC);
    mqtt_client.subscribe(AZ_IOT_HUB_CLIENT_C2D_SUBSCRIBE_TOPIC);

    return 0;
}

static az_result SendTelemetry()
{
    char telemetry_topic[128];
    if (az_result_failed(az_iot_hub_client_telemetry_get_publish_topic(&HubClient, NULL, telemetry_topic, sizeof(telemetry_topic), NULL)))
    {
        Log("Failed az_iot_hub_client_telemetry_get_publish_topic\n" DLM);
        return AZ_ERROR_NOT_SUPPORTED;
    }

    az_json_writer json_builder;
    char telemetry_payload[200];
    AZ_RETURN_IF_FAILED(az_json_writer_init(&json_builder, AZ_SPAN_FROM_BUFFER(telemetry_payload), NULL));
    AZ_RETURN_IF_FAILED(az_json_writer_append_begin_object(&json_builder));
    AZ_RETURN_IF_FAILED(az_json_writer_append_property_name(&json_builder, AZ_SPAN_LITERAL_FROM_STR(TELEMETRY_PEOPLE_COUNT)));
    AZ_RETURN_IF_FAILED(az_json_writer_append_int32(&json_builder, peopleCount));
    
    AZ_RETURN_IF_FAILED(az_json_writer_append_property_name(&json_builder, AZ_SPAN_LITERAL_FROM_STR(TELEMETRY_PEOPLE_IN)));
    AZ_RETURN_IF_FAILED(az_json_writer_append_int32(&json_builder, peopleIn));
    
    AZ_RETURN_IF_FAILED(az_json_writer_append_property_name(&json_builder, AZ_SPAN_LITERAL_FROM_STR(TELEMETRY_PEOPLE_OUT)));
    AZ_RETURN_IF_FAILED(az_json_writer_append_int32(&json_builder, peopleOut));        
    AZ_RETURN_IF_FAILED(az_json_writer_append_end_object(&json_builder));
    const az_span out_payload{ az_json_writer_get_bytes_used_in_destination(&json_builder)};
        
    static int sendCount = 0;
    if (!mqtt_client.publish(telemetry_topic, az_span_ptr(out_payload), az_span_size(out_payload), false))
    {
        DisplayPrintf("ERROR: Send telemetry %d\n", sendCount);
    }
    else
    {
        ++sendCount;
        DisplayPrintf("Sent telemetry %d\n", sendCount);
    }
    return AZ_OK;
}

static void HandleCommandMessage(az_span payload, az_iot_hub_client_method_request* command_request)
{
    int command_res_code = 200;
    az_result res;

    // Unsupported command
    Log("Unsupported command received: %.*s.\n" DLM, az_span_size(command_request->name), az_span_ptr(command_request->name));

    int rc;
    if ((rc = SendCommandResponse(command_request, 404, AZ_SPAN_LITERAL_FROM_STR("{}"))) != 0)
    {
        Log("Unable to send %d response, status %d\n", 404, rc);
        }
    }

static int SendCommandResponse(az_iot_hub_client_method_request* request, uint16_t status, az_span response)
{
    az_result rc;
    // Get the response topic to publish the command response
    char commands_response_topic[128];
    if (az_result_failed(rc = az_iot_hub_client_methods_response_get_publish_topic(&HubClient, request->request_id, status, commands_response_topic, sizeof(commands_response_topic), NULL)))
    {
        Log("Unable to get method response publish topic\n" DLM);
        return rc;
    }

    Log("Status: %u\tPayload: '", status);
    char* payload_char = (char*)az_span_ptr(response);
    if (payload_char != NULL)
    {
        for (int32_t i = 0; i < az_span_size(response); i++)
        {
            Log("%c", *(payload_char + i));
        }
    }
    Log("'" DLM);

    // Send the commands response
    if (mqtt_client.publish(commands_response_topic, az_span_ptr(response), az_span_size(response), false))
    {
        Log("Sent response\n" DLM);
    }

    return rc;
}

static void MqttSubscribeCallbackHub(char* topic, byte* payload, unsigned int length)
{
    az_span topic_span = az_span_create((uint8_t *)topic, strlen(topic));
    az_iot_hub_client_method_request command_request;

    if (az_result_succeeded(az_iot_hub_client_methods_parse_received_topic(&HubClient, topic_span, &command_request)))
    {
        DisplayPrintf("Command arrived!");
        // Determine if the command is supported and take appropriate actions
        HandleCommandMessage(az_span_create(payload, length), &command_request);
    }

    Log(DLM);
}


////////////////////////////////////////////////////////////////////////////////
// setup and loop

void setup()
{       
    ////////////////////
    // Load storage

    Storage::Load();

    ////////////////////
    // Init I/O

    Serial.begin(115200);

    pinMode(WIO_KEY_A, INPUT_PULLUP);
    pinMode(WIO_KEY_B, INPUT_PULLUP);
    pinMode(WIO_KEY_C, INPUT_PULLUP);

    ////////////////////
    // Init display
    lv_init();
    tft.begin();
    tft.setRotation(3);
    
    lv_disp_buf_init(&disp_buf, buf, NULL, LV_HOR_RES_MAX * 10);
    lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = 320;
    disp_drv.ver_res = 240;
    disp_drv.flush_cb = my_disp_flush;
    disp_drv.buffer = &disp_buf;
    lv_disp_drv_register(&disp_drv);

    lv_buttons();
    
    ////////////////////
    // Enter configuration mode

    if (digitalRead(WIO_KEY_A) == LOW &&
        digitalRead(WIO_KEY_B) == LOW &&
        digitalRead(WIO_KEY_C) == LOW   )
    {
        DisplayPrintf("In configuration mode");
        while (!Serial) {delay(10);}
        CliMode();
    }

    ////////////////////
    // Connect Wi-Fi

    DisplayPrintf("Connecting to SSID: %s", IOT_CONFIG_WIFI_SSID);
    do
    {
        Log(".");
        WiFi.begin(IOT_CONFIG_WIFI_SSID, IOT_CONFIG_WIFI_PASSWORD);
        delay(500);
    }
    while (WiFi.status() != WL_CONNECTED);
    DisplayPrintf("Connected");

    ////////////////////
    // Sync time server

    ntp.begin();

    ////////////////////
    // Provisioning

#if defined(USE_CLI) || defined(USE_DPS)

    if (RegisterDeviceToDPS(IOT_CONFIG_GLOBAL_DEVICE_ENDPOINT, IOT_CONFIG_ID_SCOPE, IOT_CONFIG_REGISTRATION_ID, IOT_CONFIG_SYMMETRIC_KEY, ntp.epoch() + TOKEN_LIFESPAN, &HubHost, &DeviceId) != 0)
    {
        Abort("RegisterDeviceToDPS()");
    }

#else

    HubHost = IOT_CONFIG_IOTHUB;
    DeviceId = IOT_CONFIG_DEVICE_ID;

#endif // USE_CLI || USE_DPS

    mqtt_connect();

    if (EI_CLASSIFIER_RAW_SAMPLES_PER_FRAME != axis_num) {
        Log("ERR: EI_CLASSIFIER_RAW_SAMPLES_PER_FRAME should be equal to (%d) (the (%d) sensor axes)\n", axis_num, axis_num);
        return;
    }
    
    // Create the threads that will be managed by the rtos
    // Sets the stack size and priority of each task
    // Also initializes a handler pointer to each task, which are important to communicate with and retrieve info from tasks
    
    xTaskCreate(lv_tick_task, "LVGL Tick", 128, NULL, tskIDLE_PRIORITY + 1, &Handle_aTask); 
    xTaskCreate(run_inference_background, "Inference", 512, NULL, tskIDLE_PRIORITY + 2, &Handle_bTask);
    xTaskCreate(read_data, "Data collection", 2048, NULL, tskIDLE_PRIORITY + 3, &Handle_cTask); 
    
}

void mqtt_connect(){
  
  static uint64_t reconnectTime;

  if (!mqtt_client.connected()) {
    
    DisplayPrintf("Connecting to Azure IoT Hub...");
    const uint64_t now = ntp.epoch();
    if (ConnectToHub(&HubClient, HubHost, DeviceId, IOT_CONFIG_SYMMETRIC_KEY, now + TOKEN_LIFESPAN) != 0)
    {
        DisplayPrintf("#ff00ff > ERROR.#");
        Log("> ERROR. Status code =%d. Try again in 5 seconds.\n" DLM, mqtt_client.state());
        delay(5000);
        mqtt_connect();
    }

    DisplayPrintf("#0000ff > SUCCESS.#");
    reconnectTime = now + TOKEN_LIFESPAN * 0.85;
    }
    
    else {
      if ((uint64_t)ntp.epoch() >= reconnectTime)
      {
          DisplayPrintf("Disconnect");
          mqtt_client.disconnect();
          mqtt_connect();
      }
    }
}

/**
 * @brief      Run inferencing in the background.
 */
static void run_inference_background(void* pvParameters)
{   
    // wait until we have a full buffer
    delay((EI_CLASSIFIER_INTERVAL_MS * EI_CLASSIFIER_RAW_SAMPLE_COUNT) + 100);

    // This is a structure that smoothens the output result
    // With the default settings 70% of readings should be the same before classifying.
    ei_classifier_smooth_t smooth;
    ei_classifier_smooth_init(&smooth, 3 /* no. of readings */, 3 /* min. readings the same */, 0.5 /* min. confidence */, 0.3 /* max anomaly */);

    while (1) {
        // copy the buffer
        memcpy(inference_buffer, buffer, EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE * sizeof(float));

        // Turn the raw buffer in a signal which we can the classify
        signal_t signal;
        int err = numpy::signal_from_buffer(inference_buffer, EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE, &signal);
        if (err != 0) {
            Log("Failed to create signal from buffer (%d)\n", err);
            return;
        }

        // Run the classifier
        ei_impulse_result_t result = {0};

        err = run_classifier(&signal, &result, debug_nn);
        if (err != EI_IMPULSE_OK) {
            Log("ERR: Failed to run classifier (%d)\n", err);
            return;
        }

        // print the predictions
        Log("Predictions ");
        Log("(DSP: %d ms., Classification: %d ms., Anomaly: %d ms.)",result.timing.dsp, result.timing.classification, result.timing.anomaly);
        Log(": ");

        // ei_classifier_smooth_update yields the predicted label
        const char *prediction = ei_classifier_smooth_update(&smooth, &result);
        Log("%s ", prediction);

        if (prediction != prev_prediction)
        {
        if (prediction == "out") {peopleOut++; DisplayPrintf("#ff00ff Person left#");}
        if (prediction == "in") {peopleIn++; DisplayPrintf("#0000ff Person entered#");}
        prev_prediction = prediction;
        update_screen();
        }
        
        // print the cumulative results
        Log(" [ ");
        for (size_t ix = 0; ix < smooth.count_size; ix++) {
            Log("%u", smooth.count[ix]);
            if (ix != smooth.count_size + 1) {
                Log(", ");
            }
            else {
              Log(" ");
            }
        }
        Log("]\n");

        delay(run_inference_every_ms);
    }

    ei_classifier_smooth_free(&smooth);
}

/**
* @brief      Get data and run inferencing
*
* @param[in]  debug  Get debug info if true
*/
static void read_data(void* pvParameters)
{   
    while (1) {
        // Determine the next tick (and then sleep later)
        uint64_t next_tick = micros() + (EI_CLASSIFIER_INTERVAL_MS * 1000);

        // roll the buffer -axis_num points so we can overwrite the last one
        numpy::roll(buffer, EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE, -axis_num);
        
        distance = ultrasonic.MeasureInCentimeters();
        if (distance > 200.0) { distance = -1;}
        
        buffer[EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE - 1] = distance;

        // and wait for next tick
        uint64_t time_to_wait = next_tick - micros();
        delay((int)floor((float)time_to_wait / 1000.0f));
        delayMicroseconds(time_to_wait % 1000);
    }
}

static void lv_tick_task(void* pvParameters) {
    while(1){
      lv_tick_inc(LVGL_TICK_PERIOD);
      delay(LVGL_TICK_PERIOD);
    }
}

void update_screen()
{   
  peopleCount = peopleIn - peopleOut;
  lv_label_set_text_fmt(peopleInLabel, "%d", peopleIn);
  lv_label_set_text_fmt(peopleOutLabel, "%d", peopleOut);
  lv_label_set_text_fmt(peopleNumLabel, "%d", peopleCount);    
  lv_task_handler();
}

void loop()
{
  mqtt_connect();
  mqtt_client.loop();
  SendTelemetry();
  delay(TELEMETRY_FREQUENCY_MILLISECS);
}
