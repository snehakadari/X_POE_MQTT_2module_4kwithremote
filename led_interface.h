#define FASTLED_RMT_MAX_CHANNELS 2  //Restict to use only 2 RMT channels to drive LED's.
#define FASTLED_RMT_BUILTIN_DRIVER 1 //this is manadatory as RMT is also used by IR remote

#include <FastLED.h>

boolean ledUpdate=false;

#define LED_DATA_PIN 25
#define NUM_LEDS 7

boolean sleepAllLeds=false;
boolean sleepStatus =false;
//volatile uint8_t globalStatusLED=0;

#define WHITE_COLOR 0xFFFFFF
#define GRAY_COLOR 0x404040
#define RED_COLOR 0x550000
#define GREEN_COLOR 0x005500
#define BLUE_COLOR 0x000055
#define LED_TRUNOFF 0x000000

#define YELLOW_COLOR 0xFFCC00
#define PURPLE_COLOR 0xCC66FF

#define  MQTT_CONNETCED_LED BLUE_COLOR
#define  MQTT_DISCONNECTED  GREEN_COLOR //MQTT disconect or Wifi Connected
#define  WIFI_DISCONNECTED RED_COLOR
#define  WIFI_AP_MODE_LED PURPLE_COLOR

#define LED_ON_COLOR  BLUE_COLOR  //cP.led_on_color
#define LED_OFF_COLOR  GRAY_COLOR //RED_COLOR//cP.led_off_color //GRAY_COLOR
#define LED_STA_CONNECTED_COLOR GREEN_COLOR
#define MASTER_OFF_COLOR RED_COLOR

#define LED_OTA_OFF_COLOR YELLOW_COLOR
#define LED_OTA_ON_COLOR PURPLE_COLOR

enum ledName {
  led_R1 = 0,   // U5
  led_UNUSED1 = 1,  // Placeholder
  led_R3 = 2,   // U7
  led_R6 = 3,   // U8  
  led_UNUSED2 = 4,  // Placeholder
  led_R7 = 5,   // U11 → Move this from 3 → 5
  led_STA = 6   // U12 WiFi LED
};


// Define the array of leds
CRGB leds[NUM_LEDS];
void updateLeds()
{
    leds[led_R7] = (configs[0].prevState ? LED_ON_COLOR : LED_OFF_COLOR);  // U11 → Channel 1
    leds[led_R6] = (configs[1].prevState ? LED_ON_COLOR : LED_OFF_COLOR);  // U8  → Channel 2
    leds[led_R1] = (configs[2].prevState ? LED_ON_COLOR : LED_OFF_COLOR);  // U5  → Channel 3
    leds[led_R3] = (configs[3].prevState ? LED_ON_COLOR : LED_OFF_COLOR);  // U7  → Channel 4
    FastLED.show();
}

void toggleOtaLed()
{
    if(leds[led_STA] == CRGB(LED_OTA_OFF_COLOR))
    { 
      leds[led_STA]= LED_OTA_ON_COLOR;
    }
    else
    {
      leds[led_STA]= (LED_OTA_OFF_COLOR);
    }
    FastLED.show();
    
    //delay(1);
}

void otaLedTimerHandle()
{
    toggleOtaLed();
}

void toggleStatusLed()
{
    if(leds[led_STA] == CRGB(LED_OFF_COLOR))
    {
      leds[led_STA]=LED_STA_CONNECTED_COLOR;
    }
    else
    {
      leds[led_STA]= (LED_OFF_COLOR);
    }
}

boolean setStaled(int led_status)
{
    boolean LedsetStatus=0;

     if(led_status == WIFI_SOFTAP)
     {
        Serial.println("Setting LED to WIFi AP MODE..");
        if(leds[led_STA]!= CRGB(WIFI_AP_MODE_LED))
        {
          leds[led_STA]= (WIFI_AP_MODE_LED);
          LedsetStatus=1;
        }
     }
     else if(led_status == WIFI_DISCONNECT)
     {
        Serial.println("Setting LED to WIFi Disconnected..");
        if(leds[led_STA]!= CRGB(LED_OFF_COLOR))
        {
          leds[led_STA]= (LED_OFF_COLOR);
          LedsetStatus=1;
        }
     }
     else if((led_status == WIFI_CONNECTED)|| (led_status==MQTT_DISCONNECT))
     {
        if(leds[led_STA]!= CRGB(MQTT_DISCONNECTED))
        {
          leds[led_STA]= (MQTT_DISCONNECTED);
          LedsetStatus=1;
        }
     }
     else // MQTT connected
     {
        if(leds[led_STA]!= CRGB(MQTT_CONNETCED_LED))
        {
          leds[led_STA]= (MQTT_CONNETCED_LED);
          LedsetStatus=1;
        }
     }

     if (sleepStatus) //if ALL LEDS are sleeping then Dont run status LED Enable sequence.
       LedsetStatus =0;
      return LedsetStatus;
}

void flushLed()
{
  FastLED.show();
  delay(1);
}


void setup_led() { 
      FastLED.addLeds<WS2812B, LED_DATA_PIN, GRB>(leds, NUM_LEDS);
}

void loop_led() { 
  if(ledUpdate)
  {
    Serial.println("Updating LED's");
    updateLeds();
    ledUpdate=false;
  }
}  
