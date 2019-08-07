#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <ArduinoOTA.h>
#include <ResetDetector.h>
#include <WiFiManager.h>
#include <Hash.h>
#include <RemoteDebug.h>

#include "LightState.h"
#include "helpers.h"
#include "effects.h"
#include "LedDriver.h"
#include "average.h"
#include "CapacitiveSensorButton.h"
#include "MqttProcessor.h"
#include "ota.h"
#include "WebPortal.h"

LightState lightState(LED_PINS, defaultEffects());
AbstractCapacitiveSensorButton* sensorButton = AbstractCapacitiveSensorButton::create(&lightState);
LedDriver blueLed(2);
RemoteDebug Debug;

void setupSmartWifi(bool resetPassword) {
  blueLed.blink(500);
  WiFiManagerParameter param();
  WiFiManager wifiManager;
  wifiManager.setMinimumSignalQuality(50);
  if (resetPassword) {
    wifiManager.resetSettings();
  }
  wifiManager.autoConnect(HOSTNAME);
  blueLed.setHigh();
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
  Ota::setup();
  randomSeed(ESP.getCycleCount());
  MqttProcessor::setup();
  Debug.begin(HOSTNAME);
  WebPortal::setup();
}

void loop() {
  // Stage 0: all what is not related to Light
  Ota::loop();
  Debug.handle();
  // Stage 1: read all possible sources that could change Ligst state
  sensorButton->loop();
  // Stage 2: notify all sources about Light changes
  if (lightState.isChanged()) {
    WebPortal::broadcaseLightChanges();
    MqttProcessor::broadcastStateViaMqtt();
  }
  // Stage 3: play light animation and clear change flags
  lightState.handle();
}