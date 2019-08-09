#include "animations/NoAnimation.h"

#include "dbg.h"
#include "LightController.h"

NoAnimation::NoAnimation(LightController *lightState) : 
    BaseAnimation(lightState) {
  DBG("NoAnimation constructed!\n");
}

NoAnimation::~NoAnimation() {
  DBG("NoAnimation destroyed!\n");
}

void NoAnimation::handle() {
  if (lightState->isStateOnChanged() || lightState->isEffectChanged()) {
    lightState->setAllPinValue(lightState->isOn() ? lightState->getLightBrightness() : 0);
  } else if (lightState->isMaxBrightensChanged()) {
    if (lightState->isOn()) {
      lightState->setAllPinValue(lightState->getLightBrightness());
    }
  }
}

Effect NoAnimation::effect(const char *name) {
  return {name, [] (LightController *lightState) -> BaseAnimation* { return new NoAnimation(lightState); }, 1};
}