#include "animations/FadeSwitch.h"

#include "dbg.h"
#include "helpers.h"
#include "LightState.h"

FadeSwitch::FadeSwitch(LightState *lightState) : 
    BaseAnimation(lightState) {
  DBG("FadeSwitch constructed;\n");
}

FadeSwitch::~FadeSwitch() {
  DBG("FadeSwitch destroyed;\n");
}

void FadeSwitch::handle() {
  if (lightState->isMaxBrightensChanged() 
      || lightState->isEffectChanged()) {
    updateBrightness(getEndBrightness());
    return;
  }

  if (currentBrightness == getEndBrightness()
      || micros() - lastUpdateTime < getUpdateInterval()) {
    return;
  }
  updateBrightness(currentBrightness + sgn(getEndBrightness(), currentBrightness));
}

void FadeSwitch::updateBrightness(uint8_t newValue) {
    lastUpdateTime = micros();
    currentBrightness = newValue;
    lightState->setPinValue(currentBrightness);
}

uint8_t FadeSwitch::getEndBrightness() const {
  return lightState->isOn() ? lightState->getMaxBrightness() : 0;
}

uint8_t FadeSwitch::getStartBrightness() const {
  return lightState->isOn() ? 0 : lightState->getMaxBrightness();
}

unsigned long FadeSwitch::getUpdateInterval() {
  int32_t multiplier = map(lightState->getAnimationSpeed(), 0, 255, -50, 50);
  uint8_t brightnessDifference = difference(getEndBrightness(), getStartBrightness());
  if (brightnessDifference == 0) {
    return -1;
  }
  unsigned long refreshInterval = 1000000 / brightnessDifference; // microseconds
  if (multiplier < 0) {
    refreshInterval += refreshInterval * (-multiplier) / 10;
  } else if (multiplier > 0) {
    refreshInterval = refreshInterval * 10 / (multiplier + 10);
  } 
  return refreshInterval;
}

Effect FadeSwitch::effect(const char* name) {
  return {name, [] (LightState *lightState) -> BaseAnimation* { return new FadeSwitch(lightState); }, 1};
}