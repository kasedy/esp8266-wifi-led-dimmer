#include "LightController.h"
#include "helpers.h"
#include "effects.h"
#include "LedDriver.h"
#include "MqttProcessor.h"
#include "ota.h"
#include "VoiceControl.h"

#include <WiFiManager.h>
#include <Hash.h>

LightController *lightController;

void setupWifi() {
  LedDriver blueLed(LEAD_INDICATOR_PIN, HIGH);
  blueLed.blink(500);
  WiFiManager wifiManager;
  wifiManager.autoConnect(HOSTNAME);
  blueLed.setHigh();
}

void setup() { 
#if LOGGING
  Serial.begin(74880, SERIAL_8N1, SERIAL_TX_ONLY);
  Serial.setDebugOutput(true);
#endif 
  lightController = new LightController(LED_PINS, defaultEffects());
  VoiceControl::setup();
  setupWifi();
  Ota::setup();
  randomSeed(ESP.getCycleCount());
  MqttProcessor::setup();
}

void loop() {
  // Stage 0: all what is not related to Light
  Ota::loop();
  // Stage 1: read all possible sources that could change Ligst state

  // Stage 2: notify all sources about Light changes
  if (lightController->isChanged()) {
    MqttProcessor::broadcastStateViaMqtt();
  }
  // Stage 3: play light animation and clear change flags
  lightController->loop();
}