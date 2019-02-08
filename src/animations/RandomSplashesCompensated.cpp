#include "animations/RandomSplashesCompensated.h"

#include "dbg.h"
#include "LightState.h"


RandomSplashesCompensated::RandomSplashesCompensated(LightState *lightState) 
    : BaseAnimation(lightState),
    timeToChangeBrightness(0),
    currentLedIndex(0) {
  DBG("RandomSplashesCompensated constructed!\n");
}

RandomSplashesCompensated::~RandomSplashesCompensated() {
  DBG("RandomSplashesCompensated destroyed!\n");
}

void RandomSplashesCompensated::handle() {
  if (lightState->isEffectChanged() 
      || lightState->isAnimationSpeedChanged()) {
    resetTimers(micros());
  }

  if (lightState->isMaxBrightensChanged()) {
    if (lightState->isOn()) {
      lightState->setPinValue(lightState->getMaxBrightness());
      resetTimers(micros() + 500000);
    }
  }

  if (lightState->isStateOnChanged()) {
    if (!lightState->isOn()) {
      lightState->setPinValue(0);
    } else {
      resetTimers(micros());
    }
  }

  if (!lightState->isOn() || lightState->getMaxBrightness() == 0) {
    return;
  }

  unsigned long timeDiff = micros() - timeToChangeBrightness;
  if (((unsigned long) timeDiff > (unsigned long) -timeDiff)) {
    return;
  }

  if (currentLedIndex >= lightState->getLedCount()) {
    currentLedIndex = 0;
  }

  while (true) {
    uint8_t newBrightness = random(lightState->getMaxBrightness());
    if (newBrightness != lightState->getLedBrightness(currentLedIndex)) {
      lightState->setPinValue(currentLedIndex, newBrightness);
      break;
    }
  }

  if (lightState->getLedCount() > 1) {
    int32_t brightnessTotal = 0;
    for (uint8_t i = 0; i < lightState->getLedCount(); ++i) {
      brightnessTotal += lightState->getLedBrightness(i);
    }
    
    // one led off the other MAX, it is maximum brightness we could achive. 
    int32_t idealBrightness = lightState->getMaxBrightness() * (lightState->getLedCount() - 1);
    int32_t brightnessDelta = (idealBrightness - brightnessTotal) / (lightState->getLedCount() - 1);

    for (uint8_t i = 0; i < lightState->getLedCount(); ++i) {
      if (i != currentLedIndex) {
        int32_t newBrightness = lightState->getLedBrightness(i) + brightnessDelta;
        lightState->setPinValue(i, constrain(newBrightness, 0, lightState->getMaxBrightness()));
      }    
    }
  }

  ++currentLedIndex;
  timeToChangeBrightness = micros() + getUpdateInterval();
}

void RandomSplashesCompensated::resetTimers(unsigned long newTime) {
  timeToChangeBrightness = newTime;
}

unsigned long RandomSplashesCompensated::getUpdateInterval() {
  int32_t multiplier = map(lightState->getAnimationSpeed(), 0, 255, -40, 40);
  unsigned long refreshInterval = random(500000); // microseconds
  if (multiplier < 0) {
    refreshInterval += refreshInterval * (-multiplier) / 10;
  } else if (multiplier > 0) {
    refreshInterval = refreshInterval * 10 / (multiplier + 10);
  } 
  return refreshInterval;
}

Effect RandomSplashesCompensated::effect(const char* name) {
  return {name, [] (LightState *lightState) -> BaseAnimation* { return new RandomSplashesCompensated(lightState); }, 3};
}