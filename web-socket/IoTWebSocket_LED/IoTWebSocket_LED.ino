#include <Arduino.h>

#include <WiFi.h>
#include <WiFiMulti.h>
#include <WiFiClientSecure.h>

#include <ArduinoJson.h>

#include <WebSocketsClient.h>
#include <SocketIOclient.h>

WiFiMulti WiFiMulti;
SocketIOclient socketIO;

#define USE_SERIAL Serial
#define TYPEIOT "sensor"
#define CATEGORYIOT "light"
#define ssid      "tamer"       // le nom (SSID) de votre réseau WiFi
#define password  "123oui123"  // votre mot de passe WiFi
String hostname = "ESP32 Sensor v0.1";
String initialised = "false";
String version = "0.1";
String currentStatus;

int value = 0;

void socketIOEvent(socketIOmessageType_t type, uint8_t * payload, size_t length) {
    switch(type) {
        case sIOtype_DISCONNECT:
            USE_SERIAL.printf("[IOc] Disconnected!\n");
            break;
        case sIOtype_CONNECT:
            USE_SERIAL.printf("[IOc] Connected to url: %s\n", payload);
            
            // join default namespace (no auto join in Socket.IO V3)
            socketIO.send(sIOtype_CONNECT, "/");
            init();
            initialised = "true";
            
            break;
        case sIOtype_EVENT:
        {
          USE_SERIAL.printf("Payload: ");
          USE_SERIAL.println((char *)payload);
            getMessage(payload, length);
        }
            break;
        case sIOtype_ACK:
            USE_SERIAL.printf("[IOc] get ack: %u\n", length);
            break;
        case sIOtype_ERROR:
            USE_SERIAL.printf("[IOc] get error: %u\n", length);
            break;
        case sIOtype_BINARY_EVENT:
            USE_SERIAL.printf("[IOc] get binary: %u\n", length);
            break;
        case sIOtype_BINARY_ACK:
            USE_SERIAL.printf("[IOc] get binary ack: %u\n", length);
            break;
    }
}

void setup() {
  pinMode(23, OUTPUT);
    //USE_SERIAL.begin(921600);
    USE_SERIAL.begin(115200);

    //Serial.setDebugOutput(true);
    USE_SERIAL.setDebugOutput(true);

    USE_SERIAL.println();
    USE_SERIAL.println();
    USE_SERIAL.println();

      for(uint8_t t = 4; t > 0; t--) {
          USE_SERIAL.printf("[SETUP] BOOT WAIT %d...\n", t);
          USE_SERIAL.flush();
          delay(1000);
      }

    WiFiMulti.addAP(ssid, password);

    //WiFi.disconnect();
    while(WiFiMulti.run() != WL_CONNECTED) {
        delay(100);
    }
    
    WiFi.setHostname(hostname.c_str());
    String ip = WiFi.localIP().toString();
    USE_SERIAL.printf("[SETUP] WiFi Connected %s\n", ip.c_str());

    // server address, port and URL
    socketIO.begin(WiFi.dnsIP().toString(), 8080, "/socket.io/?EIO=4");

    // event handler
    socketIO.onEvent(socketIOEvent);
}

void init(){
  
  // creat JSON message for Socket.IO (event)
  DynamicJsonDocument doc(1024);
  JsonArray array = doc.to<JsonArray>();

  // add evnet name
  // Hint: socket.on('message', ....
  array.add("connect-iot");

  // add payload (parameters) for the event
  JsonObject param1 = array.createNestedObject();
  param1["mac"] = WiFi.macAddress();
  param1["type"] = TYPEIOT;
  param1["category"] = CATEGORYIOT;
  param1["name"] = hostname;
  param1["version"] = version;
  // JSON to String (serializion)
  String output;
  serializeJson(doc, output);

  // Send event
  socketIO.sendEVENT(output);

  // Print JSON for debugging
  USE_SERIAL.println(output);
  initialised == "true";
}

void sendMessage(String content){

    // creat JSON message for Socket.IO (event)
    DynamicJsonDocument doc(1024);
    JsonArray array = doc.to<JsonArray>();
  
    // add evnet name
    // Hint: socket.on('message', ....
    array.add("send-data-iot");
  
    // add payload (parameters) for the event
    JsonObject param1 = array.createNestedObject();
    param1["value"] = content;
    param1["category"] = CATEGORYIOT;
  
    // JSON to String (serializion)
    String output;
    serializeJson(doc, output);
  
    // Send event
    socketIO.sendEVENT(output);
  
    // Print JSON for debugging
    USE_SERIAL.println(output);
}

void getMessage(uint8_t * payload, size_t length){
  char * sptr = NULL;
  int id = strtol((char *)payload, &sptr, 10);
  USE_SERIAL.printf("[IOc] get event: %s id: %d\n", payload, id);
  if(id) {
      payload = (uint8_t *)sptr;
  }
  DynamicJsonDocument doc(1024);
  DeserializationError error = deserializeJson(doc, payload, length);
  if(error) {
      USE_SERIAL.print(F("deserializeJson() failed: "));
      USE_SERIAL.println(error.c_str());
      return;
  }

  String eventName = doc[0];
  String test = doc[1];
  USE_SERIAL.printf("[IOc] event name: %s\n", eventName.c_str());
  USE_SERIAL.printf(eventName.c_str());
  if (eventName == "change-status-device"){
    
    currentStatus = test;
    USE_SERIAL.printf("\n currentStatus: ");
    USE_SERIAL.println(currentStatus);
    
  }
  // Message Includes a ID for a ACK (callback)
  if(id) {
      // creat JSON message for Socket.IO (ack)
      DynamicJsonDocument docOut(1024);
      JsonArray array = docOut.to<JsonArray>();

      // add payload (parameters) for the ack (callback function)
      JsonObject param1 = array.createNestedObject();
      param1["now"] = millis();

      // JSON to String (serializion)
      String output;
      output += id;
      serializeJson(docOut, output);

      // Send event
      socketIO.send(sIOtype_ACK, output);
  }
}

unsigned long messageTimestamp = 0;
void loop() {
    
    socketIO.loop();
    if(initialised == "true"){
      sendMessage("ON");
      USE_SERIAL.println("LED ON OU OFF?");
      if (currentStatus == "ON")  /* <- récupérer l'équivalent de status en websocket) */
        {
          USE_SERIAL.println("LED ON");
        digitalWrite(23, HIGH);
        }
      else
        {
          digitalWrite(23, LOW);
        USE_SERIAL.println("LED OFF");
        }
      
     
    }
    else {
    }
    
    /*  A INSERER AU BON ENDROIT */
    
    uint64_t now = millis();
    
    delay(500);
}
