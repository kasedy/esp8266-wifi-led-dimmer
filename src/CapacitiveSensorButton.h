#pragma once
#include <CapacitiveSensor.h>
#include <functional>

#include "average.h"

class CapacitiveSensorButton {
  CapacitiveSensor cs;
  AverageValueCalculator<uint32_t, uint32_t> touchSensorData;
  bool isPressed;
  uint32_t lastAverageCalculation;
  uint32_t lastDownTime;
  uint32_t lastUpTime;
  uint8_t rapidClickCounter;
  uint16_t longPressCounter;
  std::function<void()> onClickHandler;
  std::function<void()> onDoubleClickHandler;
  std::function<void(uint16_t)> onLongPressHandler;
public:
  // sendPin is a pin with high resistor in front
  CapacitiveSensorButton(uint8_t sendPin, uint8_t receivePin);

  void setOnClickHandler(std::function<void()> handler) {
    onClickHandler = handler;
  }

  void setOnDoubleClickHandler(std::function<void()> handler) {
    onDoubleClickHandler = handler;
  }

  void setOnLongPressHandler(std::function<void(uint16_t)> handler) {
    onLongPressHandler = handler;
  }
  void loop();
};