#include "ota.h"
#include "hardware.h"
#include "dbg.h"

#include <ArduinoOTA.h>

namespace Ota {
  bool OTAStarted = false;

  void onOTAStart() {
    DBG("Starting OTA session\n");
    OTAStarted = true;
  }

  void setup() {
    ArduinoOTA.setHostname(HOSTNAME);
    ArduinoOTA.begin();
    ArduinoOTA.onStart(onOTAStart);
  }

  void loop() {
    while (true) {
      ArduinoOTA.handle();
      if (!OTAStarted) {
        return;
      }
      yield();
    }
  }
};