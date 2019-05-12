#pragma once

#include "dbg.h"
#include <ESP.h>

#include "animations/BaseAnimation.h"

class LightState {
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
  const char *currentEffectName;
  bool stateOn:1;
  bool brightnessChanged:1;
  bool stateOnChanged:1;
  bool effectChanged:1;
  bool animationSpeedChanged:1;

public:
  LightState(const std::vector<PinStatus> &pinsGpio, const std::vector<Effect> &effects);
  ~LightState();

  size_t getAvailableAnimationCount();
  const char* getAnimationName(size_t index);

  uint8_t getLedCount() const;
  static int convertBrigtnessToOwmDutyCycle(uint8_t brightness);
  void setPinValue(uint8_t pinIndex, uint8_t brightness);
  void setPinValue(uint8_t brightness);
  void setMaxBrightness(uint8_t newMaxBrightness);
  void setAnimationSpeed(uint8_t newAnimationSpeed);
  void setStateOn(bool newStateOn);
  void toggleState() {
    setStateOn(!isOn());
  }
  void setEffect(const char* effectName);
  bool nextAnimation();
  void handle();
  void setup();
  uint8_t getLedBrightness(uint8_t pinIndex) const;
  uint8_t getMaxBrightness() const;
  bool hasCurrentEffect() const;
  const char* getCurrentEffectName() const;
  uint8_t getAnimationSpeed() const;
  bool isOn() const;

  bool isMaxBrightensChanged() const;
  bool isStateOnChanged() const;
  bool isEffectChanged() const;
  bool isAnimationSpeedChanged() const;

private:
  void setupAnimation(const Effect &effectInfo);
};