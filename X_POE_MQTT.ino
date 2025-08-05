//Includes
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "common_header.h"
#include <Update.h>
#include "my_webserver.h"
#include "index_html.h"
#include "TickTwo.h"
#include <Wire.h> 
#include "led_interface.h"

#include "bt_touch_interface.h"
#include <IRremoteESP8266.h>
#include <IRrecv.h>
#include <IRutils.h>

#define RECV_PIN 35
IRrecv irrecv(RECV_PIN);           // ✅ This is the correct IRrecv declaration
decode_results results;


//Definitions
#define TOUCH_2_MODULE
#define I2C_SDA 21
#define I2C_SCL 22
#define IR_PWR 0xF714EB  // Red Power button (master toggle)
#define IR_1   0xF715EA  // U11 → Touch Index 0
#define IR_2   0xF716E9  // U8  → Touch Index 1
#define IR_3   0xF70EF1  // U5  → Touch Index 2
#define IR_4   0xF70DF2  // U7  → Touch Index 3


//User Configurable Default Variables
int fade_time= 0; //global Fade Time
//bool lastOnStates[MAX_TOUCH_BTNS] = {false, false, false, false};

//-------------- Function Implimentations --------
//Need to impliment it for Slave LED's Status During bootup
void get_xpoe_status()
{
  StaticJsonDocument<1024> jsonDoc;
  String mqttCommand;
  jsonDoc["data"]["url"] = "level";
  jsonDoc["data"]["token"] = jwt_token;
  jsonDoc["data"]["method"] = "GET";
  
  serializeJson(jsonDoc, mqttCommand);
  sendMQTTCommand(mqttCommand.c_str());
}



void connectMQTT() {
  client.setServer(mqtt_server.c_str(), mqtt_port);
  client.setCallback(mqttCallback);
  String clientId = "ESP32_" + String(WiFi.macAddress());

  Serial.print("Connecting to MQTT...");
  if (client.connect(clientId.c_str(), mqtt_user.c_str(), mqtt_password.c_str())) {
    Serial.println("connected");
    client.subscribe(mqtt_topic_sub.c_str());
    client.subscribe(channel_state_topic.c_str());

    //Do get Current status of the X-POE -> VENU move this to loop call in case needed
    get_xpoe_status();

    staLedUpdated =setStaled(MQTT_CONNECT);//update STA LED to Blue
    globalStatusLED = MQTT_CONNECT;
    
  
  } else {
    Serial.print("failed, rc=");
    Serial.println(client.state());
    delay(2000);
  }
}

boolean getJWTToken() {
  HTTPClient http;
  boolean JWTStatus=false;
  Serial.println(login_url);
  Serial.println(login_body);
  http.begin(login_url.c_str());
  http.addHeader("Content-Type", "application/json");

  int httpResponseCode = http.POST(login_body.c_str());
  String payload = "";

  if (httpResponseCode == 200) {
    payload = http.getString();
    Serial.println(payload);
    StaticJsonDocument<1024> jsonDoc;
    deserializeJson(jsonDoc, payload);
    jwt_token = jsonDoc["data"]["access_token"].as<String>();
    Serial.println("JWT Token refreshed: " + jwt_token);
    JWTStatus=true;
  } else {
    Serial.print("Error fetching JWT Token, HTTP response code: ");
    Serial.println(httpResponseCode);
  }

  http.end();
  return JWTStatus;
}

void sendMQTTCommand(const char* payload) {
  Serial.println(String(payload));
  Serial.println(mqtt_topic_pub);
  if (client.publish(mqtt_topic_pub.c_str(), payload)) {
    Serial.println("MQTT Command sent successfully");
  } else {
    Serial.println("Failed to send MQTT Command");
  }
}

