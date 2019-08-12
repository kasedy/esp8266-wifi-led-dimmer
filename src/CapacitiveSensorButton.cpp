#include "CapacitiveSensorButton.h"
#include "hardware.h"
#include "LightController.h"
#include "average.h"

#include <CapacitiveSensor.h>
#include <ESP8266WiFi.h>
#include <RemoteDebug.h>

#define NUM_SAMPLES 50
#define PROCESSING_INTERVAL 25
#define THRESHOLD 250
#define CLICK_TIME_THRESHOLD 500
#define DOUBLE_CLICK_TIME_THRESHOLD 250

extern RemoteDebug Debug;

class DummyCapacitiveSensorButton : public AbstractCapacitiveSensorButton {
public:
  virtual void loop() {}
};

class CapacitiveSensorButton : public AbstractCapacitiveSensorButton {
  LightController* lightController;
  CapacitiveSensor cs;
  AverageValueCalculator<uint32_t, uint32_t> touchSensorData;
  bool isPressed;
  bool startHandlingLongPress;
  uint32_t lastAverageCalculation;
  uint32_t lastDownTime;
  uint32_t lastUpTime;
  uint8_t rapidClickCounter;
  uint8_t lastChangeBrightness;
  bool lightScrollDirectionUp;
public:
  // sendPin is a pin with high resistor in front
  CapacitiveSensorButton(uint8_t sendPin, uint8_t receivePin, LightController* lightController);
  virtual void loop();

private:
  void onClickHandler();
  void onDoubleClickHandler();
  void onLongPressHandler(bool isFirst);
  void onMultipleClicksHandler();
};


CapacitiveSensorButton::CapacitiveSensorButton(uint8_t sendPin, uint8_t receivePin, LightController* lightController) : 
    lightController(lightController),
    cs(CapacitiveSensor(sendPin, receivePin)),
    isPressed(false),
    startHandlingLongPress(false),
    lastAverageCalculation(0),
    lastDownTime(0),
    lastUpTime(0),
    rapidClickCounter(1),
    lastChangeBrightness(0),
    lightScrollDirectionUp(false) {
  cs.set_CS_AutocaL_Millis(0xFFFFFFFF);
  cs.set_CS_Timeout_Millis(20);
}

void CapacitiveSensorButton::loop() {
  long sensorTime = cs.capacitiveSensor(NUM_SAMPLES);
  if (sensorTime < 0) {
    // DBG("Failed to read capacitive sensor. Error code = %ld\n", sensorTime);
    return;
  }
  touchSensorData.addMeasurement(sensorTime);

  uint32_t now = millis();
  if (now - lastAverageCalculation >= PROCESSING_INTERVAL) {
    if (touchSensorData.getCounter() == 0) {
      // DBG("Failed to get average for capacitive sensor charge time\n");
      return;
    }

    uint32_t averageSensorTime = touchSensorData.getAverage();
    Debug.println(averageSensorTime);
    if (isPressed && averageSensorTime < THRESHOLD) {
      isPressed = false;
      startHandlingLongPress = false;
      if (now - lastDownTime > CLICK_TIME_THRESHOLD) {
        rapidClickCounter = 0;
      } else {
        ++rapidClickCounter;
      }
      lastUpTime = now;
      DBG("Touch up %d\n", rapidClickCounter);
    } else if (!isPressed && averageSensorTime > THRESHOLD) {
      isPressed = true;
      startHandlingLongPress = true;
      lastDownTime = now;
      DBG("Touch down\n");
    }

    touchSensorData.reset();
    lastAverageCalculation = now;
  }

  if (!isPressed && now - lastUpTime > DOUBLE_CLICK_TIME_THRESHOLD) {
    if (rapidClickCounter == 1) {
      onClickHandler();
    } else if (rapidClickCounter == 2) {
      onDoubleClickHandler();
    } else if (rapidClickCounter >= 20) {
      onMultipleClicksHandler();
    } 
    rapidClickCounter = 0;
  }

  if (isPressed && now - lastDownTime > CLICK_TIME_THRESHOLD) {
    DBG("Long press %d\n", rapidClickCounter);
    onLongPressHandler(startHandlingLongPress);
    // TODO: add double long press handler that will adjust animation speed
    startHandlingLongPress = false;
  }
}

void CapacitiveSensorButton::onClickHandler() {
  lightController->toggleState();
}

void CapacitiveSensorButton::onDoubleClickHandler() {
  lightController->setStateOn(true);
  lightController->nextAnimation();
}

void CapacitiveSensorButton::onLongPressHandler(bool isFirst) {
  uint8_t now = (uint8_t) millis();
  if (isFirst) {
    if (!lightController->isOn()) {
      lightController->setLightBrightness(0);
      lightController->setStateOn(true);
      lightScrollDirectionUp = true;
    } else {
      lightScrollDirectionUp = !lightScrollDirectionUp;
    }
  } else if ((uint8_t) (now - lastChangeBrightness) < 15) {
    return;
  }
  lastChangeBrightness = now;
  uint8_t brightness = lightController->getLightBrightness();
  uint8_t step = 2; 
  if (brightness < 30) {
    step = 1;
  } else if (brightness > 140 && brightness < 190) {
    step = 3;
  } else if (brightness >= 190) {
    step = 4;
  }
  if (brightness > 255 - step) {
    lightScrollDirectionUp = false;
  } else if (brightness < step) {
    lightScrollDirectionUp = true;
  }
  lightController->setLightBrightness(brightness + (lightScrollDirectionUp ? step : -step));
}

void CapacitiveSensorButton::onMultipleClicksHandler() {
  WiFi.disconnect(true);
  delay(200);
  ESP.reset();
}

AbstractCapacitiveSensorButton* AbstractCapacitiveSensorButton::create(LightController* lightController) {
#if defined(CAPACITIVE_SENSOR_SEND_PIN) && defined(CAPACITIVE_SENSOR_RECEIVE_PIN)
  return new CapacitiveSensorButton(CAPACITIVE_SENSOR_SEND_PIN, CAPACITIVE_SENSOR_RECEIVE_PIN, lightController);
#else
  return new DummyCapacitiveSensorButton();
#endif
}