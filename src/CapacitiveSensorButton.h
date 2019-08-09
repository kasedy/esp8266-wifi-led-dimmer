#pragma once

class LightController;

class AbstractCapacitiveSensorButton {
public:
  static AbstractCapacitiveSensorButton* create(LightController* lightController);
  virtual void loop();
};