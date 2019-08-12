#include "VoiceControl.h"
#include "dbg.h"
#include "LightController.h"

#include <ESP8266WiFi.h>
#include <Ticker.h>
#include <AsyncMqttClient.h>

#define HOST "io.adafruit.com"
#define PORT 8883
#define TOPIC "voice_control"
#define AIO_SSL_FINGERPRINT {0x77, 0x00, 0x54, 0x2D, 0xDA, 0xE7, 0xD8, 0x03, 0x27, 0x31, 0x23, 0x99, 0xEB, 0x27, 0xDB, 0xCB, 0xA5, 0x4C, 0x57, 0x18}

#ifndef VOICE_CONTROL_MQTT_USER
  #define VOICE_CONTROL_MQTT_USER ""
#endif

#ifndef VOICE_CONTROL_MQTT_PASSWORD
  #define VOICE_CONTROL_MQTT_PASSWORD ""
#endif

extern LightController *lightController;

namespace VoiceControl {

  using namespace std::placeholders;

  class VoiceControlImpl {
    AsyncMqttClient mqttClient;
    Ticker mqttReconnectTimer;
    const char* mqttUser;
    const char* mqttPassword;
    bool invalidPassword;

  public:
    VoiceControlImpl(const char* mqttUser, const char* mqttPassword);

  private:
    void connectToMqtt();
    void onMqttConnect(bool sessionPresent);
    void onMqttDisconnect(AsyncMqttClientDisconnectReason reason);
    void onMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t length, size_t index, size_t total);
  };


  VoiceControlImpl::VoiceControlImpl(const char* mqttUser, const char* mqttPassword) :
      mqttUser(mqttUser),
      invalidPassword(false) {
    mqttClient
      .setServer(HOST, PORT)
      .setCredentials(mqttUser, mqttPassword)
      .setSecure(true)
      .addServerFingerprint((const uint8_t[])AIO_SSL_FINGERPRINT)
      .onConnect(std::bind(&VoiceControlImpl::onMqttConnect, this, _1))
      .onDisconnect(std::bind(&VoiceControlImpl::onMqttDisconnect, this, _1))
      .onMessage(std::bind(&VoiceControlImpl::onMqttMessage, this, _1, _2, _3, _4, _5, _6));
    connectToMqtt();
  }

  void VoiceControlImpl::connectToMqtt() {
    if (!WiFi.isConnected()) {
      DBG("Trying to connect MQTT but WiFi is not connected!\n");
      mqttReconnectTimer.once(10, std::bind(&VoiceControlImpl::connectToMqtt, this));
    } else if (!mqttClient.connected()) {
      DBG("Connecting to MQTT...\n");
      mqttClient.connect();
    }  
  }

  void VoiceControlImpl::onMqttConnect(bool sessionPresent) {
    DBG("Connected to MQTT. Session present: %d\n", sessionPresent);
    String topicName = String(mqttUser) + F("/feeds/") + TOPIC;
    DBG("Subscribing: %s\n", topicName.c_str());
    mqttClient.subscribe(topicName.c_str(), 1);
  }

  void VoiceControlImpl::onMqttDisconnect(AsyncMqttClientDisconnectReason reason) {
    DBG("Disconnected from MQTT. Rery in 10 sec. Error code %d\n", reason);
    if (reason == AsyncMqttClientDisconnectReason::MQTT_NOT_AUTHORIZED) {
      invalidPassword = true;
    } else if (reason == AsyncMqttClientDisconnectReason::TCP_DISCONNECTED) {
      if (invalidPassword) {
        delete this; // invalid login and password... no reason to live... making suicide
        return;
      } else {
        mqttReconnectTimer.once(10, std::bind(&VoiceControlImpl::connectToMqtt, this));
      }
    }
  }

  void VoiceControlImpl::onMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t length, size_t index, size_t total) {
    DBG("Message arrived [%s]\n", topic);
    String message;
    message.reserve(length);
    memcpy(message.begin(), payload, length);
    DBG("%s\n", message.c_str());
    int delimeterIndex = message.indexOf(' ');
    String command = delimeterIndex == -1 ? message : message.substring(0, delimeterIndex);
    String argument = delimeterIndex == -1 ? String() : message.substring(delimeterIndex);
    if (command == F("on")) {
      lightController->setStateOn(true);
    } else if (command == F("off")) {
      lightController->setStateOn(false);
    }
  }

  void setup() {
    const char *mqttUser = VOICE_CONTROL_MQTT_USER;
    const char *mqttPassord = VOICE_CONTROL_MQTT_PASSWORD;
    if (strlen(mqttUser) > 0 && strlen(mqttPassord) > 0) {
      new VoiceControlImpl(mqttUser, mqttPassord);
    }
  }
}