#include "bt_cap1114.h"

#define TOUCH_2_MODULE

#ifdef TOUCH_2_MODULE
#define CONFIG_KEY_DATA 8192 //0x800
#endif

#define POWERKEY_PRESS_TIME 5 //sec
bool u11_long_press = false;  // Tracks if U11 long press happened

void dimmerHandle();

TickTwo dimmerTicker(dimmerHandle, dimTickerMillis, 0, MILLIS);

boolean dimkeyInTouch=false;

//Enums and structures
enum error_status
{
  BT_SUCCESS,
  BT_FAIL,
  BT_HW_ERROR
};

struct touchStatus
{
  uint16_t touch1:1;
  uint16_t touch2:1;
  uint16_t touch3:1;
  uint16_t touch4:1;
  uint16_t touch5:1;
  uint16_t touch6:1;
  uint16_t touchDn:1;
  uint16_t touchUp:1;
  uint16_t touch7:1;
  uint16_t touch8:1;
  uint16_t touch9:1;
  uint16_t touch10:1;
  uint16_t touch11:1;
  uint16_t touch12:1;
  uint16_t touch13:1;
  uint16_t touch14:1;
};


//global variables
struct touchStatus *tS;
volatile boolean capInterruptCame=false;
unsigned long powerkeyTouchtime=0;

//API's
void setup_touch();
void loop_touch();

//Local Functions
void handleTouchdata(int touchData);
uint16_t readTouchData(uint8_t i2c_addr);
void clearTouchInterrupt(uint8_t i2c_addr);
void processTouchInterrupt(uint8_t i2c_addr);
void IRAM_ATTR handleTouchInterrupt();
String makeChannelString(int touchIndex);

//Function implimentation
uint16_t readTouchData(uint8_t i2c_addr)
{
    return readcap1114TouchData(i2c_addr);
}

void clearTouchInterrupt(uint8_t i2c_addr)
{
  clearCap1114Interrupt(i2c_addr);
}


void dimmerHandle() {
  touch1_dimcounter--;
  if (touch1_dimcounter < 1) {
    touch1_dimcounter = dimSteps;
  }

  int dimming_level = touch1_dimcounter * (100 / dimSteps);
  Serial.println("DIM TICKER..");

  // Long press behavior for U11 (index 2)
  if (currTouchIndex == 0 && dimmerTicker.counter() >= 20) {
    Serial.println("U11 LONG PRESS → Goto AP Mode");
    gotoApmode = true;
    staLedUpdated = setStaled(WIFI_SOFTAP);  // Sets U12 (led_STA) to PURPLE
    dimmerTicker.stop();
    return;
  }

  // Regular dimming command
  String Scommand = "LEVEL " + makeChannelString(currTouchIndex) + " " + String(dimming_level);
  Serial.println(Scommand);
  processSerialCommand(Scommand);

  configs[currTouchIndex].prevState = (dimming_level == 0) ? LOW : HIGH;

  if (!dimkeyInTouch) {
    dimmerTicker.stop();
  }
}


unsigned long firstTouchTime = 0;

void processTouchInterrupt(uint8_t i2c_addr)
{
  Serial.println("-----Process Touch Interrupt--------");
  uint16_t prevStatus = readTouchData(i2c_addr);
  Serial.print("Touch data: "); Serial.print(prevStatus);
  Serial.print("Prev Touch Data:"); Serial.println(prevStatus);

  clearTouchInterrupt(i2c_addr);
  if (prevStatus == 0) {
    Serial.println("No touch detected, Invalid interrupt");
    return;
  }

  delay(1);
  uint16_t currStatus = readTouchData(i2c_addr);
  Serial.print("Touch data: "); Serial.print(currStatus);
  if (currStatus == 0) Serial.println("No data ");
  Serial.print("Current Touch Data:"); Serial.println(currStatus);

  currTouchIndex = -1;

  // ✅ Check for valid light touch
  for (int i = 0; i < MAX_TOUCH_BTNS; i++) {
    if (touch_key_data_mapping[i] == prevStatus) {
      currTouchIndex = i;
      break;
    }
  }

  Serial.print("Touch Index:"); Serial.println(currTouchIndex);

  // ✅ Handle pairing mode via U12 (CONFIG_KEY_DATA = 8192)
  if (prevStatus == CONFIG_KEY_DATA) {
    Serial.println("🟡 Pairing Mode triggered via U12");
    gotoApmode = true;
    return;
  }

  // ✅ Ignore anything beyond U5, U7, U8, U11
  if (currTouchIndex < 0 || currTouchIndex >= MAX_TOUCH_BTNS) {
    Serial.println("❌ Ignored: Touch from unused pad");
    return;
  }

  if (prevStatus == currStatus) {
    // Touch detected
    Serial.println("******Touch interrupt*******");
    dimmerTicker.start();
    dimkeyInTouch = true;
    firstTouchTime = millis();
  } else {
    // Release detected
    Serial.println("******release interrupt******");
    dimkeyInTouch = false;
    dimmerTicker.stop();

    if (millis() - firstTouchTime < 1000) {
      handleTouchdata(currTouchIndex);
    }

    ledUpdate = true;
  }
}