String parseLevelString(String input) {
  // Remove "LEVEL " from the beginning of the string
  
  // Create a JSON array string
  String jsonArray = "[";
  
  // Split the string by comma
  int startIndex = 0;
  int commaIndex = input.indexOf(',');
  
  while (commaIndex != -1) {
    // Extract the substring between start and comma
    String level = input.substring(startIndex, commaIndex);
    
    // Add to JSON array
    jsonArray += level;
    
    // Add comma if not last element
    if (commaIndex < input.length() - 1) {
      jsonArray += ",";
    }
    
    // Move to next segment
    startIndex = commaIndex + 1;
    commaIndex = input.indexOf(',', startIndex);
  }
  
  // Add last element if exists
  if (startIndex < input.length()) {
    if (jsonArray.charAt(jsonArray.length() - 1) != '[') {
      jsonArray += ",";
    }
    jsonArray += input.substring(startIndex);
  }
  
  // Close JSON array
  jsonArray += "]";
  
  return jsonArray;
}



JsonArray parseChannelString(String input) {
  // Create a static JsonDocument with sufficient size
  static StaticJsonDocument<200> doc;
  
  // Clear the previous content of the document
  doc.clear();
  
  // Create a JSON array
  JsonArray channels = doc.createNestedArray();
  
  // Split the string by comma
  int startIndex = 1;
  int commaIndex = input.indexOf(',');
  //Serial.println(input);
  
  while (commaIndex != -1) {
    // Extract the substring between start and comma
    String level = input.substring(startIndex, commaIndex);
    //Serial.println(level);
    // Add to JSON array (convert to integer)
    channels.add(level.toInt());
    
    // Move to next segment
    startIndex = commaIndex + 1;
    commaIndex = input.indexOf(',', startIndex);
  }
  
  // Add last element if exists
  if (startIndex < input.length()) {
    channels.add(input.substring(startIndex).toInt());
  }
  return channels;
}

