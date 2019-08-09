#pragma once

#include <Arduino.h>
#include <stdint.h>
#include <vector>
#include <Ticker.h>

class LedDriver {
  Ticker timer;
  uint8_t pin;
  bool initialState;
  std::vector<uint32_t> patternMs;
  uint8_t currentIndex;
  bool cycle;

public:
  LedDriver(uint8_t pin, bool state=LOW);
  void setPattern(std::vector<uint32_t> pattern, bool initialState = LOW, bool cycle = true);
  void blink(uint32_t interval);
  void setLow();
  void setHigh();
private:
  void updateLed();
  void changeState();
};