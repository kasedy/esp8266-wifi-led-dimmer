#pragma once

#include "animations/BaseAnimation.h"

/*
 * No animation. Just jump from one colour to another.
 */ 
class NoAnimation : public BaseAnimation {
public:
  NoAnimation(LightController *lightController);
  virtual ~NoAnimation();
  virtual void handle() override;

  static Effect effect(const char* name);
};