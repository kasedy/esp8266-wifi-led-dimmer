#pragma once

#include <functional>

struct LightController;

class BaseAnimation {
protected:
  LightController * const lightState;

public:
  BaseAnimation(LightController *lightState) : lightState(lightState) {}
  virtual ~BaseAnimation() {}
  virtual void handle() = 0;
};

struct Effect {
  const char* name;
  std::function<BaseAnimation* (LightController *lightState)> animationBuilder;
  uint8_t pinsRequires;
};