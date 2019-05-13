#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ESP8266mDNS.h>
#include <ArduinoOTA.h>
#include <ResetDetector.h>
#include <WiFiManager.h>

#include "config.h"
#include "LightState.h"
#include "helpers.h"
#include "effects.h"
#include "LedDriver.h"
#include "average.h"
#include "CapacitiveSensorButton.h"
#include "MqttProcessor.h"

bool OTAStarted = false;

LightState lightState(LED_PINS, defaultEffects());
LedDriver ledBuildIn(-1);
LedDriver ledOne(-1);
LedDriver ledTwo(-1);
AbstractCapacitiveSensorButton* sensorButton = AbstractCapacitiveSensorButton::create(&lightState);

void onOTAStart() {
  DBG("Starting OTA session\n");
  OTAStarted = true;
}

void setupOta() {
  ArduinoOTA.setHostname(HOSTNAME);
  ArduinoOTA.begin();
  ArduinoOTA.onStart(onOTAStart);
}

void setupSmartWifi(bool resetPassword) {
  WiFiManagerParameter param();
  WiFiManager wifiManager;
  wifiManager.setMinimumSignalQuality(75);
  if (resetPassword) {
    wifiManager.resetSettings();
  }
  wifiManager.autoConnect(HOSTNAME);
}

void setup() { 
  int resetCount = ResetDetector::execute(2000);

#if LOGGING
  Serial.begin(74880, SERIAL_8N1, SERIAL_TX_ONLY);
  Serial.setDebugOutput(true);
#endif 
  DBG("Starting with number of resets = %d\n", resetCount);
  lightState.setup();
  setupSmartWifi(resetCount >= 3);
  setupOta();
  
  randomSeed(ESP.getCycleCount());
  MqttProcessor::setup();
  ledBuildIn.setPattern({500, 1000}, LOW);
  ledOne.blink(300);
  ledTwo.blink(600);
}

void loop() {
  ArduinoOTA.handle();
  if (OTAStarted) {
    return;
  }
  lightState.handle();
  ledBuildIn.loop();
  ledOne.loop();
  ledTwo.loop();
  sensorButton->loop();
}