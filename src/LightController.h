#pragma once

#include "dbg.h"
#include "animations/BaseAnimation.h"

#include <Esp.h>

class LightController {
  struct PinStatus {
    PinStatus(uint8_t gpio) : gpio(1, gpio), brightness(255) {}
    PinStatus(std::initializer_list<uint8_t> gpio) : gpio(gpio), brightness(255) {}
    const std::vector<uint8_t> gpio;
    uint8_t brightness;
  };

  uint8_t brightness;
  uint8_t animationSpeed;
  std::vector<PinStatus> pins;
  const std::vector<Effect> effects;
  BaseAnimation *currentEffect;
  uint8_t currentAnimationIndex;
  bool stateOn:1;
  bool brightnessChanged:1;
  bool stateOnChanged:1;
  bool effectChanged:1;
  bool animationSpeedChanged:1;

public:
  LightController(const std::vector<PinStatus> &pinsGpio, const std::vector<Effect> &effects);
  ~LightController();

  void loop();

  bool isOn() const;
  void setStateOn(bool newStateOn);
  void toggleState();

  size_t getAnimationCount();
  uint8_t getCurrentAnimationIndex();
  const char* getAnimationName(size_t index);
  const char* getCurrentAnimationName() const;
  void nextAnimation();
  void setAnimationByIndex(uint8_t animationIndex);
  void setAnimationByName(const char* effectName);

  void setLightBrightness(uint8_t newMaxBrightness);
  uint8_t getLightBrightness() const;

  void setAnimationSpeed(uint8_t newAnimationSpeed);
  uint8_t getAnimationSpeed() const;

  void setPinValue(uint8_t pinIndex, uint8_t brightness);
  void setAllPinValue(uint8_t brightness);
  uint8_t getLedBrightness(uint8_t pinIndex) const;

  uint8_t getLedCount() const;
  bool isMaxBrightensChanged() const;
  bool isStateOnChanged() const;
  bool isEffectChanged() const;
  bool isAnimationSpeedChanged() const;
  bool isChanged() const {
    return isMaxBrightensChanged() 
        || isStateOnChanged() 
        || isEffectChanged() 
        || isAnimationSpeedChanged();
  }
};