String makeChannelString(int touchIndex)
{
  bool addComa=false;
  String mqtt_macidTouch = macIds[configs[touchIndex].macIDNum];
  mqtt_topic_pub = "switch/" + String(mqtt_macidTouch) + "/api";
  //mqtt_topic_sub = "switch/" + String(mqtt_macid) + "/api/out";

  if(configs[touchIndex].macIDNum == 0)
  {
     master_pub =true;
  }
  else
  {
     master_pub =false;
  }

  if(master_pub==false) //if it is slave x-poe publish, update the right JWT token of it
  {
    SlaveJwt = jwtTokens[configs[touchIndex].macIDNum];
  }
  String ChannelStr="[";
    for (int ch = 0; ch < 16; ch++) {
            if (configs[touchIndex].channels[ch] > 0) {
                if(addComa)
                {
                  ChannelStr+=",";
                }
                else //first packet dont add coma before
                {
                  addComa=true;
                }
                ChannelStr += String(configs[touchIndex].channels[ch]);
            }
        }
    ChannelStr+="]";
   return ChannelStr;
}

//boolean lightStatus=0;
void handleTouchdata(int touchIndex)
{
  String Scommand="";
  Serial.println("Touch handler");
  Serial.print("PREVSTATE: ");Serial.println(configs[touchIndex].prevState);
  
  if(configs[touchIndex].prevState==HIGH){
    
    Scommand="OFF "+ makeChannelString(touchIndex); //String(configs[touchIndex].channels[0]);
    Serial.println(Scommand);
    processSerialCommand(Scommand);
    configs[touchIndex].prevState=LOW;
  }
  else
  {
    if(touch1_dimcounter<5)
    {
      //Scommand="LEVEL "+String(configs[touchIndex].channels[0]) + String(" 50");
      Scommand="LEVEL "+ makeChannelString(touchIndex) + String(" 50");
      Serial.println(Scommand);
      processSerialCommand(Scommand);
      
      //As there is a problem here like if Level <30, the light is not turning on! we need to make it work later
//      delay(300);
//      int dimming_level = touch1_dimcounter*10;
//      String Scommand="LEVEL 1 "+ String(dimming_level);
//      Serial.println(Scommand);
//      processSerialCommand(Scommand);
    }
    else{
      Scommand="LEVEL "+ makeChannelString(touchIndex) + String(" ") + String(configs[touchIndex].target_level);
      //Scommand="ON "+ makeChannelString(touchIndex);//String(configs[touchIndex].channels[0]);
      Serial.println(Scommand);
      Serial.println("*** --- ***");
      processSerialCommand(Scommand);
    }
    configs[touchIndex].prevState=HIGH;
  }
}


void IRAM_ATTR handleTouchInterrupt()
{
  //Serial.println("Touch Interrupt received");
  capInterruptCame=true;
}


void setup_touch()
{
  //Configure GPIO interrupt
  pinMode(CAP_INTR_PIN, INPUT);
  attachInterrupt(CAP_INTR_PIN, handleTouchInterrupt, RISING);
  //Enable Touch IC
  pinMode(ESP_CAP_RESET, OUTPUT);
  digitalWrite(ESP_CAP_RESET, LOW);
  delay(15); // 10mS delay needed before doing any I2C transactions on CAP1114
  //Initialise CAP
  boolean capStatus = bt_cap1114_init(CAP_I2CADDR);
  if(!capStatus)
  {  //try again
     digitalWrite(ESP_CAP_RESET, HIGH);
     delay(10);
     digitalWrite(ESP_CAP_RESET, LOW);
     delay(20);
     capStatus = bt_cap1114_init(CAP_I2CADDR);
     if(!capStatus)
     {
        Serial.println("Touch initialisation failed");
        //update LED in such way that touch is failed to detect.
     }
  }
}

void loop_touch()
{
  if(capInterruptCame)
  {
     processTouchInterrupt(CAP_I2CADDR);
     capInterruptCame = false;
  }  
  dimmerTicker.update();
}
