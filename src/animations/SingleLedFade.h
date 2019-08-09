#pragma once

#include "animations/BaseAnimation.h"

#include <stdint.h>

/*
 * Fade on switch on-off.
 */
class SingleLedFade : public BaseAnimation {
  const uint8_t brightnessOverlap; // when next led starts
  uint8_t currentLed;
  bool raising;
  unsigned long lastUpdateTime;

  uint8_t getNextLedIndex() const;
  unsigned long getUpdateInterval();
public:
  SingleLedFade(LightController *lightController, uint8_t brightnessOverlap);
  virtual ~SingleLedFade();
  virtual void handle() override;

  static Effect effect(const char* name, uint8_t brightnessOverlap);
};
