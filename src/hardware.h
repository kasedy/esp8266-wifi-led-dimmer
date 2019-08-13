#pragma once

#if defined(HARDWARE_COINTREAU_BOTTLE)
  #define DEVICE_TYPE "cointreau_bottle"
  #define DEVICE_NAME "Cointreau Bottle"
  #define LED_PINS {0, 1, 2, 3}
#elif defined(HARDWARE_COINTREAU_BOTTLE_2)
  #define DEVICE_TYPE "cointreau_bottle_2"
  #define DEVICE_NAME "Cointreau Bottle 2"
  #define LED_PINS {3, 12, 13, 16}
  #define CAPACITIVE_SENSOR_SEND_PIN 5
  #define CAPACITIVE_SENSOR_RECEIVE_PIN 4
#elif defined(HARDWARE_TEST)
  #define DEVICE_TYPE "DevEnv"
  #define DEVICE_NAME "Testing and Development"
  #define LED_PINS {3, 12, 13, 14, 16}
  #define LOGGING true
 #define CAPACITIVE_SENSOR_SEND_PIN 5
 #define CAPACITIVE_SENSOR_RECEIVE_PIN 4
#elif defined(HARDWARE_KITCHEN_LIGHT)
  #define DEVICE_TYPE "kitchen_light"
  #define DEVICE_NAME "Kitchen Light"
  #define LED_PINS {{0, 2}}
  #define NO_WEB_PORTAL
#elif defined(HARDWARE_HANGING_BULBS)
  #define DEVICE_TYPE "hanging_bulbs"
  #define DEVICE_NAME "Hanging bulbs"
  #define LED_PINS {0}  
#endif

#define LEAD_INDICATOR_PIN 2

#define MQTT_SERVER "raspberrypi.local"
#define MQTT_PORT 1883

#define CONFIG_MQTT_PAYLOAD_ON "ON"
#define CONFIG_MQTT_PAYLOAD_OFF "OFF"

#ifdef BOARD_ID
  #define HOSTNAME DEVICE_TYPE "_" STRINGIZE(BOARD_ID)
  #define LED_CONFIG_MQTT_TOPIC_COMMAND "homeassistant/light/" STRINGIZE(BOARD_ID) "/" DEVICE_TYPE
  #define DEVICE_FULL_NAME DEVICE_NAME " " STRINGIZE(BOARD_ID)
#else
  #define HOSTNAME DEVICE_TYPE
  #define LED_CONFIG_MQTT_TOPIC_COMMAND "homeassistant/light/" DEVICE_TYPE
  #define DEVICE_FULL_NAME DEVICE_NAME
#endif

#define LED_CONFIG_MQTT_TOPIC_CONFIG LED_CONFIG_MQTT_TOPIC_COMMAND "/config"
#define LED_CONFIG_MQTT_TOPIC_STATE LED_CONFIG_MQTT_TOPIC_COMMAND "/state"