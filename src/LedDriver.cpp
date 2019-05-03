#include "LedDriver.h"

#include <Arduino.h>

LedDriver::LedDriver(uint8_t pin) : pin(pin), initialState(LOW) {
  init();
}

void LedDriver::setPattern(std::vector<uint32_t> newPattern, bool newInitialState) {
  patternMs = newPattern;
  initialState = newInitialState;
  init();
}

void LedDriver::init() {
  if (pin == -1) {
    return;
  }
  pinMode(pin, OUTPUT);
  currentIndex = 0;
  digitalWrite(pin, initialState);
  lastUpdateTime = millis();
}

void LedDriver::loop() {
  if (pin == -1 || patternMs.empty()) {
    return;
  }

  uint32_t now = millis();
  if (now - lastUpdateTime < patternMs[currentIndex]) {
    return;
  }

  if (++currentIndex >= patternMs.size()) {
    currentIndex = 0;
  }

  uint8_t state;
  if (patternMs.size() == 1) {
    initialState = !initialState;
    state = initialState;
  } else if (currentIndex % 2 == 0) {
    state = initialState;
  } else {
    state = !initialState;
  }
  digitalWrite(pin, state);
  lastUpdateTime = now;
}

void LedDriver::setLow() {
  setPattern({}, LOW);
}

void LedDriver::setHigh() {
  setPattern({}, HIGH);
}

void LedDriver::blink(uint32_t interval) {
  setPattern({interval});
}