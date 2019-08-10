#include <Arduino.h>
#include <EEPROM.h>
#include <unity.h>


void test_basic(void) {
  //TODO: place test body here  
}

void setup() {
    // NOTE!!! Wait for >2 secs
    // if board doesn't support software reset via Serial.DTR/RTS
    delay(2000);

    UNITY_BEGIN();
    RUN_TEST(test_basic);
    UNITY_END();
}

void loop() {
}
