#include <WebServer.h>
#include <Preferences.h>
#include <HTTPClient.h>

// Preferences for flash storage
Preferences preferences;

// Webserver
WebServer server(80);

void server_ota();
bool updateInProgress = false;
int updateProgress = 0;
String updateStatus = "No update in progress";
bool otaEnabled = false;
unsigned long otaEnableTime=0;

#define MAX_MACIDS 5
#define MAX_TOUCH_BTNS 4
// Match Load Channels [1,2,3,4] in order: U11, U8, U5, U7
// Fixed order to match UI load channels 1,2,3,4
// Order: U11, U8, U5, U7
uint16_t touch_key_data_mapping[] = {
  4096,   // U11 → Channel 1
  1024,   // U8  → Channel 2
  1,      // U5  → Channel 3
  4       // U7  → Channel 4
};




String macIds[MAX_MACIDS]={"f8:dc:7a:42:82:b1","","","",""};
String jwtTokens[MAX_MACIDS] = {"","","","",""};
boolean staLedUpdated = true;
boolean loop_wifi_enable =false;
boolean loop_mqtt_enable =false;
boolean loop_ota_enable = true;  //default enable
// Configuration structure
struct TouchButtonConfig {
    int macIDNum;
    int channels[16];
    int target_level;
    int high_target_level;
    int fade_time;
    bool prevState;
};

// Array of configurations for 10 touch buttons
TouchButtonConfig configs[MAX_TOUCH_BTNS];
int currTouchIndex = -1;

// Function to save configurations to Preferences
void saveConfigs() {
    preferences.begin("touch-configs", false);
    for (int i = 0; i < MAX_TOUCH_BTNS; i++) {
        String key = "config" + String(i);
        String json = "";
        StaticJsonDocument<512> doc;
        doc["macIDNum"] = configs[i].macIDNum;
        JsonArray channels = doc.createNestedArray("channels");
        for (int ch = 0; ch < 16; ch++) {
            channels.add(configs[i].channels[ch]);
        }
        doc["target_level"] = configs[i].target_level;
        doc["high_target_level"] = configs[i].high_target_level;
        doc["fade_time"] = configs[i].fade_time;
        serializeJson(doc, json);
        //Serial.println(json);
        preferences.putString(key.c_str(), json);
    }
    preferences.end();
}

void handleSaveMacIds() {
  bool changed = false;
  preferences.begin("touch-configs", false);
  for(int i = 0; i < MAX_MACIDS; i++) {
    char macKey[8], jwtKey[8];
    snprintf(macKey, sizeof(macKey), "mac%d", i);
    snprintf(jwtKey, sizeof(jwtKey), "jwt%d", i);
    
    String newMac = server.arg("mac" + String(i));
    String newJwt = server.arg("jwt" + String(i));
    
    // Only update if either MAC ID or JWT has changed
    if(newMac != macIds[i] || newJwt != jwtTokens[i]) {
      macIds[i] = newMac;
      jwtTokens[i] = newJwt;
      preferences.putString(macKey, newMac.c_str());
      preferences.putString(jwtKey, newJwt.c_str());
      changed = true;
    }
  }
  preferences.end();
  if(changed) {
    server.send(200, "text/html", "<html><body><h2>MAC IDs Updated</h2><p><a href='/'>Back to Home</a></p></body></html>");
  } else {
    server.send(200, "text/html", "<html><body><h2>No Changes Made</h2><p><a href='/'>Back to Home</a></p></body></html>");
  }
}

// Function to load configurations from Preferences
void loadConfigs() {
    preferences.begin("touch-configs", true);
    for (int i = 0; i < MAX_TOUCH_BTNS; i++) {
        String key = "config" + String(i);
        String json = preferences.getString(key.c_str(), "{}");
        StaticJsonDocument<512> doc;
        deserializeJson(doc, json);
        //Serial.println(json);
        configs[i].macIDNum = doc["macIDNum"] | 0;
        for (int ch = 0; ch < 16; ch++) {
            configs[i].channels[ch] = doc["channels"][ch] | 0;
        }
        configs[i].target_level = doc["target_level"] | 100;
        configs[i].high_target_level = doc["high_target_level"] | 70;
        configs[i].fade_time = doc["fade_time"] | 1;
    }
    
    for(int i = 0; i < MAX_MACIDS; i++) {
      char macKey[8], jwtKey[8];
      snprintf(macKey, sizeof(macKey), "mac%d", i);
      snprintf(jwtKey, sizeof(jwtKey), "jwt%d", i);
      
      String storedMac = preferences.getString(macKey);
      String storedJwt = preferences.getString(jwtKey);
      
      if(storedMac.length() > 0) {
        macIds[i] = storedMac;
      }
      if(storedJwt.length() > 0) {
        jwtTokens[i] = storedJwt;
      }
    }
    //load MACID's
    preferences.end();
}

