#pragma once

#include "animations/BaseAnimation.h"

#include <stdint.h>

/*
 * Fade on switch on-off.
 */
class FadeSwitch : public BaseAnimation {
  uint8_t currentBrightness = 0;
  unsigned long lastUpdateTime = 0;
  
  unsigned long getUpdateInterval();
  void updateBrightness(uint8_t newValue);
  uint8_t getEndBrightness() const;
  uint8_t getStartBrightness() const;
public:
  FadeSwitch(LightController *lightController);
  virtual ~FadeSwitch();
  virtual void handle() override;

  static Effect effect(const char* name);
};
