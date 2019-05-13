#include "MqttProcessor.h"
#include "config.h"
#include "dbg.h"
#include "LightState.h"

#include <ESP8266WiFi.h>
#include <Ticker.h>
#include <AsyncMqttClient.h>
#include <ArduinoJson.h>

extern LightState lightState;

namespace MqttProcessor {
  AsyncMqttClient mqttClient;
  Ticker mqttReconnectTimer;

  void connectToMqtt() {
    if (!WiFi.isConnected()) {
      Serial.println("Trying to connect MQTT but WiFi is not connected!");
      mqttReconnectTimer.once(10, connectToMqtt);
    } else if (!mqttClient.connected()) {
      Serial.println("Connecting to MQTT...");
      mqttClient.connect();
    }
  }

  void onMqttConnect(bool sessionPresent) {
    Serial.printf("Connected to MQTT. Session present: %d\n", sessionPresent);
    mqttClient.subscribe(LED_CONFIG_MQTT_TOPIC_COMMAND, 1);

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
    mqttClient.publish(LED_CONFIG_MQTT_TOPIC_CONFIG, 2, true, output.c_str(), output.length());
  }

  void onMqttDisconnect(AsyncMqttClientDisconnectReason reason) {
    Serial.printf("Disconnected from MQTT. Rery in 10 sec. Error code %d\n", reason);
    mqttReconnectTimer.once(10, connectToMqtt);
  }

  void broadcastStateViaMqtt() {
    if (!mqttClient.connected()) {
      Serial.printf("Unable to publish the state to MQTT. Client is not connected.\n");
      return;
    }

    StaticJsonDocument<JSON_OBJECT_SIZE(20)> doc;

    doc["state"] = lightState.isOn() ? CONFIG_MQTT_PAYLOAD_ON : CONFIG_MQTT_PAYLOAD_OFF;
    doc["brightness"] = lightState.getMaxBrightness();
    if (lightState.hasCurrentEffect()) {
      doc["effect"] = lightState.getCurrentEffectName();
    }
    doc["white_value"] = lightState.getAnimationSpeed();

    size_t jsonSize = measureJson(doc);
    char buffer[jsonSize + 1];
    serializeJson(doc, buffer, sizeof(buffer));

    const char* topic = LED_CONFIG_MQTT_TOPIC_STATE;
    DBG("Publishing state to %s -> %s\n", topic, buffer);
    mqttClient.publish(topic, 2, true, buffer, jsonSize);
  }

  void onMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t length, size_t index, size_t total) {
    DBG("Message arrived [%s]\n", topic);

    char message[length + 1];
    memcpy(message, payload, length);
    message[length] = '\0';
    DBG("%s\n", message);
    
    StaticJsonDocument<JSON_OBJECT_SIZE(20)> doc;
    DeserializationError error = deserializeJson(doc, message);

    if (error) {
      DBG("parseObject() failed\n");
      return;
    }

    if (doc.containsKey("state")) {
      const char *state = doc["state"];
      Serial.printf("State = %s\n", state);
      if (strcmp(doc["state"], CONFIG_MQTT_PAYLOAD_ON) == 0) {
        Serial.print("Switching ON\n");
        lightState.setStateOn(true);
      }
      else if (strcmp(doc["state"], CONFIG_MQTT_PAYLOAD_OFF) == 0) {
        Serial.print("Switching OFF\n");
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

    broadcastStateViaMqtt();
  }

  void setup() {
    mqttClient
      .onConnect(onMqttConnect)
      .onDisconnect(onMqttDisconnect)
      .onMessage(onMqttMessage)
      .setClientId(HOSTNAME)
      .setServer(MQTT_SERVER, MQTT_PORT);
    connectToMqtt();
  }
};
