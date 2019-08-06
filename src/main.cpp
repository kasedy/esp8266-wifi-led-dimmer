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
}

void loop() {
  Ota::loop();
  lightState.handle();
  sensorButton->loop();
  Debug.handle();
}