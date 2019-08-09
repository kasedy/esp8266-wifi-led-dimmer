#include "LightState.h"

#include "helpers.h"

#include "animations/all.h"

LightState::LightState(const std::vector<PinStatus> &pinsGpio, const std::vector<Effect> &effects) :
    brightness(255),
    animationSpeed(255 / 2),
    pins(pinsGpio), 
    effects(filter_vector(effects, [&](Effect e){ return pinsGpio.size() >= e.pinsRequires; })),
    currentEffect(nullptr),
    currentAnimationIndex(-1),
    stateOn(false),
    brightnessChanged(false),
    stateOnChanged(false),
    effectChanged(false) {
  // TODO: make sure we have at least one animation
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
  for (size_t animationIndex = 0; animationIndex < effects.size(); ++animationIndex) {
    if (strcmp(effects[animationIndex].name, effectName) == 0) {
      setupAnimation(animationIndex);
      return;
    }
  }
}

void LightState::nextAnimation() {
  uint8_t animationIndex = getCurrentAnimationIndex();
  if (++animationIndex >= effects.size()) {
    animationIndex = 0;
  }
  setupAnimation(animationIndex);
}

void LightState::setupAnimation(uint8_t animationIndex) {
  if (currentAnimationIndex == animationIndex 
      || animationIndex >= effects.size()) {
    return;
  }
  if (currentEffect) {
    delete currentEffect;
  }
  const Effect &effectInfo = effects[animationIndex];
  currentEffect = effectInfo.animationBuilder(this);
  effectChanged = true;
  currentAnimationIndex = animationIndex;
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
  return currentAnimationIndex != -1;
}

uint8_t LightState::getCurrentAnimationIndex() {
  return currentAnimationIndex;
}

const char* LightState::getCurrentEffectName() const {
  return effects[currentAnimationIndex].name;
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