void processSerialCommand(String command) {
  StaticJsonDocument<1024> jsonDoc;
  String mqttCommand;

  jsonDoc["data"]["url"] = "level";
  if(master_pub){
    jsonDoc["data"]["token"] = jwt_token;
  }else{
    jsonDoc["data"]["token"] = SlaveJwt;
  }
  
  jsonDoc["data"]["method"] = "POST";

  if (command.startsWith("ON")) {
    //int channel = command.substring(3).toInt();
    Serial.println("ON Command****");
    String channel =command.substring(3);
    Serial.println(channel);
    jsonDoc["data"]["payload"]["target_level"] = "on";
    jsonDoc["data"]["payload"]["fade_time"] = fade_time;
    JsonArray channels= parseChannelString(channel);
    jsonDoc["data"]["payload"].createNestedArray("channels");
    jsonDoc["data"]["payload"]["channels"] = channels;
  } else if (command.startsWith("OFF")) {
    //int channel = command.substring(4).toInt();
    String channel =command.substring(4);
    //jsonDoc["data"]["payload"]["channels"] = parseChannelString(channel);
    jsonDoc["data"]["payload"]["target_level"] ="off";
    jsonDoc["data"]["payload"]["fade_time"] = fade_time;
    JsonArray channels= parseChannelString(channel);
    jsonDoc["data"]["payload"].createNestedArray("channels");
    jsonDoc["data"]["payload"]["channels"] = channels;
  } else if (command.startsWith("LEVEL")) {
    // Trim and clean the command for safety
    command.trim();
    
    // Find the first and second spaces in the command
    int firstSpace = command.indexOf(' ');
    int secondSpace = command.indexOf(' ', firstSpace + 1);
    
    // Validate that the command has both spaces
    if (firstSpace == -1 || secondSpace == -1) {
      Serial.println("Invalid LEVEL command format. Use: LEVEL <channel> <level>");
      return;
    }
    
    // Extract the channel and level values
    //int channel = command.substring(firstSpace + 1, secondSpace).toInt();
    String channel = command.substring(firstSpace + 1, secondSpace);
    int level = command.substring(secondSpace + 1).toInt();
    
    // Populate the JSON object
    //jsonDoc["data"]["payload"]["channels"] = parseChannelString(channel);
    jsonDoc["data"]["payload"]["target_level"] = level;
    jsonDoc["data"]["payload"]["fade_time"] = fade_time;
    JsonArray channels= parseChannelString(channel);
    jsonDoc["data"]["payload"].createNestedArray("channels");
    jsonDoc["data"]["payload"]["channels"] = channels;

  } else {
    Serial.println("Invalid command");
    return;
  }

  serializeJson(jsonDoc, mqttCommand);
  sendMQTTCommand(mqttCommand.c_str());
  //ledUpdate=true;
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  String message;
  String deviceMacId;
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }

  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("]: ");
  Serial.println(message);
 
  String topicStr = String(topic);
 

  if (topicStr.startsWith("switch/") && topicStr.endsWith("/api/out")) {
    StaticJsonDocument<512> jsonDoc;
    deserializeJson(jsonDoc, message);
     // Extract MAC ID from topic
    
    int startIndex = topicStr.indexOf("switch/") + 7; // Length of "switch/" is 7
    int endIndex = topicStr.indexOf("/api/out");
    
    // if (startIndex >= 7 && endIndex > startIndex) {
    //   deviceMacId = topicStr.substring(startIndex, endIndex);
    //   //Serial.print("Extracted MAC ID: ");
    //   //Serial.println(deviceMacId);
    // }
    deviceMacId = jsonDoc["meta"]["sender"].as<String>();
    Serial.print("SenderMAC: ");Serial.println(deviceMacId);

    if (jsonDoc["data"]["errors"][0] == "Invalid Token or no access to Mac Address") {
      Serial.println("Token expired, refreshing...");
      getJWTToken();
    }
    else if(jsonDoc["data"]["errors"][0] == "Succesfully added set_level request to queue")
    {
      //Nothing to do now
    }
    else if(jsonDoc["data"]["errors"][0] == "Failed to add set_level request to queue")
    {
      //May be Fresh Bootup, make all LED's to minimum 50% or 100% Bridghtness
      Serial.println("**** Failed to add set_level request to queue ****");
    } /*
    else
    {
      JsonObject channelData = jsonDoc["data"]["data"];
      if (!channelData.isNull()) {
       for (int i = 0; i < MAX_TOUCH_BTNS; i++) {
          //check if topic macid matches with tuchpad macID
          if (!(String(macIds[configs[i].macIDNum]).startsWith(deviceMacId))) //if (macIds[configs[i].macIDNum]!=deviceMacId)
          {
            Serial.println("MAC ID did not match 1");
            continue;
          }
          //finding first assigned channel index for that touch pad
          int chIndex=-1;
          for (int ch = 0; ch < 16; ch++)
          {
            if(configs[i].channels[ch]) //if any channel's assigned to this port get the channel index
            {
               chIndex = configs[i].channels[ch];
               //Serial.print(chIndex);Serial.print(",");
               break;
            }
          }
          Serial.println();
          // Channel index
          if(chIndex!=-1)
          {
            Serial.print("Touch"); Serial.println(i+1);
            String channelKey = "channel_";
            if (chIndex < 10) {
              channelKey += "0"; // Add leading zero for single-digit channel numbers
            }
            channelKey += String(chIndex);
            Serial.print("Key:"); Serial.println(channelKey);
            if(channelData.containsKey(channelKey))
            {
              configs[i].target_level = channelData[channelKey]["level"]; // Update target_level
              if(configs[i].target_level)
              {
                configs[i].prevState=true;
              }
              else
              {
                configs[i].prevState=false;
              }
            }
          }
          // Log updated configuration
          //Serial.println("Config updated with channel levels:");
        }
        ledUpdate=true;
      }
    } */
  }

