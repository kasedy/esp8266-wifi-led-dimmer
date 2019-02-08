#include "animations/NoAnimation.h"

#include "dbg.h"
#include "LightState.h"

NoAnimation::NoAnimation(LightState *lightState) : 
    BaseAnimation(lightState) {
  DBG("NoAnimation constructed!\n");
}

NoAnimation::~NoAnimation() {
  DBG("NoAnimation destroyed!\n");
}

void NoAnimation::handle() {
  if (lightState->isStateOnChanged() || lightState->isEffectChanged()) {
    DBG("NoAnimation::handle change state or effect\n");
    lightState->setPinValue(lightState->isOn() ? lightState->getMaxBrightness() : 0);
  } else if (lightState->isMaxBrightensChanged()) {
    DBG("NoAnimation::handle brightness changed\n");
    if (lightState->isOn()) {
      lightState->setPinValue(lightState->getMaxBrightness());
    }
  }
}

Effect NoAnimation::effect(const char *name) {
  return {name, [] (LightState *lightState) -> BaseAnimation* { return new NoAnimation(lightState); }, 1};
}