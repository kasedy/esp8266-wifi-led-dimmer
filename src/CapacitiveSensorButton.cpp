#include "CapacitiveSensorButton.h"
#include "hardware.h"
#include "LightState.h"
#include "MqttProcessor.h"
#include "average.h"

#include <CapacitiveSensor.h>

#define NUM_SAMPLES 100
#define PROCESSING_INTERVAL 25
#define THRESHOLD 900
#define CLICK_TIME_THRESHOLD 500
#define DOUBLE_CLICK_TIME_THRESHOLD 250

class DummyCapacitiveSensorButton : public AbstractCapacitiveSensorButton {
public:
  virtual void loop() {}
};

class CapacitiveSensorButton : public AbstractCapacitiveSensorButton {
  LightState* lightState;
  CapacitiveSensor cs;
  AverageValueCalculator<uint32_t, uint32_t> touchSensorData;
  bool isPressed;
  uint32_t lastAverageCalculation;
  uint32_t lastDownTime;
  uint32_t lastUpTime;
  uint8_t rapidClickCounter;
  bool lightScrollDirectionUp;
public:
  // sendPin is a pin with high resistor in front
  CapacitiveSensorButton(uint8_t sendPin, uint8_t receivePin, LightState* lightState);
  virtual void loop();

private:
  void onClickHandler();
  void onDoubleClickHandler();
  void onLongPressHandler(bool isFirst);
};


CapacitiveSensorButton::CapacitiveSensorButton(uint8_t sendPin, uint8_t receivePin, LightState* lightState) : 
    lightState(lightState),
    cs(CapacitiveSensor(sendPin, receivePin)),
    isPressed(false),
    lastAverageCalculation(0),
    lastDownTime(0),
    lastUpTime(0),
    rapidClickCounter(0),
    lightScrollDirectionUp(false) {
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
    onLongPressHandler(rapidClickCounter != 0);
    rapidClickCounter = 0; // ignore next click-up
  }
}

void CapacitiveSensorButton::onClickHandler() {
  lightState->toggleState();
  MqttProcessor::broadcastStateViaMqtt();
}

void CapacitiveSensorButton::onDoubleClickHandler() {
  if (lightState->nextAnimation()) {
    MqttProcessor::broadcastStateViaMqtt();
  }
}

void CapacitiveSensorButton::onLongPressHandler(bool isFirst) {
  if (isFirst) {
    lightScrollDirectionUp = !lightScrollDirectionUp;
  }
  uint8_t brightness = lightState->getMaxBrightness();
  if (brightness >= 254) {
    lightScrollDirectionUp = false;
  } else if (brightness <= 1) {
    lightScrollDirectionUp = true;
  }
  lightState->setMaxBrightness(brightness + (lightScrollDirectionUp ? 2 : -2));
  MqttProcessor::broadcastStateViaMqtt();
}

AbstractCapacitiveSensorButton* AbstractCapacitiveSensorButton::create(LightState* lightState) {
#if defined(CAPACITIVE_SENSOR_SEND_PIN) && defined(CAPACITIVE_SENSOR_RECEIVE_PIN)
  return new CapacitiveSensorButton(CAPACITIVE_SENSOR_SEND_PIN, CAPACITIVE_SENSOR_RECEIVE_PIN, lightState);
#else
  return new DummyCapacitiveSensorButton();
#endif
}