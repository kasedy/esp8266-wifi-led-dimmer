#include "EmergencyProtocol.h"
#include "Ota.h"
#include "hardware.h"
#include "dbg.h"

#include <ResetDetector.h>
#include <ESP8266WiFi.h>

#define ALLOWED_TIME_INTERVAL_BETWEEN_RESETS 3000
#define EMERGENCY_THRESHOLD 3

namespace EmergencyProtocol {
  uint8_t waitForConnectResult(unsigned long connectTimeoutMs) {
    unsigned long start = millis();
    while (true) {
      uint8_t status = WiFi.status();
      bool timeout = connectTimeoutMs != 0 && millis() > start + connectTimeoutMs;
      if (timeout
          || status == WL_CONNECTED 
          || status == WL_CONNECT_FAILED) {
        return status;
      }
      delay(100);
    }
  }
  
  void checkOnActivation() {
    pinMode(LEAD_INDICATOR_PIN, OUTPUT);
    digitalWrite(LEAD_INDICATOR_PIN, HIGH);
    uint8_t numResets = ResetDetector::execute(ALLOWED_TIME_INTERVAL_BETWEEN_RESETS);
    DBG("Starting with %d reset(s)\n", numResets);
    if (numResets < EMERGENCY_THRESHOLD) {
      return;
    }
    DBG("Acivating EMERGENCY protocol!\n");
    digitalWrite(LEAD_INDICATOR_PIN, LOW);
    if (WiFi.SSID().length() > 0) {
      DBG("Trying to connect with saved credantials\n");
      if (!WiFi.isConnected()) {
        DBG("Change Wifi mode\n");
        WiFi.mode(WIFI_STA);
        waitForConnectResult(5000);
        DBG("Wifi status %d\n", WiFi.status());
      }
      if (!WiFi.isConnected()) {
        DBG("Force to connect\n");
        ETS_UART_INTR_DISABLE();
        wifi_station_disconnect();
        ETS_UART_INTR_ENABLE();
        WiFi.begin();
        waitForConnectResult(30000);
      }
      DBG("Wifi status %d\n", WiFi.status());
    }
    if (!WiFi.isConnected()) {
      DBG("Opening AP mode\n");
      WiFi.mode(WIFI_AP);
      WiFi.softAP(DEVICE_NAME);
    }
    DBG("Start OTA\n");
    Ota::setup();
    while (true) {
      Ota::loop();
      yield();
    }
  }
}