#pragma once

class LightState;

class AbstractCapacitiveSensorButton {
public:
  static AbstractCapacitiveSensorButton* create(LightState* lightState);
  virtual void loop();
};