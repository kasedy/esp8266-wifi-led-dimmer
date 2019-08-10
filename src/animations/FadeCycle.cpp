#include "animations/FadeCycle.h"

#include "dbg.h"
#include "LightController.h"


FadeCycle::FadeCycle(LightController *lightController) 
    : BaseAnimation(lightController) {
  DBG("FadeCycle constructed!\n");
}

FadeCycle::~FadeCycle() {
  DBG("FadeCycle destroyed!\n");
}

void FadeCycle::handle() {
  if (lightController->isAnimationSpeedChanged()) {
    DBG("Refresh interval changed to %lu;\n", getUpdateInterval());
  }

  if (lightController->isEffectChanged()) {
    if (lightController->isOn()) {
      uint16_t averageBrightness = 0;
      for (size_t i = 0; i < lightController->getLedCount(); ++i) {
        averageBrightness += lightController->getLedBrightness(i);
      }
      changeBrightness(averageBrightness / lightController->getLedCount());
    } else {
      changeBrightness(0);
    }
    return;
  }

  if (lightController->isStateOnChanged()) {
    if (!lightController->isOn()) {
      changeBrightness(0);
      return;
    }
  }

  if (lightController->isMaxBrightensChanged()) {
    DBG("Max brightness changed to %d. Refresh interval changed to %lu;\n", lightController->getLightBrightness(), getUpdateInterval());
    if (lightController->isOn()) {
      changeBrightness(lightController->getLightBrightness());
      return;
    }
  }

  if (!lightController->isOn() 
        || lightController->getLightBrightness() == 0 
        || micros() - lastUpdateTime < getUpdateInterval()) {
    return;
  }

  if (actualBrightness == 0) {
    step = 1;
  } else if (actualBrightness == lightController->getLightBrightness()) {
    step = -1;
  }
  changeBrightness(actualBrightness + step);
}

void FadeCycle::changeBrightness(uint8_t newBrightness) {
  lightController->setAllPinValue(newBrightness);
  actualBrightness = newBrightness;
  lastUpdateTime = micros();
}

unsigned long FadeCycle::getUpdateInterval() {
  int32_t multiplier = map(lightController->getAnimationSpeed(), 0, 255, -100, 100);
  if (lightController->getLightBrightness() == 0) {
    return -1;
  }
  unsigned long refreshInterval = 1000000 / lightController->getLightBrightness(); // microseconds
  if (multiplier < 0) {
    refreshInterval += refreshInterval * (-multiplier) / 5;
  } else if (multiplier > 0) {
    refreshInterval = refreshInterval * 5 / (multiplier + 5);
  } 
  return refreshInterval;
}
  
Effect FadeCycle::effect(const char* name) {
  return {name, [] (LightController *lightController) -> BaseAnimation* { return new FadeCycle(lightController); }, 1};
}
