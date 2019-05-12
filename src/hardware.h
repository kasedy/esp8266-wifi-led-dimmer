#pragma once

#if defined(HARDWARE_COINTREAU_BOTTLE)
  #define DEVICE_TYPE "cointreau_bottle"
  #define DEVICE_NAME "Cointreau Bottle"
  #define LED_PINS {0, 1, 2, 3}
#elif defined(HARDWARE_COINTREAU_BOTTLE_2)
  #define DEVICE_TYPE "cointreau_bottle_2"
  #define DEVICE_NAME "Cointreau Bottle 2"
  #define LED_PINS {3, 12, 13, 14, 16}
#elif defined(HARDWARE_TEST)
  #define DEVICE_TYPE "test"
  #define DEVICE_NAME "Testing and Development"
  #define LED_PINS {12, 13, 14, 16}
  #define LOGGING true
#elif defined(HARDWARE_KITCHEN_LIGHT)
  #define DEVICE_TYPE "kitchen_light"
  #define DEVICE_NAME "Kitchen Light"
  #define LED_PINS {{0, 2}}
#elif defined(HARDWARE_HANGING_BULBS)
  #define DEVICE_TYPE "hanging_bulbs"
  #define DEVICE_NAME "Hanging bulbs"
  #define LED_PINS {0}  
#endif