#pragma once

#include <functional>

struct LightState;

class BaseAnimation {
protected:
  LightState * const lightState;

public:
  BaseAnimation(LightState *lightState) : lightState(lightState) {}
  virtual ~BaseAnimation() {}
  virtual void handle() = 0;
};

struct Effect {
  const char* name;
  std::function<BaseAnimation* (LightState *lightState)> animationBuilder;
  uint8_t pinsRequires;
};