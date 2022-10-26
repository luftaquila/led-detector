#include <WiFi.h>
#include <WiFiMulti.h>
#include <WebSocketsClient.h>
#include <SocketIOclient.h>
#include <ArduinoJson.h>

#define DEBUG true

#define CdS_COUNT 6

// device id and CdS cell count MUST be defined here
const char suburl[] = "/socket.io/?EIO=4&deviceId=0&CdS_count=6";

WiFiMulti WiFiMulti;
SocketIOclient socketIO;

// AP and HOST configurations
const char ssid[] = "A-FA ECU";
const char pwd[] = "55555555";
const char server[] = "test.luftaquila.io";
const int port = 80;

// sensor readings and thresholds
int CdS[CdS_COUNT] = { 0, };
int CdS_GPIO[CdS_COUNT] = { 32, 33, 34, 35, 36, 39};
int threshold[CdS_COUNT] = { 0, };

// socketIO event handler
void socketIOEvent(socketIOmessageType_t type, uint8_t* payload, size_t length) {
  switch(type) {
    case sIOtype_EVENT: {
      DynamicJsonDocument content(512);
      DeserializationError contentError = deserializeJson(content, payload, length);
      if(contentError) return;
      String event = content[0];

      // socket.on REQUEST_VALUE
      if (event == String("REQUEST_VALUE")) {
        // update CdS values
        for(int i = 0; i < CdS_COUNT; i++) {
          CdS[i] = analogRead(CdS_GPIO[i]);
        }

        // socket.emit ANSWER_VALUE
        DynamicJsonDocument content(256);
        JsonArray array = content.to<JsonArray>();
        array.add("ANSWER_VALUE");
        JsonObject param = array.createNestedObject();

        for(int i = 0; i < CdS_COUNT; i++) {
          param["CdS"][i] = CdS[i];
        }
        
        String output;
        serializeJson(content, output);
        socketIO.sendEVENT(output);

        #if DEBUG
          Serial.println("REQUEST_VALUE");
          Serial.println(output);
        #endif
      }

      // socket.on REQUEST_STATUS
      else if (event == String("REQUEST_STATUS")) {
        // update CdS values
        for(int i = 0; i < CdS_COUNT; i++) {
          CdS[i] = analogRead(CdS_GPIO[i]);

          #if DEBUG
            Serial.print("CdS ");
            Serial.print(i);
            Serial.print(": GPIO ");
            Serial.print(CdS_GPIO[i]);
            Serial.print(": ");
            Serial.println(CdS[i]);
          #endif
        }

        // socket.emit ANSWER_STATUS
        DynamicJsonDocument content(256);
        JsonArray array = content.to<JsonArray>();
        array.add("ANSWER_STATUS");
        JsonObject param = array.createNestedObject();

        for(int i = 0; i < CdS_COUNT; i++) {
          param["CdS"][i] = CdS[i] > threshold[i] ? true : false;
        }
        
        String output;
        serializeJson(content, output);
        socketIO.sendEVENT(output);

        #if DEBUG
          Serial.println("REQUEST_STATUS");
          Serial.println(output);
        #endif
      }

      // socket.on UPDATE_THRESHOLD
      else if (event == String("UPDATE_THRESHOLD")) {
        for(int i = 0; i < CdS_COUNT; i++) {
          String data = content[1]["threshold"][i];
          threshold[i] = data.toInt();
        }

        // socket.emit OK_UPDATE_THRESHOLD
        DynamicJsonDocument content(256);
        JsonArray array = content.to<JsonArray>();
        array.add("OK_UPDATE_THRESHOLD");
        JsonObject param = array.createNestedObject();

        for(int i = 0; i < CdS_COUNT; i++) {
          param["threshold"][i] = threshold[i];
        }
        
        String output;
        serializeJson(content, output);
        socketIO.sendEVENT(output);

        #if DEBUG
          Serial.println("UPDATE_THRESHOLD");
          Serial.println(output);
        #endif
      }
      break;
    }

    case sIOtype_DISCONNECT:
      if (WiFiMulti.run() != WL_CONNECTED) WiFi.reconnect();

      #if DEBUG
        Serial.println("sIOtype_DISCONNECT");
      #endif

      break;

    case sIOtype_CONNECT: {
      // join default socket namespace
      socketIO.send(sIOtype_CONNECT, "/");

      #if DEBUG
        Serial.println("sIOtype_CONNECT");
      #endif

      break;
    }
  }
}

void setup() {
  #if DEBUG
    Serial.begin(115200);
    Serial.println("DEVICE STARTUP");
  #endif

  // set ADC resolution to 12bit
  analogReadResolution(12);

  // AP init
  if(WiFi.getMode() & WIFI_AP) WiFi.softAPdisconnect(true);
  WiFiMulti.addAP(ssid, pwd);
  while(WiFiMulti.run() != WL_CONNECTED) delay(100);
  WiFi.setAutoReconnect(true);
  WiFi.persistent(true);

  // socket attach
  socketIO.onEvent(socketIOEvent);
  socketIO.begin(server, port, suburl);
}

void loop() {
  socketIO.loop();
}

