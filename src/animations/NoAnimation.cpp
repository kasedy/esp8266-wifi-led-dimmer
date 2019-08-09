#include "animations/NoAnimation.h"

#include "dbg.h"
#include "LightController.h"

NoAnimation::NoAnimation(LightController *lightController) : 
    BaseAnimation(lightController) {
  DBG("NoAnimation constructed!\n");
}

NoAnimation::~NoAnimation() {
  DBG("NoAnimation destroyed!\n");
}

void NoAnimation::handle() {
  if (lightController->isStateOnChanged() || lightController->isEffectChanged()) {
    lightController->setAllPinValue(lightController->isOn() ? lightController->getLightBrightness() : 0);
  } else if (lightController->isMaxBrightensChanged()) {
    if (lightController->isOn()) {
      lightController->setAllPinValue(lightController->getLightBrightness());
    }
  }
}

Effect NoAnimation::effect(const char *name) {
  return {name, [] (LightController *lightController) -> BaseAnimation* { return new NoAnimation(lightController); }, 1};
}