#include "LedDriver.h"

#include <Arduino.h>

LedDriver::LedDriver(uint8_t pin, bool state) 
      : pin(pin), initialState(state), cycle(true) {
  if (pin != -1) {
    pinMode(pin, OUTPUT);
    updateLed();
  }
}

void LedDriver::setPattern(std::vector<uint32_t> newPattern, bool newInitialState, bool newCycle) {
  if (patternMs == newPattern || pin == -1) {
    return;
  }
  patternMs = newPattern;
  initialState = newInitialState;
  cycle = newCycle;
  updateLed();
}

void LedDriver::updateLed() {
  currentIndex = 0;
  digitalWrite(pin, initialState);
  if (!patternMs.empty()) {
    timer.once_ms(patternMs[0], [this] () -> void { changeState(); } );
  }
}

void LedDriver::changeState() {
  if (pin == -1 || patternMs.empty()) {
    return;
  }

  if (++currentIndex >= patternMs.size()) {
    if (cycle) {
      if (patternMs.size() % 2 == 1) {
        initialState = !initialState;
      }
      currentIndex = 0;
    } else {
      return;
    }
  }

  digitalWrite(pin, currentIndex % 2 == 0 ? initialState : !initialState);
  timer.once_ms(patternMs[currentIndex], [this] () -> void { changeState(); } );
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