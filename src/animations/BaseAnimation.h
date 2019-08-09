#pragma once

#include <functional>

struct LightController;

class BaseAnimation {
protected:
  LightController * const lightController;

public:
  BaseAnimation(LightController *lightController) : lightController(lightController) {}
  virtual ~BaseAnimation() {}
  virtual void handle() = 0;
};

struct Effect {
  const char* name;
  std::function<BaseAnimation* (LightController *lightController)> animationBuilder;
  uint8_t pinsRequires;
};