//  if(dimkeyInTouch) //if Dimm key in touch, lets not handle to save the got mqtt data, as it is not the proper data.
//  {
//    return;
//  }

  if (String(topic).startsWith("state/")) {
    Serial.println("Channel state updated: " + message);
    StaticJsonDocument<512> jsonDoc;
    deserializeJson(jsonDoc, message);
    
    
    int startIndex = topicStr.indexOf("state/") + 6; // Length of "switch/" is 7
    int endIndex = topicStr.indexOf("/channels");
    
    // if (startIndex >= 7 && endIndex > startIndex) {
    //   deviceMacId = topicStr.substring(startIndex, endIndex);
    //   Serial.print("Extracted MAC ID: ");
    //   Serial.println(deviceMacId);
    // }

    deviceMacId = jsonDoc["meta"]["sender"].as<String>();
    Serial.print("SenderMAC: ");Serial.println(deviceMacId);
    // Extract the "data" array
    JsonArray data = jsonDoc["data"].as<JsonArray>();
    Serial.println(data);
    if (!data.isNull()) {
      for (int i = 0; i < MAX_TOUCH_BTNS; i++) {
        //Serial.println(macIds[configs[i].macIDNum]);
        if (!(String(macIds[configs[i].macIDNum]).startsWith(deviceMacId)))
        {
          //Serial.println("MAC ID did not match 2");
          continue;
        }
        //finding first assigned channel index for that touch pad
        int chIndex=-1;
        for (int ch = 0; ch < 16; ch++)
        {
          if(configs[i].channels[ch]) //if any channel's assigned to this port get the channel index
          {
             chIndex = configs[i].channels[ch];
             //Serial.print(chIndex);Serial.print(",");
             break;
          }
        }
        Serial.println();
        // Channel index
        if(chIndex!=-1)
        {
          Serial.print("Touch Pad:"); Serial.println(i+1);
          //configs[i].target_level = data[chIndex-1];     // Set target_level from data array
          int rcvd_level = data[chIndex-1];
          Serial.print("Ch: ");Serial.print(chIndex-1); Serial.print(" Level: ");Serial.println(rcvd_level);
          if(rcvd_level)
          {
            configs[i].prevState=true;
            configs[i].target_level = rcvd_level;
          }
          else
          {
            configs[i].prevState=false;
          }
          ledUpdate=true;
        }
      }
    }
  }
}

void setup() {
  Serial.begin(115200);
  Wire.begin(I2C_SDA,I2C_SCL);
  delay(100);
  
  setup_touch();
  setup_led();
  
  setup_wifi_mqtt();
  setup_IrReceiver();
}

void loop() {
  
  loop_touch();
  loop_wifi_mqtt();
  loop_IrReceiver();
  
//  if (!client.connected()) {
//    connectMQTT();
//  }
  //client.loop();
  loop_led();
  if (Serial.available()) {
    String command = Serial.readStringUntil('\n');
    command.trim();
    processSerialCommand(command);
  }
  
}
void setup_IrReceiver() {
  irrecv.enableIRIn();  // Start the receiver
}


bool lastIrHandled = false;

void loop_IrReceiver() {
  if (irrecv.decode(&results)) {
    uint32_t result = results.value;
    irrecv.resume();  // Ready for next code

    // Debounce repeated key press
    static uint32_t lastIR = 0;
    if (result == lastIR && lastIrHandled) return;
    lastIrHandled = true;
    lastIR = result;

    Serial.printf("IR Received: 0x%X\n", result);

    switch (result) {
      case IR_1:
      case IR_2:
      case IR_3:
      case IR_4: {
        int touchIndex = -1;
        if (result == IR_1) touchIndex = 0;  // U11
        else if (result == IR_2) touchIndex = 1;  // U8
        else if (result == IR_3) touchIndex = 2;  // U5
        else if (result == IR_4) touchIndex = 3;  // U7

        if (touchIndex >= 0 && touchIndex < MAX_TOUCH_BTNS) {
          handleTouchdata(touchIndex);  // Toggle ON/OFF
          ledUpdate = true;
        }
        break;
      }

      default:
        break;  // Ignore other IR signals
    }
  } else {
    lastIrHandled = false;  // Reset debounce
  }
}


