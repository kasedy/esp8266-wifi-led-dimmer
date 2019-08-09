#include "animations/RandomSplashes.h"

#include "dbg.h"
#include "LightController.h"

RandomSplashes::RandomSplashes(LightController *lightController) : RandomSplashes(lightController, {}) {
}

RandomSplashes::RandomSplashes(
    LightController *lightController, 
    std::vector<BrightnessSettings> bSettings) 
    : BaseAnimation(lightController),
    ledInfo(new LedInfo[lightController->getLedCount()]),
    brightnessSettings(bSettings) {
  DBG("RandomSplashes constructed!\n");
}

RandomSplashes::~RandomSplashes() {
  delete[] ledInfo;
  DBG("RandomSplashes destroyed!\n");
}

void RandomSplashes::handle() {
  if (lightController->isEffectChanged() 
      || lightController->isAnimationSpeedChanged()) {
    resetTimers(micros());
  }

  if (lightController->isMaxBrightensChanged()) {
    if (lightController->isOn()) {
      lightController->setAllPinValue(lightController->getLightBrightness());
      resetTimers(micros() + 500000);
    }
  }

  if (lightController->isStateOnChanged()) {
    if (!lightController->isOn()) {
      lightController->setAllPinValue(0);
    } else {
      resetTimers(micros());
    }
  }

  if (!lightController->isOn()) {
    return;
  }

  for (uint8_t index = 0; index < lightController->getLedCount(); ++index) {
    // time may overflow unsigned long
    unsigned long timeDiff = micros() - ledInfo[index].timeToChangeBrightness;
    if (((unsigned long) timeDiff <= (unsigned long) -timeDiff)) {
      BrightnessSettings brigtnessRange = {0, 255};
      if (brightnessSettings.size() > 0) {
        uint8_t &brightnessSettingsIndex = ledInfo[index].brightnessSettingsIndex;
        if (brightnessSettingsIndex >= brightnessSettings.size()) {
          brightnessSettingsIndex = 0;
        }
        brigtnessRange = brightnessSettings[brightnessSettingsIndex];
        brightnessSettingsIndex += 1;
      }
      uint8_t minBrightness = map(brigtnessRange.minBrightness, 0, 255, 0, lightController->getLightBrightness());
      uint8_t maxBrightness = map(brigtnessRange.maxBrightness, 0, 255, 0, lightController->getLightBrightness());
      if (minBrightness == maxBrightness) {
        lightController->setPinValue(index, minBrightness);
      } else {
        while (true) {
          uint8_t newBrightness = random(minBrightness, maxBrightness + 1);
          if (newBrightness != lightController->getLedBrightness(index)) {
            lightController->setPinValue(index, newBrightness);
            break;
          }
        }
      }
      ledInfo[index].timeToChangeBrightness = micros() + getUpdateInterval();
    }
  }
}

void RandomSplashes::resetTimers(unsigned long newTime) {
  for (uint8_t index = 0; index < lightController->getLedCount(); ++index) {
    ledInfo[index].brightnessSettingsIndex = random(brightnessSettings.size());
    ledInfo[index].timeToChangeBrightness = newTime;
  }
}

unsigned long RandomSplashes::getUpdateInterval() {
  int32_t multiplier = map(lightController->getAnimationSpeed(), 0, 255, -40, 40);
  unsigned long refreshInterval = random(1000000); // microseconds
  if (multiplier < 0) {
    refreshInterval += refreshInterval * (-multiplier) / 10;
  } else if (multiplier > 0) {
    refreshInterval = refreshInterval * 10 / (multiplier + 10);
  } 
  return refreshInterval;
}

Effect RandomSplashes::effect(const char* name) {
  return {name, [] (LightController *lightController) -> BaseAnimation* { return new RandomSplashes(lightController); } };
}

Effect RandomSplashes::effect(const char* name, std::vector<BrightnessSettings> brightnessSettings) {
  return {name, [=] (LightController *lightController) -> BaseAnimation* { return new RandomSplashes(lightController, brightnessSettings); }, 1};
}