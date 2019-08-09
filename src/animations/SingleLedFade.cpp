#include "animations/SingleLedFade.h"

#include <functional>
#include "LightController.h"

SingleLedFade::SingleLedFade(LightController *lightState, uint8_t brightnessOverlap) : 
    BaseAnimation(lightState),
    brightnessOverlap(brightnessOverlap),
    currentLed(0),
    raising(false),
    lastUpdateTime(0) {
}

SingleLedFade::~SingleLedFade() {
}

void SingleLedFade::handle() {
  if (lightState->isEffectChanged()) {
    currentLed = 0;
    for (int i = 0; i < lightState->getLedCount(); ++i) {
      lightState->setPinValue(i, i == currentLed ? lightState->getLightBrightness() : 0);
    }
    return;
  }

  if (lightState->isStateOnChanged()) {
    if (!lightState->isOn()) {
      lightState->setAllPinValue(0);
      return;
    }
  }

  if (lightState->isMaxBrightensChanged()) {
    if (lightState->isOn()) {
      for (int i = 0; i < lightState->getLedCount(); ++i) {
        lightState->setPinValue(i, i == currentLed ? lightState->getLightBrightness() : 0);
      }
      return;
    }
  }

  if (!lightState->isOn() 
        || lightState->getLightBrightness() == 0 
        || micros() - lastUpdateTime < getUpdateInterval()) {
    return;
  }

  uint8_t currentLedBrightness = lightState->getLedBrightness(currentLed);
  if (currentLedBrightness == 0) {
    currentLed = getNextLedIndex();
    currentLedBrightness = lightState->getLedBrightness(currentLed);
    raising = true;
  }
  
  if (currentLedBrightness == lightState->getLightBrightness()) {
    raising = false;
  }

  if (!raising && currentLedBrightness <= brightnessOverlap) {
    uint8_t nextLedIndex = getNextLedIndex();
    uint8_t nextLedBrightness = lightState->getLedBrightness(nextLedIndex);
    lightState->setPinValue(nextLedIndex, nextLedBrightness + 1);
  }

  lightState->setPinValue(currentLed, currentLedBrightness + (raising ? 1 : -1));
  lastUpdateTime = micros();
}

uint8_t SingleLedFade::getNextLedIndex() const {
  uint8_t ledIndex = currentLed + 1;
  if (ledIndex >= lightState->getLedCount()) {
    ledIndex = 0;
  } 
  return ledIndex;
}

unsigned long SingleLedFade::getUpdateInterval() {
  int32_t multiplier = map(lightState->getAnimationSpeed(), 0, 255, -50, 50);
  if (lightState->getLightBrightness() == 0) {
    return -1;
  }
  unsigned long refreshInterval = 2000000 / lightState->getLightBrightness(); // microseconds
  if (multiplier < 0) {
    refreshInterval += refreshInterval * (-multiplier) / 10;
  } else if (multiplier > 0) {
    refreshInterval = refreshInterval * 10 / (multiplier + 10);
  } 
  return refreshInterval;
}

Effect SingleLedFade::effect(const char* name, uint8_t brightnessOverlap) {
  return {name, [=] (LightController *lightState) -> BaseAnimation* { return new SingleLedFade(lightState, brightnessOverlap); }, 2};
}