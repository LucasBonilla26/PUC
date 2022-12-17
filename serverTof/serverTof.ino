#include <VL53L0X.h>
#include <Wire.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include "ESPAsyncWebServer.h"
#include "M5StickC.h"

VL53L0X tof_sensor;
TFT_eSprite img = TFT_eSprite(&M5.Lcd);

// Replace with your network credentials
const char* ssid     = "ESP32Wifi";
const char* password = "123456789";

bool active = false;
int counter = 0;

const char* PARAM_INPUT_1 = "input1";
const char* PARAM_INPUT_2 = "input2";

String inputMessage = "";
String inputParam = "";
String inputMessage2 = "";
String inputParam2 = "";

// HTML web page to handle 3 input fields (input1, input2, input3)
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html><head>
  <title>ESP Input Form</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  </head><body>
  <form action="/get">
    ssid: <input type="text" name="input1">
    <input type="submit" value="Submit">
  </form><br>
  <form action="/get">
    password: <input type="text" name="input2">
    <input type="submit" value="Submit">
  </form><br>
</body></html>)rawliteral";


AsyncWebServer server(80);

void tof_initialitation()
{
    tof_sensor.setTimeout(500);
    if (!tof_sensor.init()) {
        img.setCursor(10, 10);
        img.print("Failed");
        img.pushSprite(0, 0);
        Serial.println("Failed to detect and initialize sensor!");
        while (1) {
        }
    }
    // Start continuous back-to-back mode (take readings as
    // fast as possible).  To use continuous timed mode
    // instead, provide a desired inter-measurement period in
    // ms (e.g. sensor.startContinuous(100)).
    tof_sensor.startContinuous();
}

void startWebServer()
{
    Serial.begin(115200);
    Serial.println("Configuring access point...");
  
    WiFi.mode(WIFI_AP);
    WiFi.softAP(ssid, password);
    Serial.println("Wait 100 ms for AP_START...");
    delay(100);
    
    Serial.println("Set softAPConfig");
    IPAddress IP(192, 168, 1, 1);
    IPAddress NMask(255, 255, 255, 0);
    WiFi.softAPConfig(IP, IP, NMask);
    
    IPAddress myIP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(myIP);

    Serial.print("Setting AP (Access Point)…");
    // Remove the password parameter, if you want the AP (Access Point) to be open
    Serial.print("AP IP address: ");
    Serial.println(IP);

    server.on("/counter", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send_P(200, "text/plain", String(active).c_str());
    });
      // Send web page with input fields to client
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html);
  });

  // Send a GET request to <ESP_IP>/get?input1=<inputMessage>
  server.on("/get", HTTP_GET, [] (AsyncWebServerRequest *request) {
    
    // GET input1 value on <ESP_IP>/get?input1=<inputMessage>
    if (request->hasParam(PARAM_INPUT_1)) {
      inputMessage = request->getParam(PARAM_INPUT_1)->value();
      inputParam = PARAM_INPUT_1;
    }
    // GET input2 value on <ESP_IP>/get?input2=<inputMessage>
    else if (request->hasParam(PARAM_INPUT_2)) {
      inputMessage2 = request->getParam(PARAM_INPUT_2)->value();
      inputParam2 = PARAM_INPUT_2;
    }
    request->send(200, "text/html", index_html);
  });
  server.on("/ssid", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", inputMessage.c_str());
  });
  server.on("/password", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", inputMessage2.c_str());
  });
    server.begin();
}

bool counterPU(uint16_t distance)
{
      if(distance <= 400 && !active)
      {
        counter += 1;
        active = true;
      }
      else if (distance > 400 && active)
      {
        active = false;
      }

      return active;
}

void setup() {
    Serial.begin(115200);
    Serial.print("Initialization begin");
    Wire.begin(0, 26, 100000UL);  //(0,26, 100000UL) for I2C of HAT connection
    M5.begin();
    
    // Creación del sprite(PANTALLA)
    img.createSprite(160, 80);
    img.fillSprite(BLACK);
    img.setTextColor(WHITE);
    img.setTextSize(2);

    Serial.print("Test 1 Passed");
    
    //Inicializacion del sensor
    tof_initialitation();
    //WebServer Initialization
    startWebServer();
}

void loop() {
    //Serial.print(distance);
    //Muestra en pantalla el valor de la distancia obtenida por el sensor
    uint16_t distance = tof_sensor.readRangeContinuousMillimeters();
    Serial.println(counterPU(distance));
    img.fillSprite(BLACK);
    img.setCursor(10, 10);
    img.print(distance);
    img.pushSprite(0, 0);

}
