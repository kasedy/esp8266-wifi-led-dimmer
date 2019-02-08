#include "animations/FadeCycle.h"

#include "dbg.h"
#include "LightState.h"


FadeCycle::FadeCycle(LightState *lightState) 
    : BaseAnimation(lightState) {
  DBG("FadeCycle constructed!\n");
}

FadeCycle::~FadeCycle() {
  DBG("FadeCycle destroyed!\n");
}

void FadeCycle::handle() {
  if (lightState->isAnimationSpeedChanged()) {
    DBG("Refresh interval changed to %lu;\n", getUpdateInterval());
  }

  if (lightState->isEffectChanged()) {
    if (lightState->isOn()) {
      uint16_t averageBrightness = 0;
      for (size_t i = 0; i < lightState->getLedCount(); ++i) {
        averageBrightness += lightState->getLedBrightness(i);
      }
      changeBrightness(averageBrightness / lightState->getLedCount());
    } else {
      changeBrightness(0);
    }
    return;
  }

  if (lightState->isStateOnChanged()) {
    if (!lightState->isOn()) {
      changeBrightness(0);
      return;
    }
  }

  if (lightState->isMaxBrightensChanged()) {
    DBG("Max brightness changed to %d. Refresh interval changed to %lu;\n", lightState->getMaxBrightness(), getUpdateInterval());
    if (lightState->isOn()) {
      changeBrightness(lightState->getMaxBrightness());
      return;
    }
  }

  if (!lightState->isOn() 
        || lightState->getMaxBrightness() == 0 
        || micros() - lastUpdateTime < getUpdateInterval()) {
    return;
  }

  if (actualBrightness == 0) {
    step = 1;
  } else if (actualBrightness == lightState->getMaxBrightness()) {
    step = -1;
  }
  changeBrightness(actualBrightness + step);
}

void FadeCycle::changeBrightness(uint8_t newBrightness) {
  lightState->setPinValue(newBrightness);
  actualBrightness = newBrightness;
  lastUpdateTime = micros();
}

unsigned long FadeCycle::getUpdateInterval() {
  int32_t multiplier = map(lightState->getAnimationSpeed(), 0, 255, -100, 100);
  if (lightState->getMaxBrightness() == 0) {
    return -1;
  }
  unsigned long refreshInterval = 1000000 / lightState->getMaxBrightness(); // microseconds
  if (multiplier < 0) {
    refreshInterval += refreshInterval * (-multiplier) / 5;
  } else if (multiplier > 0) {
    refreshInterval = refreshInterval * 5 / (multiplier + 5);
  } 
  return refreshInterval;
}
  
Effect FadeCycle::effect(const char* name) {
  return {name, [] (LightState *lightState) -> BaseAnimation* { return new FadeCycle(lightState); }, 1};
}
