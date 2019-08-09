#include "animations/RandomSplashes.h"

#include "dbg.h"
#include "LightController.h"

RandomSplashes::RandomSplashes(LightController *lightState) : RandomSplashes(lightState, {}) {
}

RandomSplashes::RandomSplashes(
    LightController *lightState, 
    std::vector<BrightnessSettings> bSettings) 
    : BaseAnimation(lightState),
    ledInfo(new LedInfo[lightState->getLedCount()]),
    brightnessSettings(bSettings) {
  DBG("RandomSplashes constructed!\n");
}

RandomSplashes::~RandomSplashes() {
  delete[] ledInfo;
  DBG("RandomSplashes destroyed!\n");
}

void RandomSplashes::handle() {
  if (lightState->isEffectChanged() 
      || lightState->isAnimationSpeedChanged()) {
    resetTimers(micros());
  }

  if (lightState->isMaxBrightensChanged()) {
    if (lightState->isOn()) {
      lightState->setAllPinValue(lightState->getLightBrightness());
      resetTimers(micros() + 500000);
    }
  }

  if (lightState->isStateOnChanged()) {
    if (!lightState->isOn()) {
      lightState->setAllPinValue(0);
    } else {
      resetTimers(micros());
    }
  }

  if (!lightState->isOn()) {
    return;
  }

  for (uint8_t index = 0; index < lightState->getLedCount(); ++index) {
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
      uint8_t minBrightness = map(brigtnessRange.minBrightness, 0, 255, 0, lightState->getLightBrightness());
      uint8_t maxBrightness = map(brigtnessRange.maxBrightness, 0, 255, 0, lightState->getLightBrightness());
      if (minBrightness == maxBrightness) {
        lightState->setPinValue(index, minBrightness);
      } else {
        while (true) {
          uint8_t newBrightness = random(minBrightness, maxBrightness + 1);
          if (newBrightness != lightState->getLedBrightness(index)) {
            lightState->setPinValue(index, newBrightness);
            break;
          }
        }
      }
      ledInfo[index].timeToChangeBrightness = micros() + getUpdateInterval();
    }
  }
}

void RandomSplashes::resetTimers(unsigned long newTime) {
  for (uint8_t index = 0; index < lightState->getLedCount(); ++index) {
    ledInfo[index].brightnessSettingsIndex = random(brightnessSettings.size());
    ledInfo[index].timeToChangeBrightness = newTime;
  }
}

unsigned long RandomSplashes::getUpdateInterval() {
  int32_t multiplier = map(lightState->getAnimationSpeed(), 0, 255, -40, 40);
  unsigned long refreshInterval = random(1000000); // microseconds
  if (multiplier < 0) {
    refreshInterval += refreshInterval * (-multiplier) / 10;
  } else if (multiplier > 0) {
    refreshInterval = refreshInterval * 10 / (multiplier + 10);
  } 
  return refreshInterval;
}

Effect RandomSplashes::effect(const char* name) {
  return {name, [] (LightController *lightState) -> BaseAnimation* { return new RandomSplashes(lightState); } };
}

Effect RandomSplashes::effect(const char* name, std::vector<BrightnessSettings> brightnessSettings) {
  return {name, [=] (LightController *lightState) -> BaseAnimation* { return new RandomSplashes(lightState, brightnessSettings); }, 1};
}