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
#include "ESPAsyncWebServer.h"
#include "M5StickC.h"

VL53L0X tof_sensor;
TFT_eSprite img = TFT_eSprite(&M5.Lcd);

// Replace with your network credentials
const char* ssid     = "ESP32Wifi";
const char* password = "123456789";

bool active = false;
int counter = 0;

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
    Serial.print("Setting AP (Access Point)…");
    // Remove the password parameter, if you want the AP (Access Point) to be open
    WiFi.softAP(ssid, password);
    IPAddress IP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(IP);
    server.on("/counter", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send_P(200, "text/plain", String(active).c_str());
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
      else if (distance > 500 && active)
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
