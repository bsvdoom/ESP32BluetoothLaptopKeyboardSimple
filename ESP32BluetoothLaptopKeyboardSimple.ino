#include <WiFi.h>
#include <AsyncTCP.h>

#include <ESPAsyncWebServer.h>
#include <AsyncElegantOTA.h>

#include <BleKeyboard.h>
#include <CD74HC4067.h>

#include "secret.h"

const char* ssid = SSID;
const char* password = PASSWORD;

AsyncWebServer server(80);

#ifdef BLE
  BleKeyboard bleKeyboard;
#endif

const int mux_pin = 7; // select a pin to share with the 16 channels of the CD74HC4067

CD74HC4067 mux(19, 18, 9, 10); 

#define KEYS_NUM 20

uint16_t delayMicrosInRead = 50;
uint16_t delayMicrosInWrite = 50;
uint32_t thresholdHold = 50000;
uint32_t thresholdRepeat = 2500;

uint8_t pin[KEYS_NUM] = {128,129,130,131,132,133, 134,135,136,137,138,139,140,141,142,143, 1,2,4,5};

const int keyMatrix[KEYS_NUM] = {'A','B','c','d','e','f','h','g','n','s','v','y','z','i','o','q','w','v','m','x'};

uint32_t keyDurationMatrix[KEYS_NUM] = {};

void readKeys() {
  int keyButton = 0;
  int keyButtonStatus = 0;
  int pinToRead = 0;
  uint8_t channel = 0;
  uint8_t pinStat = 0;

  for(int i = 0; i < KEYS_NUM; i++) {
    if(pin[i] >= 128) {
      channel = pin[i]-128;
      mux.channel(channel);
      pinToRead = mux_pin;
    }
    else
    {
      pinToRead = pin[i]; 
    }
    
    delay(1);

    pinStat = digitalRead(pinToRead);

    keyButtonStatus = keyDurationMatrix[i];

    keyButton = keyMatrix[i];

    // Serial.print("KEY #");
    // Serial.print(i);
    // Serial.print(" is ");
    // Serial.println(pinStat);

    if(keyButtonStatus != pinStat) {//state changed
      if (pinStat > 0) {  //if this button is pressed down, so was not before
      Serial.println("Pressing");
      #ifdef BLE
        bleKeyboard.press(keyButton);
      #endif
      } else {  //if this button is NOT pressed down, and was before
      Serial.println("Releaseing");
      #ifdef BLE
        bleKeyboard.release(keyButton);
      #endif
      }

      keyDurationMatrix[i] = pinStat;
      Serial.println(keyButton);

    }

  }

  
}



void startOTA() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  server.on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
    request->send(200, "text/plain", "Hi! This is a sample response.");
  });

  AsyncElegantOTA.begin(&server);  // Start AsyncElegantOTA
  server.begin();
  Serial.println("HTTP server started");
}

void setup() {
  delay(1000);
  Serial.begin(115200);
  
  delay(10);
  Serial.println("Starting!");

#ifdef OTA
    startOTA();
#endif

#ifdef BLE
  Serial.println("Starting BLE work!");
  bleKeyboard.begin();
#endif

  pinMode(mux_pin, INPUT_PULLDOWN);          //seting row as input

  pinMode(1, INPUT_PULLDOWN);  //one more row pin
  pinMode(2, INPUT_PULLDOWN);  //one more row pin
  pinMode(4, INPUT_PULLDOWN);  //one more row pin
  pinMode(5, INPUT_PULLDOWN);  //one more row pin

  
}

void loop() {
  
  
#ifdef BLE
 if (bleKeyboard.isConnected()) {
   readKeys();
 } else {
   Serial.println("Waiting 5 seconds for BT connection...");
   delay(5000);
 }
#else
  readKeys();
  delay(1000);
#endif 
}

