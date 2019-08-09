#pragma once

class LightController;

class AbstractCapacitiveSensorButton {
public:
  static AbstractCapacitiveSensorButton* create(LightController* lightState);
  virtual void loop();
};