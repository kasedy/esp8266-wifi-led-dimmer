#include "animations/SingleLedFade.h"

#include <functional>
#include "LightController.h"

SingleLedFade::SingleLedFade(LightController *lightController, uint8_t brightnessOverlap) : 
    BaseAnimation(lightController),
    brightnessOverlap(brightnessOverlap),
    currentLed(0),
    raising(false),
    lastUpdateTime(0) {
}

SingleLedFade::~SingleLedFade() {
}

void SingleLedFade::handle() {
  if (lightController->isEffectChanged()) {
    currentLed = 0;
    for (int i = 0; i < lightController->getLedCount(); ++i) {
      lightController->setPinValue(i, i == currentLed ? lightController->getLightBrightness() : 0);
    }
    return;
  }

  if (lightController->isStateOnChanged()) {
    if (!lightController->isOn()) {
      lightController->setAllPinValue(0);
      return;
    }
  }

  if (lightController->isMaxBrightensChanged()) {
    if (lightController->isOn()) {
      for (int i = 0; i < lightController->getLedCount(); ++i) {
        lightController->setPinValue(i, i == currentLed ? lightController->getLightBrightness() : 0);
      }
      return;
    }
  }

  if (!lightController->isOn() 
        || lightController->getLightBrightness() == 0 
        || micros() - lastUpdateTime < getUpdateInterval()) {
    return;
  }

  uint8_t currentLedBrightness = lightController->getLedBrightness(currentLed);
  if (currentLedBrightness == 0) {
    currentLed = getNextLedIndex();
    currentLedBrightness = lightController->getLedBrightness(currentLed);
    raising = true;
  }
  
  if (currentLedBrightness == lightController->getLightBrightness()) {
    raising = false;
  }

  if (!raising && currentLedBrightness <= brightnessOverlap) {
    uint8_t nextLedIndex = getNextLedIndex();
    uint8_t nextLedBrightness = lightController->getLedBrightness(nextLedIndex);
    lightController->setPinValue(nextLedIndex, nextLedBrightness + 1);
  }

  lightController->setPinValue(currentLed, currentLedBrightness + (raising ? 1 : -1));
  lastUpdateTime = micros();
}

uint8_t SingleLedFade::getNextLedIndex() const {
  uint8_t ledIndex = currentLed + 1;
  if (ledIndex >= lightController->getLedCount()) {
    ledIndex = 0;
  } 
  return ledIndex;
}

unsigned long SingleLedFade::getUpdateInterval() {
  int32_t multiplier = map(lightController->getAnimationSpeed(), 0, 255, -50, 50);
  if (lightController->getLightBrightness() == 0) {
    return -1;
  }
  unsigned long refreshInterval = 2000000 / lightController->getLightBrightness(); // microseconds
  if (multiplier < 0) {
    refreshInterval += refreshInterval * (-multiplier) / 10;
  } else if (multiplier > 0) {
    refreshInterval = refreshInterval * 10 / (multiplier + 10);
  } 
  return refreshInterval;
}

Effect SingleLedFade::effect(const char* name, uint8_t brightnessOverlap) {
  return {name, [=] (LightController *lightController) -> BaseAnimation* { return new SingleLedFade(lightController, brightnessOverlap); }, 2};
}