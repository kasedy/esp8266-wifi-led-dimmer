#include "EmergencyProtocol.h"
#include "Ota.h"
#include "hardware.h"

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
    if (ResetDetector::execute(ALLOWED_TIME_INTERVAL_BETWEEN_RESETS) < EMERGENCY_THRESHOLD) {
      return;
    }
    digitalWrite(2, LOW);
    WiFi.mode(WIFI_STA);
    waitForConnectResult(5000);
    if (!WiFi.isConnected() && WiFi.SSID()) {
      ETS_UART_INTR_DISABLE();
      wifi_station_disconnect();
      ETS_UART_INTR_ENABLE();
      WiFi.begin();
      waitForConnectResult(30000);
    }
    if (!WiFi.isConnected()) {
      WiFi.mode(WIFI_AP);
      WiFi.softAP(DEVICE_NAME);
    }
    Ota::setup();
    while (true) {
      Ota::loop();
      yield();
    }
  }
}