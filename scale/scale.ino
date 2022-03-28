#include<Arduino.h>
#include<HX711.h>

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ArduinoOTA.h>
#include <ESP8266mDNS.h>
#include <WebSocketsServer.h>
#include <ESPAsyncWebServer.h>

#define DT_PIN 5
#define SCK_PIN 4

HX711 scale;

const double calibration_factor = 21500.0; // 21500 for S/N 18082920837, 20750 for S/N 18080902727


ESP8266WiFiMulti wifiMulti;       // Create an instance of the ESP8266WiFiMulti class, called 'wifiMulti'

AsyncWebServer webServer(8080);

WebSocketsServer webSocket(80);    // create a websocket server on port 80

const char *ssid = "inuvaha"; // The name of the Wi-Fi network that will be created
const char *password = "vahahadla";   // The password required to connect to it, leave blank for an open network

const char *OTAName = "Inuits-Vaha";           // A name and a password for the OTA service
const char *OTAPassword = "otasazmota";

const char* mdnsName = "inuits-vaha";

volatile bool wsClient = false;
volatile uint8_t wsClientNum = 0;

double prevWeight = 0;



void startWiFi(){
  WiFi.softAP(ssid, password);
  Serial.print("AP \"");
  Serial.print(ssid);
  Serial.print("\" started with password \"");
  Serial.print(password);
  Serial.println("\"");
  wifiMulti.addAP("inuits", "{wifi_pw}");
  Serial.print("Connecting");
  while(wifiMulti.run() != WL_CONNECTED && WiFi.softAPgetStationNum() <1){
    delay(250);
    Serial.print(".");
  }
  Serial.print("\r\n");
  if(WiFi.softAPgetStationNum() == 0){
    Serial.print("Connected to: ");
    Serial.println(WiFi.SSID());
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  }
  else{
    Serial.print("Station successfuly connected");
    Serial.print("\r\n");
  }
}
unsigned long mils = 0;

void startOTA() { // Start the OTA service
  ArduinoOTA.setHostname(OTAName);
  ArduinoOTA.setPassword(OTAPassword);

  ArduinoOTA.onStart([]() {
    Serial.println("Start");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\r\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();
  Serial.println("OTA ready\r\n");
}

void startWebSocket() { // Start a WebSocket server
  webSocket.begin();                          // start the websocket server
  webSocket.onEvent(webSocketEvent);          // if there's an incomming websocket message, go to function 'webSocketEvent'
  Serial.println("WebSocket server started.");
}

void startMDNS() { // Start the mDNS responder
  MDNS.begin(mdnsName);                        // start the multicast domain name server
  MDNS.addService("scale", "tcp", 80);
  Serial.print("mDNS responder started: ws://");
  Serial.print(mdnsName);
  Serial.println(".local");
}

void startScale(){
  scale.begin(DT_PIN, SCK_PIN);
  scale.set_scale(calibration_factor); // see hx711_calibration sketch
  scale.tare();
}

void startWeb(){
  webServer.on("/amIScale", HTTP_GET, [](AsyncWebServerRequest *request){
    AsyncWebServerResponse *response = request->beginResponse(200); //Sends 418 Just for fun
    response->addHeader("Access-Control-Allow-Origin", "*");
    request->send(response);
  });
  webServer.begin();
  Serial.println("WebServer started on port 8080");
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length){
  switch(type){
    case WStype_DISCONNECTED:
      Serial.printf("WS [%u] Disconnected!\r\n", num);
      wsClient = false;
      wsClientNum = 0;
      break;
    case WStype_CONNECTED:
      /*IPAddress ip = webSocket.remoteIP(num);
      Serial.printf("WS [%u] Connected from %d.%d.%d.%d url: %s\r\n", num, ip[0], ip[1], ip[2], ip[3], payload);*/
      wsClientNum = num;
      wsClient = true;
      webSocket.sendTXT(num, String(scale.get_units()).c_str());
      break;
    case WStype_BIN:
      /*if (length != sizeof(ProtokolPacket)){
        Serial.printf("WS [%u] Paket je moc kratky\n", num);
        break;
      }*/
      handleRX();
      break;
    case WStype_TEXT:
      Serial.printf("WS [%u] got Text: %s\r \n", num, payload);
      if(String((char *)payload).equals("__ping__")){
        Serial.printf("WS [%u] got __ping__ sending __pong__\r \n", num);
        webSocket.sendTXT(num, "__pong__");
      }
      break;
    default:
      Serial.printf("Unsupported WS message: %d\r\n", (int)type);
      break;
  }
}

void handleRX(){
  Serial.println("Got packet");
}

void setup() {
  Serial.begin(115200);        // Start the Serial communication to send messages to the computer
  delay(10);
  Serial.println("\r\n");

  startWiFi();                 // Start a Wi-Fi access point, and try to connect to some given access points. Then wait for either an AP or STA connection
  
  startOTA();                  // Start the OTA service

  startWebSocket();            // Start a WebSocket server
  
  startMDNS();                 // Start the mDNS responder

  startScale();

  startWeb();

}

void loop() {
  webSocket.loop();
  ArduinoOTA.handle();
  
  if(millis() - mils > 500){
    double weight = scale.get_units();
    if(abs(prevWeight - weight) > 0.5){
    if(wsClient){
      webSocket.sendTXT(wsClientNum, String(scale.get_units()).c_str());
      Serial.print("Scale: ");
      Serial.print( scale.get_units(), 1);
      Serial.print(" kg\r\n");
    }
        prevWeight = weight;
    }
    mils = millis();
  }
}