// Function to generate HTML for configuration cards
String generateHTML() {
    String html = R"rawliteral(
        <!DOCTYPE html>
        <html lang="en">
        <head>
            <meta name="viewport" content="width=device-width, initial-scale=1.0">
            <style>
                body { font-family: Arial, sans-serif; margin: 20px; }
                .card { border: 1px solid #ccc; border-radius: 8px; padding: 16px; margin: 16px 0; }
                .edit-btn { background-color: #007BFF; color: white; border: none; padding: 10px 20px; cursor: pointer; border-radius: 5px; }
                .edit-btn:hover { background-color: #0056b3; }
            </style>
        </head>
        <body>
            <h2>Touch Button Configurations</h2>
    )rawliteral";

    for (int i = 0; i < MAX_TOUCH_BTNS; i++) {
        html += "<div class='card'>";
        html += "<h3>Touch Button " + String(i + 1) + "</h3>";
        html += "<p>MAC ID: " + String(configs[i].macIDNum) + "</p>";
        html += "<p>Target Level: " + String(configs[i].target_level) + "</p>";
        html += "<p>Fade Time: " + String(configs[i].fade_time) + " sec</p>";
        html += "<p>Channels: [";
        for (int ch = 0; ch < 16; ch++) {
            if (configs[i].channels[ch] > 0) {
                html += String(configs[i].channels[ch]) + ",";
            }
        }
        html += "]</p>";
        html += "<button class='edit-btn' onclick='editConfig(" + String(i) + ")'>Edit</button>";
        html += "</div>";
    }

    html += R"rawliteral(
            <button onclick="location.href='/'" style="padding: 10px 20px; font-size: 16px; cursor: pointer;">Home</button>
            <script>
                function editConfig(index) {
                    let url = "/edit?index=" + index;
                    fetch(url)
                        .then(response => response.text())
                        .then(data => {
                            document.body.innerHTML = data;
                        });
                }
            </script>
        </body>
        </html>
    )rawliteral";    

    return html;
}


// Function to handle root URL
void handleTouchSettings() {
  if(!server.authenticate(http_username, http_password)) {
    return server.requestAuthentication();
  }
    server.send(200, "text/html", generateHTML());
}

// Function to handle editing of configurations
void handleEdit() {
  if(!server.authenticate(http_username, http_password)) {
    return server.requestAuthentication();
  }
    int index = server.arg("index").toInt();
    if (index < 0 || index >= MAX_TOUCH_BTNS) {
        server.send(400, "text/plain", "Invalid index");
        return;
    }

    String form = R"rawliteral(
    <!DOCTYPE html>
    <html lang="en">
    <head>
        <meta name="viewport" content="width=device-width, initial-scale=1.0">
        <style>
            body { font-family: Arial, sans-serif; margin: 20px; }
            form { display: flex; flex-direction: column; }
            label { margin: 8px 0; }
            input, .channels { padding: 8px; margin-bottom: 16px; }
            .channels { display: grid; grid-template-columns: repeat(4, 1fr); gap: 10px; }
            .channel-box { display: flex; align-items: center; }
            button { background-color: #007BFF; color: white; border: none; padding: 10px 20px; cursor: pointer; border-radius: 5px; }
            button:hover { background-color: #0056b3; }
        </style>
    </head>
    <body>
        <h2>Edit Configuration</h2>
        <form action="/save" method="POST">
)rawliteral";

//form += "<label>MAC ID:</label><input type='number' name='macIDNum' value='" + String(configs[index].macIDNum) + "' required>";
form += "<style>body{font-family:Arial;margin:20px;} select,button{padding:8px;margin:5px;}</style>";
form += "</head><body>";
form += "<h2>Select MAC ID</h2>";
form += "<form action='/select' method='POST'>";
form += "<select name='macIndex'>";
for(int i = 0; i < MAX_MACIDS; i++) {
  if(macIds[i] != "") {
    form += "<option value='" + String(i) + "'>" + macIds[i] + "</option>";
  }
}
form += "</select>";
form += "<label>Target Level:</label><input type='number' name='target_level' value='" + String(configs[index].target_level) + "' required>";
form += "<label>Fade Time:</label><input type='number' name='fade_time' value='" + String(configs[index].fade_time) + "' required>";
form += "<label>Channels:</label><div class='channels'>";

for (int ch = 0; ch < 16; ch++) {
    bool isSelected = configs[index].channels[ch] > 0;
    form += "<div class='channel-box'><input type='checkbox' name='channels' value='" + String(ch + 1) + "'";
    if (isSelected) {
        form += " checked";
    }
    form += "> Channel " + String(ch + 1) + "</div>";
}

form += "</div>";
form += "<input type='hidden' name='index' value='" + String(index) + "'>";
form += "<button type='submit'>Save</button>";
form += "</form></body></html>";


    server.send(200, "text/html", form);
}

// Function to handle saving of configurations
void handleSave() {
    if(!server.authenticate(http_username, http_password)) {
    return server.requestAuthentication();
  }
    int index = server.arg("index").toInt();
    if (index < 0 || index >= MAX_TOUCH_BTNS) {
        server.send(400, "text/plain", "Invalid index");
        return;
    }

    configs[index].macIDNum = server.arg("macIndex").toInt();
    
    Serial.print("Selected MAC ID Index: ");
    Serial.println(configs[index].macIDNum);
    Serial.print("Selected MAC ID: ");
    Serial.println(macIds[configs[index].macIDNum]);

    configs[index].target_level = server.arg("target_level").toInt();
    configs[index].fade_time = server.arg("fade_time").toInt();

    String selectedChannels = server.arg("channels"); // Handle multiple selections
    for (int ch = 0; ch < 16; ch++) {
        configs[index].channels[ch] = 0; // Reset channels
    }
    
    int numArgs = server.args();
    for (int i = 0; i < numArgs; i++) {
        if (server.argName(i) == "channels") {
            int channel = server.arg(i).toInt();
            if (channel >= 1 && channel <= 16) {
                configs[index].channels[channel - 1] = channel;
            }
        }
    }
    
    saveConfigs();
    server.sendHeader("Location", "/touchcfg");
    server.send(303);
}


//------------------------------

void WiFiEvent(WiFiEvent_t event) {
    Serial.printf("[WiFi-event] event: %d\n", event);
    switch(event) {
        case ARDUINO_EVENT_WIFI_STA_GOT_IP:
            Serial.println("WiFi connected");
            Serial.println("IP address: ");
            Serial.println(WiFi.localIP());

            // ✅ Exit pairing mode
            if (gotoApmode) {
                Serial.println("Exiting AP Mode...");
                gotoApmode = false;
            }

            staLedUpdated = setStaled(WIFI_CONNECTED); // GREEN
            globalStatusLED = WIFI_CONNECTED;

            loop_mqtt_enable = true;
            mqtt_status = HTTP_GET_CRED;

            server_routes_setup();  // if needed for web config
            break;

        case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
            Serial.println("WiFi lost connection");
            staLedUpdated = setStaled(WIFI_DISCONNECT); // RED
            globalStatusLED = WIFI_DISCONNECT;
            loop_mqtt_enable = false;
            break;
    }
}


void loadPreferences() {
    preferences.begin("mqtt", true);
    mqtt_server = preferences.getString("mqttServer", "192.168.1.102");
    mqtt_port = preferences.getUInt("mqttPort", 1883);
    mqtt_user = preferences.getString("mqttUser", "integration_user");
    mqtt_password = preferences.getString("mqttPassword", "wgv^TZ?MP613KpdL");
    mqtt_macid = preferences.getString("mqttMacID", "f8:dc:7a:42:82:b1");

    http_user = preferences.getString("httpUser", "xpoeclient");
    http_pass = preferences.getString("httpPass", "xpoepass");

    dimSteps = preferences.getUInt("dimSteps", 10);
    touch1_dimcounter = dimSteps;
    dimTickerMillis = preferences.getUInt("dimDelay", 1200);
        
    preferences.end();

    //make required variables
    mqtt_topic_pub = "switch/" + String(mqtt_macid) + "/api";
    mqtt_topic_sub = "switch/+/api/out";

    login_url = "http://" + String(mqtt_server) + "/api/login";
    login_body = "{\"username\":\"" + String(http_user) + "\",\"password\":\""+ String(http_pass) +"\"}";
    
    preferences.begin("wifi", true);
    wifiSSID = preferences.getString("wifiSSID", "JOSHITHA_EXT");
    wifiPassword = preferences.getString("wifiPassword", "saradadevi");

    useStaticIP = preferences.getBool("useStaticIP", false);
    staticIP = preferences.getString("staticIP", "192.168.2.10");
    gateway = preferences.getString("gateway", "192.168.2.1");
    subnet = preferences.getString("subnet", "255.255.255.0");

    
    preferences.end();
}

void IRAM_ATTR connectToWifi() {
  Serial.println("Connecting to Wi-Fi...");
  WiFi.mode(WIFI_STA);
  Serial.println(useStaticIP);
  Serial.println(staticIP);
  Serial.println(subnet);
  if(useStaticIP)
  {
    if (staticIP != "" && gateway != "" && subnet != "") {
        IPAddress localIP, localGateway, localSubnet;
        localIP.fromString(staticIP);
        localGateway.fromString(gateway);
        localSubnet.fromString(subnet);
        WiFi.config(localIP, localGateway, localSubnet);
        Serial.println("Using Static IP Configuration.");
      }
  }
  else {
      Serial.println("Using Dynamic IP (DHCP).");
  }
  Serial.println("Connecting to WiFi...");
  WiFi.begin(wifiSSID, wifiPassword);
}

boolean isAPMode=false;
void setupAPMode() {
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ap_ssid, ap_password);
  
  Serial.println("AP Mode activated");
  Serial.print("AP IP address: ");
  Serial.println(WiFi.softAPIP());
  isAPMode = true;
  server_routes_setup();
  staLedUpdated=setStaled(WIFI_SOFTAP);
  globalStatusLED = WIFI_SOFTAP;
}

boolean setup_wifi_mqtt()
{
  loadConfigs();
  loadPreferences();
  WiFi.onEvent(WiFiEvent);
  connectToWifi();
  return true;
}

void loop_wifi()
{
  
  if (!loop_wifi_enable)
    return;
  if(!WiFi.isConnected())
  {
    loop_ota_enable =false;
    loop_mqtt_enable =false;
  }
  else
  {
    //Anything to handle here?
  }
}

unsigned long lastCheckAttempt=0;
unsigned long lastCheckAttempt1=0;
void loop_mqtt()
{ 
  if (!loop_mqtt_enable)
      return;
  if(mqtt_status==HTTP_GET_CRED)
  {
    if (millis()-lastCheckAttempt > 2000) {
      lastCheckAttempt = millis();
      // Attempt to reconnect
      if(getJWTToken()){
        mqtt_status=MQTT_START_CONNECT;
        lastCheckAttempt = 0;
      }
    }
  }
  else if(mqtt_status==MQTT_START_CONNECT)
  {
     Serial.println("Start MQTT Timer");
     connectMQTT();
     mqtt_status=MQTT_DO_NOTHING;
  }
  else
  {
      if(!client.connected())
      {
        long now = millis();
        if (now-lastCheckAttempt1 > 20000) {
          Serial.println("Re connect MQTT....");
          connectMQTT();
          lastCheckAttempt1 = millis();
        }
        staLedUpdated =setStaled(MQTT_DISCONNECT);
        globalStatusLED = MQTT_DISCONNECT;
      }
      else
      {
        client.loop();
      }
   }
}

void loop_wifi_mqtt()
{
   if(gotoApmode){
    setupAPMode();
    gotoApmode = false;
  }
  
  if(staLedUpdated)
  {
    flushLed();
    staLedUpdated=false;
  }
  
  if(!isAPMode)
  {
    loop_wifi();
    loop_mqtt();
  }

  server.handleClient();
}
//--------------setup ----
void touch_route_setup() {
    // server routes
    server.on("/touchcfg", handleTouchSettings);
    server.on("/edit", handleEdit);
    server.on("/save", HTTP_POST, handleSave);

    
}


void server_routes_setup()
{
  server_ota();
  touch_route_setup();
  other_routes_setup();
  server.begin();
  Serial.println("HTTP server started");
}

