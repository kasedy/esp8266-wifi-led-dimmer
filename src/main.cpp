#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ESP8266mDNS.h>
#include <ArduinoOTA.h>
#include <Tasker.h>
#include <ArduinoJson.h>
#include <ResetDetector.h>
#include <CapacitiveSensor.h>

#include "config.h"
#include "LightState.h"
#include "helpers.h"
#include "effects.h"
#include "LedDriver.h"
#include "average.h"

WiFiClient wifiClient;
PubSubClient mqttClient(MQTT_SERVER, MQTT_PORT, wifiClient);
Tasker tasker;
bool OTAStarted = false;

const int BUFFER_SIZE = JSON_OBJECT_SIZE(20);

LightState lightState(LED_PINS, defaultEffects());
LedDriver ledBuildIn(-1);
LedDriver ledOne(-1);
LedDriver ledTwo(-1);
CapacitiveSensor cs = CapacitiveSensor(5, 4);

void setupWifi(bool resetPassword) 
{
  if (resetPassword) {
    DBG("Resetting WiFi password!\n");
    WiFi.disconnect();
  } else {
    DBG("Autoconnecting!\n");
    WiFi.setAutoConnect(true);
    WiFi.mode(WIFI_STA);
    WiFi.begin();
  }
  #if LOGGING
  WiFi.printDiag(Serial);
  #endif
  if (resetPassword || WiFi.waitForConnectResult() != WL_CONNECTED) {
    DBG("Connection failed. Starting autoconfig.\n");
    WiFi.beginSmartConfig();
    while(true) {
      DBG("Waiting for smart config\n");
      digitalWrite(LED_BUILTIN, LOW);
      delay(500);
      digitalWrite(LED_BUILTIN, HIGH);
      delay(500);
      if (WiFi.waitForConnectResult() == WL_CONNECTED) {
        if (resetPassword && !WiFi.smartConfigDone()) {
          continue;
        }
        WiFi.stopSmartConfig();
        delay(500);
        break;
      }
    }
  }

  DBG("Connected to WiFi!\n");

  for (int i = 0; i < 10; ++i) {
      digitalWrite(LED_BUILTIN, LOW);
      delay(50);
      digitalWrite(LED_BUILTIN, HIGH);
      delay(50);
  }
}

void onOTAStart() {
  DBG("Starting OTA session\n");
  OTAStarted = true;
}

void setupOta() {
  ArduinoOTA.setHostname(HOSTNAME);
  ArduinoOTA.begin();
  ArduinoOTA.onStart(onOTAStart);
}

void mqttDiscovery() {
  if (!mqttClient.connected()) {
    DBG("Mqtt is not connected. Can not send discovery.\n");
    return;
  }

  DynamicJsonDocument doc(1024);
  doc["schema"] = "json";
  doc["unique_id"] = HOSTNAME;
  doc["name"] = DEVICE_FULL_NAME;
  doc["state_topic"] = LED_CONFIG_MQTT_TOPIC_STATE;
  doc["command_topic"] = LED_CONFIG_MQTT_TOPIC_COMMAND;
  doc["brightness"] = true;
  doc["optimistic"] = false;
  doc["qos"] = 2;
  doc["retain"] = true;
  doc["effect"] = lightState.getAvailableAnimationCount() > 0;
  doc["white_value"] = true;

  if (lightState.getAvailableAnimationCount() > 0) {
    JsonArray effectList = doc.createNestedArray("effect_list");
    for (size_t i = 0; i < lightState.getAvailableAnimationCount(); ++i) {
      effectList.add(lightState.getAnimationName(i));
    }
  }
  
  String output;
  serializeJson(doc, output);
  doc.clear();

  DBG("Sending discovery config: %s\n", output.c_str());
  if (!mqttClient.publish(LED_CONFIG_MQTT_TOPIC_CONFIG, output.c_str(), true)) {
    DBG("Failed to send discovery! Length = %d", output.length());
  }
}

void reconnectMqtt() {
  if (!mqttClient.connected()) {
    DBG("Attempting MQTT connection...\n");
    if (mqttClient.connect(HOSTNAME)) {
      DBG("MQTT connected. Subscribing %s\n", LED_CONFIG_MQTT_TOPIC_COMMAND);
      mqttClient.subscribe(LED_CONFIG_MQTT_TOPIC_COMMAND, 1);
      mqttDiscovery();
    } else {
      DBG("MQTT failed rc= %d Try again in 5 seconds\n", mqttClient.state());
    }
  }
}

