#include "LightController.h"
#include "helpers.h"
#include "effects.h"
#include "LedDriver.h"
#include "average.h"
#include "CapacitiveSensorButton.h"
#include "MqttProcessor.h"
#include "ota.h"
#include "WebPortal.h"
#include "EmergencyProtocol.h"
#include "VoiceControl.h"

#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <ArduinoOTA.h>
#include <ResetDetector.h>
#include <ESPAsyncWiFiManager.h>
#include <Hash.h>
#include <RemoteDebug.h>
#include <ESPAsyncWebServer.h>

LightController *lightController;
AbstractCapacitiveSensorButton* sensorButton;
RemoteDebug Debug;

void setupWifi() {
  LedDriver blueLed(LEAD_INDICATOR_PIN, HIGH);
  blueLed.blink(500);
  AsyncWebServer webServer(80);
  DNSServer dns;
  AsyncWiFiManager wifiManager(&webServer, &dns);
  wifiManager.setMinimumSignalQuality(60);
  wifiManager.setLoopExtraRoutine([] () {
    sensorButton->loop();
    lightController->loop();
  });
  wifiManager.autoConnect(HOSTNAME);
  blueLed.setHigh();
}

void setup() { 
#if LOGGING
  Serial.begin(74880, SERIAL_8N1, SERIAL_TX_ONLY);
  Serial.setDebugOutput(true);
#endif 
  EmergencyProtocol::checkOnActivation();
  lightController = new LightController(LED_PINS, defaultEffects());
  sensorButton = AbstractCapacitiveSensorButton::create(lightController);
  VoiceControl::setup();
  setupWifi();
  Ota::setup();
  randomSeed(ESP.getCycleCount());
  MqttProcessor::setup();
  Debug.begin(HOSTNAME);
  WebPortal::setup();
}

unsigned long lastStatusCheck = 0;

void loop() {
    // Stage 0: all what is not related to Light
  Ota::loop();
  Debug.handle();
  // Stage 1: read all possible sources that could change Ligst state
  sensorButton->loop();
  // Stage 2: notify all sources about Light changes
  if (lightController->isChanged()) {
    WebPortal::broadcaseLightChanges();
    MqttProcessor::broadcastStateViaMqtt();
  }
  // Stage 3: play light animation and clear change flags
  lightController->loop();

  if (millis() - lastStatusCheck > 3000) {
    // DBG("Free heap: %d\n", ESP.getFreeHeap());
    // DBG("Heap fragmentation: %d%%\n", ESP.getHeapFragmentation());
    lastStatusCheck = millis();
  }
}