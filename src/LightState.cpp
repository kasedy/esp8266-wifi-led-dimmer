#include "LightState.h"

#include "helpers.h"
#include <ESP.h>

#include "animations/all.h"

LightState::LightState(const std::vector<PinStatus> &pinsGpio, const std::vector<Effect> &effects) :
    brightness(255),
    animationSpeed(255 / 2),
    pins(pinsGpio), 
    effects(filter_vector(effects, [&](Effect e){ return pinsGpio.size() >= e.pinsRequires; })),
    currentEffect(nullptr),
    currentEffectName(nullptr),
    stateOn(true),
    brightnessChanged(false),
    stateOnChanged(false),
    effectChanged(false) {
}

LightState::~LightState() {
}

size_t LightState::getAvailableAnimationCount() {
  return effects.size();
}

const char* LightState::getAnimationName(size_t index) {
  return effects[index].name;
}

uint8_t LightState::getLedCount() const {
  return pins.size();
}   

int LightState::convertBrigtnessToOwmDutyCycle(uint8_t brightness) {
  return map(brightness, 0, 255, 0, 1023);
}

void LightState::setPinValue(uint8_t pinIndex, uint8_t brightness) {
  int pwmDuty = convertBrigtnessToOwmDutyCycle(brightness);
  for (int gpio : pins[pinIndex].gpio) {
    analogWrite(gpio, pwmDuty);
  }
  pins[pinIndex].brightness = brightness;
}

void LightState::setPinValue(uint8_t brightness) {
  for (uint8_t i = 0; i < getLedCount(); ++i) {
    setPinValue(i, brightness);
  }    
}

void LightState::setMaxBrightness(uint8_t newMaxBrightness) {
  if (brightness == newMaxBrightness) {
    return;
  }
  brightness = newMaxBrightness;
  brightnessChanged = true;
}

void LightState::setAnimationSpeed(uint8_t newAnimationSpeed) {
  if (animationSpeed == newAnimationSpeed) {
    return;
  }
  animationSpeed = newAnimationSpeed;
  animationSpeedChanged = true;
}

void LightState::setStateOn(bool newStateOn) {
  if (stateOn == newStateOn) {
    return;
  }
  stateOn = newStateOn;
  stateOnChanged = true;
}

void LightState::setEffect(const char* effectName) {
    if (currentEffectName && strcmp(effectName, currentEffectName) == 0) {
      return;
    }
    for (size_t i = 0; i < effects.size(); ++i) {
        const Effect &effectInfo = effects[i];
        if (strcmp(effectInfo.name, effectName) == 0) {
            if (currentEffect) {
                delete currentEffect;
            }
            currentEffect = effectInfo.animationBuilder(this);
            currentEffectName = effectInfo.name;
            effectChanged = true;
            return;
        }
    }
}

void LightState::handle() {
  currentEffect->handle();
  brightnessChanged = false;
  stateOnChanged = false;
  effectChanged = false;
  animationSpeedChanged = false;
}

void LightState::setup() {
  if (effects.empty()) {
    currentEffect = new NoAnimation(this);
  } else {
    setEffect(effects[0].name);
  }
  for (int i = 0; i < getLedCount(); ++i) {
    for (int gpio : pins[i].gpio) {
      DBG("Set pin %d to output\n", gpio);
      pinMode(gpio, OUTPUT);
      setPinValue(i, pins[i].brightness);
    }
  }
}

uint8_t LightState::getLedBrightness(uint8_t pinIndex) const {
  return pins[pinIndex].brightness;
}

uint8_t LightState::getMaxBrightness() const {
  return brightness;
}

bool LightState::hasCurrentEffect() const {
  return currentEffectName;
}

const char* LightState::getCurrentEffectName() const {
  return currentEffectName;
}

uint8_t LightState::getAnimationSpeed() const {
  return animationSpeed;
}

bool LightState::isOn() const {
  return stateOn;
}

bool LightState::isMaxBrightensChanged() const {
  return brightnessChanged;
}

bool LightState::isStateOnChanged() const {
  return stateOnChanged;
}

bool LightState::isEffectChanged() const {
  return effectChanged;
}

bool LightState::isAnimationSpeedChanged() const {
  return animationSpeedChanged;
}