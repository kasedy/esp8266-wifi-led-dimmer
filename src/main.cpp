#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ESP8266mDNS.h>
#include <ArduinoOTA.h>
#include <Tasker.h>
#include <ArduinoJson.h>
#include <ResetDetector.h>

#include "config.h"
#include "LightState.h"
#include "helpers.h"
#include "effects.h"

WiFiClient wifiClient;
PubSubClient mqttClient(MQTT_SERVER, MQTT_PORT, wifiClient);
Tasker tasker;
bool OTAStarted = false;

const int BUFFER_SIZE = JSON_OBJECT_SIZE(20);

LightState lightState(LED_PINS, defaultEffects());

void setupWifi(bool resetPassword) 
{
  pinMode(LED_BUILTIN, OUTPUT);
  analogWrite(LED_BUILTIN, 512);
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

  DynamicJsonBuffer jsonBuffer;
  JsonObject& config = jsonBuffer.createObject();
  config["schema"] = "json";
  config["unique_id"] = HOSTNAME;
  config["name"] = DEVICE_FULL_NAME;
  config["state_topic"] = LED_CONFIG_MQTT_TOPIC_STATE;
  config["command_topic"] = LED_CONFIG_MQTT_TOPIC_COMMAND;
  config["brightness"] = true;
  config["optimistic"] = false;
  config["qos"] = 2;
  config["retain"] = true;
  config["effect"] = lightState.getAvailableAnimationCount() > 0;
  config["white_value"] = true;

  if (lightState.getAvailableAnimationCount() > 0) {
    JsonArray& effectList = config.createNestedArray("effect_list");
    for (size_t i = 0; i < lightState.getAvailableAnimationCount(); ++i) {
      effectList.add(lightState.getAnimationName(i));
    }
  }
  
  String output;
  config.printTo(output);
  jsonBuffer.clear();

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
  StaticJsonBuffer<BUFFER_SIZE> jsonBuffer;

  JsonObject& root = jsonBuffer.parseObject(message);

  if (!root.success()) {
    DBG("parseObject() failed\n");
    return false;
  }

  if (root.containsKey("state")) {
    if (strcmp(root["state"], CONFIG_MQTT_PAYLOAD_ON) == 0) {
      lightState.setStateOn(true);
    }
    else if (strcmp(root["state"], CONFIG_MQTT_PAYLOAD_OFF) == 0) {
      lightState.setStateOn(false);
    }
  }

  if (root.containsKey("brightness")) {
    lightState.setMaxBrightness(root["brightness"]);
  }

  if (root.containsKey("effect")) {
    lightState.setEffect(root["effect"]);
  }

  if (root.containsKey("white_value")) {
    lightState.setAnimationSpeed(root["white_value"]);
  }

  return true;
}

void sendState() {
  StaticJsonBuffer<BUFFER_SIZE> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();

  root["state"] = lightState.isOn() ? CONFIG_MQTT_PAYLOAD_ON : CONFIG_MQTT_PAYLOAD_OFF;
  root["brightness"] = lightState.getMaxBrightness();
  if (lightState.hasCurrentEffect()) {
    root["effect"] = lightState.getCurrentEffectName();
  }
  root["white_value"] = lightState.getAnimationSpeed();

  char buffer[root.measureLength() + 1];
  root.printTo(buffer, sizeof(buffer));

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
  Serial.begin(74880);
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
}

void loop() {
  ArduinoOTA.handle();
  if (OTAStarted) {
    return;
  }
  tasker.loop();
  mqttClient.loop();
  lightState.handle();
}