#pragma once

#include <Arduino.h>
#include <stdint.h>
#include <vector>

class LedDriver {
  uint8_t pin;
  bool initialState;
  std::vector<uint32_t> patternMs;
  uint8_t currentIndex;
  uint32_t lastUpdateTime;

public:
  LedDriver(uint8_t pin);
  void setPattern(std::vector<uint32_t> pattern, bool initialState = LOW);
  void blink(uint32_t interval);
  void setLow();
  void setHigh();
  void loop();

private:
  void init();
};