#include "EmergencyProtocol.h"
#include "Ota.h"
#include "dbg.h"

#include <ResetDetector.h>
#include <ESP8266WiFi.h>

namespace EmergencyProtocol {
  
  uint8_t waitForConnectResult(unsigned long connectTimeoutMs);
  
  void checkOnActivation() {
    uint8_t resetCount = ResetDetector::execute(2000);
    DBG("Starting with number of resets = %d\n", resetCount);
    if (resetCount < 10) {
      return;
    }
    DBG("Activating emergency protocol\n");
    digitalWrite(2, HIGH);
    DBG("Connection to WiFi\n");
    if (WiFi.begin() == WL_CONNECT_FAILED || waitForConnectResult(30000) != WL_CONNECTED) {
      DBG("Connection failed. Open AP.\n");
      WiFi.mode(WIFI_AP);
      WiFi.softAP(DEVICE_NAME);
    }
    DBG("Setup Ota\n");
    Ota::setup();
    while (true) {
      Ota::loop();
      yield();
    }
  }

  uint8_t waitForConnectResult(unsigned long connectTimeoutMs) {
    if (connectTimeoutMs == 0) {
      return WiFi.waitForConnectResult();
    }

    unsigned long start = millis();
    while (true) {
      uint8_t status = WiFi.status();
      if (millis() > start + connectTimeoutMs 
          || status == WL_CONNECTED 
          || status == WL_CONNECT_FAILED) {
        return status;
      }
      delay(100);
    }
  }
}