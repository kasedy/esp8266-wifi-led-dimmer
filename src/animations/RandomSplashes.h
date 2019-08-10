#pragma once

#include "animations/BaseAnimation.h"

#include <stdint.h>
#include <vector>

class RandomSplashes : public BaseAnimation {
public:
  struct BrightnessSettings {
    uint8_t minBrightness;
    uint8_t maxBrightness;
  };

private:
  struct LedInfo {
    uint8_t brightnessSettingsIndex;
    unsigned long timeToChangeBrightness;
  };

  LedInfo *ledInfo;
  std::vector<BrightnessSettings> brightnessSettings;

  unsigned long getUpdateInterval();
  void resetTimers(unsigned long timeToChangeBrightness);

public:
  RandomSplashes(LightController *lightController);
  RandomSplashes(
    LightController *lightController, 
    std::vector<BrightnessSettings> brightnessSettings);
  virtual ~RandomSplashes();
  virtual void handle() override;

  static Effect effect(const char* name);
  static Effect effect(const char* name, std::vector<BrightnessSettings> brightnessSettings);
};
