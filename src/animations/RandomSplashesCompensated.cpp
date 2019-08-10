#include "animations/RandomSplashesCompensated.h"

#include "dbg.h"
#include "LightController.h"


RandomSplashesCompensated::RandomSplashesCompensated(LightController *lightController) 
    : BaseAnimation(lightController),
    timeToChangeBrightness(0),
    currentLedIndex(0) {
  DBG("RandomSplashesCompensated constructed!\n");
}

RandomSplashesCompensated::~RandomSplashesCompensated() {
  DBG("RandomSplashesCompensated destroyed!\n");
}

void RandomSplashesCompensated::handle() {
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

  if (!lightController->isOn() || lightController->getLightBrightness() == 0) {
    return;
  }

  unsigned long timeDiff = micros() - timeToChangeBrightness;
  if (((unsigned long) timeDiff > (unsigned long) -timeDiff)) {
    return;
  }

  if (currentLedIndex >= lightController->getLedCount()) {
    currentLedIndex = 0;
  }

  while (true) {
    uint8_t newBrightness = random(lightController->getLightBrightness());
    if (newBrightness != lightController->getLedBrightness(currentLedIndex)) {
      lightController->setPinValue(currentLedIndex, newBrightness);
      break;
    }
  }

  if (lightController->getLedCount() > 1) {
    int32_t brightnessTotal = 0;
    for (uint8_t i = 0; i < lightController->getLedCount(); ++i) {
      brightnessTotal += lightController->getLedBrightness(i);
    }
    
    // one led off the other MAX, it is maximum brightness we could achive. 
    int32_t idealBrightness = lightController->getLightBrightness() * (lightController->getLedCount() - 1);
    int32_t brightnessDelta = (idealBrightness - brightnessTotal) / (lightController->getLedCount() - 1);

    for (uint8_t i = 0; i < lightController->getLedCount(); ++i) {
      if (i != currentLedIndex) {
        int32_t newBrightness = lightController->getLedBrightness(i) + brightnessDelta;
        lightController->setPinValue(i, constrain(newBrightness, 0, lightController->getLightBrightness()));
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
  int32_t multiplier = map(lightController->getAnimationSpeed(), 0, 255, -40, 40);
  unsigned long refreshInterval = random(500000); // microseconds
  if (multiplier < 0) {
    refreshInterval += refreshInterval * (-multiplier) / 10;
  } else if (multiplier > 0) {
    refreshInterval = refreshInterval * 10 / (multiplier + 10);
  } 
  return refreshInterval;
}

Effect RandomSplashesCompensated::effect(const char* name) {
  return {name, [] (LightController *lightController) -> BaseAnimation* { return new RandomSplashesCompensated(lightController); }, 3};
}