// WiFi and MQTT client objects
WiFiClient espClient;
PubSubClient client(espClient);

String wifiSSID;
String wifiPassword;

bool useStaticIP = false;
String staticIP = "";
String gateway = "";
String subnet = "";

String Sversion = "V2.0";

const char* ap_ssid = "PROCOM_AP";
const char* ap_password = "12345678";
boolean gotoApmode =false;

const char* http_username = "admin";
const char* http_password = "admin";

int dimSteps = 10;
int touch1_dimcounter=10;
int dimTickerMillis=1200;

enum sta_led
{
  WIFI_DISCONNECT,
  WIFI_CONNECTED,
  MQTT_DISCONNECT,
  MQTT_CONNECT,
  WIFI_SOFTAP
};

enum
{
  MQTT_DO_NOTHING,
  HTTP_GET_CRED,
  MQTT_START_CONNECT,
};
static int mqtt_status=0;

void processSerialCommand(String);
boolean setStaled(int led_status);
volatile uint8_t globalStatusLED=false;
boolean getJWTToken(void);
void connectMQTT(void);
void myServer_on_setup();
void flushLed();
void other_routes_setup();
void server_routes_setup();
//boolean mqttDisconnectStatus =false;
// MQTT broker details
String mqtt_server = "192.168.1.2";
int mqtt_port = 1883;
String mqtt_user = "integration_user";
String mqtt_password = "wgv^TZ?MP613KpdL";
String mqtt_macid ="f8:dc:7a:42:82:b1";

String http_user = "xpoeclient";
String http_pass = "xpoepass";

String channel_state_topic = "state/+/channels";
//Venu -> Below are configurable Dynamically
String mqtt_topic_pub = "switch/f8:dc:7a:42:82:b1/api";
String mqtt_topic_sub = "switch/+/api/out";

// HTTP endpoint for JWT token
String login_url = "http://" + String(mqtt_server) + "/api/login";
String login_body = "{\"username\":\"xpoeclient\",\"password\":\"xpoepass\"}";

bool master_pub =true;
// JWT token
String jwt_token = "";

String SlaveJwt ="";
