/*
*******************************************************************************
* Copyright (c) 2021 by M5Stack
*                  Equipped with M5StickC sample source code
*                          配套  M5StickC 示例源代码
* Visit for more information: https://docs.m5stack.com/en/core/m5stickc
* 获取更多资料请访问: https://docs.m5stack.com/zh_CN/core/m5stickc
*
* Product:  ToF.  激光测距
* Date: 2022/6/3
*******************************************************************************
  Please connect to hat,Use ToF Hat to detect distance and display distance
  data on the screen in real time.
  请连接HAT,使用ToF HAT检测距离，并在屏幕上实时显示距离数据。

  Please install vl53l0x lib first(https://github.com/pololu/vl53l0x-arduino)
  lib in Sketch->Includ Library->Library Manager, search for vl53l0x
  Author:pololu
*/

#include <VL53L0X.h>
#include <Wire.h>
#include <WiFi.h>
#include <HTTPClient.h>
//#include <AsyncTCP.h>
//#include <Arduino.h>
#include "ESPAsyncWebServer.h"
#include "M5StickC.h"

VL53L0X tof_sensor;
TFT_eSprite img = TFT_eSprite(&M5.Lcd);

// Replace with your network credentials
const char* ssid     = "ESP32Wifi";
const char* password = "123456789";

String inputMessage = "";
String inputParam = "";

bool active = false;
int counter = 0;

const char* PARAM_INPUT_1 = "input1";
const char* PARAM_INPUT_2 = "input2";

// HTML web page to handle 3 input fields (input1, input2, input3)
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html><head>
  <title>ESP Input Form</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  </head><body>
  <form action="/get">
    input1: <input type="text" name="input1">
    <input type="submit" value="Submit">
  </form><br>
  <form action="/get">
    input2: <input type="text" name="input2">
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
      Serial.println("Input Param:");
      Serial.println(inputParam);

    }
    
    // GET input2 value on <ESP_IP>/get?input2=<inputMessage>
    
    else if (request->hasParam(PARAM_INPUT_2)) {
      inputMessage = request->getParam(PARAM_INPUT_2)->value();
      inputParam = PARAM_INPUT_2;
    }
    /*
    else {
      inputMessage = "No message sent";
      inputParam = "none";
    }*/
    
    Serial.println(inputMessage);
    request->send_P(200, "text/html", index_html);    });

    /*
    request->send(200, "text/html", "HTTP GET request sent to your ESP on input field (" 
                                     + inputParam + ") with value: " + inputMessage +
                                     "<br><a href=\"/\">Return to Home Page</a>");
    */
    server.on("/ssid", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send_P(200, "text/html", inputMessage.c_str());
    });
    
    server.on("/contraseña", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send_P(200, "text/html", inputMessage.c_str());
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
    //Serial.println(counterPU(distance));
    img.fillSprite(BLACK);
    img.setCursor(10, 10);
    img.print(distance);
    img.pushSprite(0, 0);

}
