#pragma once

template<typename ValueType, typename AccumType>
struct AverageValueCalculator { 
  AccumType accumulator = 0;
  uint16_t counter = 0;
  ValueType minValue = 0;
  ValueType maxValue = 0;

  void addMeasurement(ValueType val) {
    if (counter == 0) {
      minValue = val;
      maxValue = val;
    } else {
      minValue = min(minValue, val);
      maxValue = max(maxValue, val);
    }
    accumulator += val;
    counter += 1;
  }

  ValueType getAverage() {
    return counter == 0 ? 0 : (accumulator / counter);
  }

  ValueType getDelta() {
    return maxValue - minValue;
  }

  uint16_t getCounter() {
    return counter;
  }

  void reset() {
    accumulator = 0;
    counter = 0;
    minValue = 0;
    maxValue = 0;
  }
};