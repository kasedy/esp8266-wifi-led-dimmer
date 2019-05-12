#include <CapacitiveSensorButton.h>

#include "dbg.h"

#define NUM_SAMPLES 100
#define PROCESSING_INTERVAL 25
#define THRESHOLD 900
#define CLICK_TIME_THRESHOLD 500
#define DOUBLE_CLICK_TIME_THRESHOLD 250

CapacitiveSensorButton::CapacitiveSensorButton(uint8_t sendPin, uint8_t receivePin) 
    : cs(CapacitiveSensor(sendPin, receivePin)),
    isPressed(false),
    lastAverageCalculation(0),
    lastDownTime(0),
    lastUpTime(0),
    rapidClickCounter(0) {
  cs.set_CS_AutocaL_Millis(0xFFFFFFFF);
  cs.set_CS_Timeout_Millis(200);
}

void CapacitiveSensorButton::loop() {
  long sensorTime = cs.capacitiveSensor(NUM_SAMPLES);
  if (sensorTime < 0) {
    DBG("Failed to read capacitive sensor. Error code = %ld\n", sensorTime);
    return;
  }
  touchSensorData.addMeasurement(sensorTime);

  uint32_t now = millis();
  if (now - lastAverageCalculation >= PROCESSING_INTERVAL) {
    if (touchSensorData.getCounter() == 0) {
      DBG("Failed to get average for capacitive sensor charge time\n");
      return;
    }

    uint32_t averageSensorTime = touchSensorData.getAverage();
    if (isPressed && averageSensorTime < THRESHOLD) {
      isPressed = false;
      ++rapidClickCounter;
      lastUpTime = now;
      DBG("Touch up %d\n", rapidClickCounter);
    } else if (!isPressed && averageSensorTime > THRESHOLD) {
      isPressed = true;
      lastDownTime = now;
      longPressCounter = 0;
      DBG("Touch down\n");
    }

    touchSensorData.reset();
    lastAverageCalculation = now;
  }

  if (now - lastUpTime > DOUBLE_CLICK_TIME_THRESHOLD) {
    if (rapidClickCounter == 2) {
      onClickHandler();
    } else if (rapidClickCounter == 3) {
      onDoubleClickHandler();
    }
    rapidClickCounter = 1;
  }

  if (isPressed && now - lastDownTime > CLICK_TIME_THRESHOLD) {
    onLongPressHandler(longPressCounter++);
    rapidClickCounter = 0; // ignore next click-up
  }
}