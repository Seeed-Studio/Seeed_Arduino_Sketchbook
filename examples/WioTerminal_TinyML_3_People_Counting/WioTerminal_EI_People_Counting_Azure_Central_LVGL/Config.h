#define USE_CLI
//#define USE_DPS

#if defined(USE_CLI)

// Wi-Fi
#define IOT_CONFIG_WIFI_SSID				Storage::WiFiSSID.c_str()
#define IOT_CONFIG_WIFI_PASSWORD			Storage::WiFiPassword.c_str()

// Azure IoT Hub DPS
#define IOT_CONFIG_GLOBAL_DEVICE_ENDPOINT	"global.azure-devices-provisioning.net"
#define IOT_CONFIG_ID_SCOPE					Storage::IdScope
#define IOT_CONFIG_REGISTRATION_ID			Storage::RegistrationId
#define IOT_CONFIG_SYMMETRIC_KEY			Storage::SymmetricKey
#define IOT_CONFIG_MODEL_ID					"dtmi:peopleCounterWintermute:newk7:peopleCount;1"

#else // USE_CLI

// Wi-Fi
#define IOT_CONFIG_WIFI_SSID				"[wifi ssid]"
#define IOT_CONFIG_WIFI_PASSWORD			"[wifi password]"

#if !defined(USE_DPS)
// Azure IoT Hub
#define IOT_CONFIG_IOTHUB					"[Azure IoT Hub host name].azure-devices.net"
#define IOT_CONFIG_DEVICE_ID				"[device id]"
#define IOT_CONFIG_SYMMETRIC_KEY			"[symmetric key]"
#define IOT_CONFIG_MODEL_ID					"dtmi:peopleCounterWintermute:newk7:peopleCount;1"
#else // USE_DPS

// Azure IoT Hub DPS
#define IOT_CONFIG_GLOBAL_DEVICE_ENDPOINT	"global.azure-devices-provisioning.net"
#define IOT_CONFIG_ID_SCOPE					"[id scope]"
#define IOT_CONFIG_REGISTRATION_ID			"[registration id]"
#define IOT_CONFIG_SYMMETRIC_KEY			"[symmetric key]"
#define IOT_CONFIG_MODEL_ID					"dtmi:peopleCounterWintermute:newk7:peopleCount;1"
#endif // USE_DPS

#endif // USE_CLI

#define TOKEN_LIFESPAN                      3600
#define TELEMETRY_FREQUENCY_MILLISECS    5000

#define TELEMETRY_PEOPLE_COUNT				"peopleCount"
#define TELEMETRY_PEOPLE_IN           "peopleIn"
#define TELEMETRY_PEOPLE_OUT          "PeopleOut"