int convertBrigtnessToOwmDutyCycle(int brightness) {
  return map(brightness, 0, 255, 0, 1023);
}

bool processJson(char* message) {
  StaticJsonDocument<BUFFER_SIZE> doc;
  DeserializationError error = deserializeJson(doc, message);

  if (error) {
    DBG("parseObject() failed\n");
    return false;
  }

  if (doc.containsKey("state")) {
    if (strcmp(doc["state"], CONFIG_MQTT_PAYLOAD_ON) == 0) {
      lightState.setStateOn(true);
    }
    else if (strcmp(doc["state"], CONFIG_MQTT_PAYLOAD_OFF) == 0) {
      lightState.setStateOn(false);
    }
  }

  if (doc.containsKey("brightness")) {
    lightState.setMaxBrightness(doc["brightness"]);
  }

  if (doc.containsKey("effect")) {
    lightState.setEffect(doc["effect"]);
  }

  if (doc.containsKey("white_value")) {
    lightState.setAnimationSpeed(doc["white_value"]);
  }

  return true;
}

void sendState() {
  StaticJsonDocument<BUFFER_SIZE> doc;

  doc["state"] = lightState.isOn() ? CONFIG_MQTT_PAYLOAD_ON : CONFIG_MQTT_PAYLOAD_OFF;
  doc["brightness"] = lightState.getMaxBrightness();
  if (lightState.hasCurrentEffect()) {
    doc["effect"] = lightState.getCurrentEffectName();
  }
  doc["white_value"] = lightState.getAnimationSpeed();

  char buffer[measureJson(doc) + 1];
  serializeJson(doc, buffer, sizeof(buffer));

  const char* topic = LED_CONFIG_MQTT_TOPIC_STATE;
  DBG("Publishing state to %s -> %s\n", topic, buffer);
  mqttClient.publish(topic, buffer, true);
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  DBG("Message arrived [%s]\n", topic);

  char message[length + 1];
  memcpy(message, payload, length);
  message[length] = '\0';
  DBG("%s\n", message);
  
  if (!processJson(message)) {
    return;
  }

  sendState();
}

void setup() { 
  int resetCount = ResetDetector::execute(2000);

#if LOGGING
  Serial.begin(74880, SERIAL_8N1, SERIAL_TX_ONLY);
  Serial.setDebugOutput(true);
#endif 

  DBG("Starting with number of resets = %d\n", resetCount);
  lightState.setup();
  setupWifi(resetCount >= 3);
  setupOta();
  
  mqttClient.setCallback(mqttCallback);
  reconnectMqtt();

  tasker.setInterval(reconnectMqtt, 5000);

  randomSeed(ESP.getCycleCount());
  ledBuildIn.setPattern({500, 1000}, LOW);
  ledOne.blink(300);
  ledTwo.blink(600);
  cs.set_CS_AutocaL_Millis(0xFFFFFFFF);
}

unsigned long lastPrint = 0;
long maxValue = 0;
long minValue = 0;
AverageValueCalculator<uint32_t, uint32_t> touchSensorData;

void processSensorButton() {
  long start = millis();
  long total = cs.capacitiveSensor(30);
  touchSensorData.addMeasurement(total);
  if (touchSensorData.counter >= 50) {
    Serial.print(touchSensorData.minValue);
    Serial.print("\t");
    Serial.print(touchSensorData.getAverage());
    Serial.print("\t");
    Serial.print(touchSensorData.maxValue);
    Serial.println();
    touchSensorData.reset();
  }

  minValue = min(minValue, total);
  maxValue = max(maxValue, total);
  unsigned long now = millis();
  if (now - lastPrint > 200) {
    lightState.setStateOn(!lightState.isOn());
      Serial.print(millis() - start);        // check on performance in milliseconds
      Serial.print("\t");                    // tab character for debug window spacing
      Serial.print(minValue);                  // print sensor output 1
      Serial.print("\t");
      Serial.print(total);                  // print sensor output 1
      Serial.print("\t");
      Serial.println(maxValue);                  // print sensor output 1
      minValue = maxValue = total;
      lastPrint = now;
  }

  if (total < 100) {
    ledTwo.setLow();
  } else if (total > 400) {
    ledTwo.setHigh();
  } else {
    ledTwo.blink(100);    
  }
}

void loop() {
  ArduinoOTA.handle();
  if (OTAStarted) {
    return;
  }
  tasker.loop();
  mqttClient.loop();
  lightState.handle();
  ledBuildIn.loop();
  ledOne.loop();
  ledTwo.loop();
  processSensorButton();
}