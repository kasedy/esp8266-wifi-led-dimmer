#include <WebPortal.h>
#include "config.h"
#include "dbg.h"
#include "LightController.h"

#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>

#ifndef NO_WEB_PORTAL

extern LightController *lightController;

const char *WEBPAGE PROGMEM = "<!DOCTYPE html><html lang=en><head><meta charset=UTF-8 name=viewport content=\"width=device-width, initial-scale=1, user-scalable=no\"/><title>Control Light</title> <style>body{text-align:center;font-family:verdana;user-select:none}input,select{outline:0;font-size:1em;padding:8px;width:100%;box-sizing:border-box}select{margin-top:10px}form>*{display:block;margin-bottom:15px}form{text-align:left;width:80%;display:inline-block;}.c{text-align:center}input[type=\'checkbox\']{zoom:1.8;width:auto;margin:10px;vertical-align:middle}</style></head><body><div style=display:inline-block;min-width:340px><h1 class=c>Countreau Bottle</h1><form><label class=c>Enable<input type=checkbox name=state></label><div>Brightness<input name=brightness type=range min=1 max=255 value=128></div><div>Speed<input name=speed type=range min=1 max=255 value=128></div><div>Effect <select name=effect>{effects}</select></div></form></div> <script>function p(el){return el.type==\'checkbox\'?\'checked\':\'value\';};var ws=new WebSocket(\"ws://\"+location.host+\"/ws\"),form=document.forms[0];form.onchange=()=>{ws.send(JSON.stringify([].reduce.call(form,(data,el)=>{data[el.name]=el[p(el)];return data;},{})));};ws.onmessage=(event)=>{var jsondata=JSON.parse(event.data);[].forEach.call(form,(el)=>{if(jsondata.hasOwnProperty(el.name)){el[p(el)]=jsondata[el.name];}});}</script></body></html>";
const char *STATE_PARAM PROGMEM = "state";
const char *BRIGHTNESS_PARAM PROGMEM = "brightness";
const char *SPEED_PARAM PROGMEM = "speed";
const char *EFFECT_PARAM PROGMEM = "effect";

namespace WebPortal {
  AsyncWebServer *server;
  AsyncWebSocket *ws;

  AsyncWebSocketMessageBuffer *getLightStateJson() {
    StaticJsonDocument<528> jsonBuffer;
    jsonBuffer[FPSTR(STATE_PARAM)] = lightController->isOn();
    jsonBuffer[FPSTR(BRIGHTNESS_PARAM)] = lightController->getLightBrightness();
    jsonBuffer[FPSTR(SPEED_PARAM)] = lightController->getAnimationSpeed();
    jsonBuffer[FPSTR(EFFECT_PARAM)] = lightController->getCurrentAnimationIndex();
    size_t len = measureJson(jsonBuffer); // len without null terminating char
    AsyncWebSocketMessageBuffer *websocketBuffer = ws->makeBuffer(len); //  creates a buffer (len + 1) for you.
    if (websocketBuffer) {
      serializeJson(jsonBuffer, (char *)websocketBuffer->get(), len + 1);
    }
    return websocketBuffer;
  }

  void onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len){
    if (type == WS_EVT_CONNECT) {
      client->text(getLightStateJson());
    } else if (type == WS_EVT_DATA){
      AwsFrameInfo * info = (AwsFrameInfo*)arg;
      String msg = "";
      if(info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT){
        StaticJsonDocument<528> jsonBuffer;
        char buffer[len + 1];
        buffer[len] = '\0';
        memcpy(buffer, data, len);
        DeserializationError error = deserializeJson(jsonBuffer, buffer);
        if (!error) {
          bool state = jsonBuffer[FPSTR(STATE_PARAM)];
          int brightness = jsonBuffer[FPSTR(BRIGHTNESS_PARAM)];
          int speed = jsonBuffer[FPSTR(SPEED_PARAM)];
          int effectIndex = jsonBuffer[FPSTR(EFFECT_PARAM)];
          lightController->setStateOn(state);
          lightController->setLightBrightness(brightness);
          lightController->setAnimationSpeed(speed);
          lightController->setAnimationByIndex(effectIndex);
          DBG("WEB PORTAL! state: %s; brightness: %d; speed: %d; effect: %d\n", state ? "true" : "false", brightness, speed, effectIndex);          
        }
      }
    }
  }

  void setup() {
    server = new AsyncWebServer(80);
    ws = new AsyncWebSocket(F("/ws"));
    server->on("/heap", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send(200, F("text/plain"), String(ESP.getFreeHeap()));
    });
    server->on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
      String effects;
      for (size_t index = 0; index < lightController->getAnimationCount(); ++index) {
        const char *animationName = lightController->getAnimationName(index);
        effects += String(F("<option value=")) + String(index) + '>' + animationName + F("</option>");
      }
      String html(FPSTR(WEBPAGE));
      html.replace(F("{effects}"), effects);
      request->send(200, F("text/html"), html);
    });
    server->onNotFound([](AsyncWebServerRequest *request){
      request->redirect("/");
    });
    ws->onEvent(onWsEvent);
    server->addHandler(ws);
    server->begin();
  }

  void broadcaseLightChanges() {
    ws->textAll(getLightStateJson());
  }
}

#else

namespace WebPortal {
  void setup() {}
  void broadcaseLightChanges() {}
};

#endif