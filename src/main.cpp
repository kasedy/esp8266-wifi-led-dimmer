#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ESP8266mDNS.h>
#include <ArduinoOTA.h>
#include <ResetDetector.h>
#include <WiFiManager.h>

#include "LightState.h"
#include "helpers.h"
#include "effects.h"
#include "LedDriver.h"
#include "average.h"
#include "CapacitiveSensorButton.h"
#include "MqttProcessor.h"
#include "ota.h"

LightState lightState(LED_PINS, defaultEffects());
AbstractCapacitiveSensorButton* sensorButton = AbstractCapacitiveSensorButton::create(&lightState);

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
  Ota::setup();
  randomSeed(ESP.getCycleCount());
  MqttProcessor::setup();
}

void loop() {
  if (!Ota::loop()) {
    return;
  }
  lightState.handle();
  sensorButton->loop();
}