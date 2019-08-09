#pragma once

#include "animations/BaseAnimation.h"

#include <stdint.h>

class RandomSplashesCompensated : public BaseAnimation {
  unsigned long timeToChangeBrightness;
  uint8_t currentLedIndex;

  unsigned long getUpdateInterval();
  void resetTimers(unsigned long timeToChangeBrightness);

public:
  RandomSplashesCompensated(LightController *lightController);
  virtual ~RandomSplashesCompensated();
  virtual void handle() override;

  static Effect effect(const char* name);
};
