#include "WiFi.h"
#include "HTTPClient.h"
//#include <detaBaseArduinoESP32.h>
//#include <WiFiClientSecure.h>
#include <Wire.h>
#include <PN532_I2C.h>
#include <PN532.h>
#include <NfcAdapter.h>
#include <DFRobotDFPlayerMini.h>
#include <SoftwareSerial.h>


PN532_I2C pn532i2c(Wire);
PN532 nfc(pn532i2c);

SoftwareSerial mySoftwareSerial(25, 26); 
DFRobotDFPlayerMini altavoz;

volatile bool connected = false;


const char* serverName = "http://192.168.4.1/counter";
const char* ssid = "ESP32Wifi";
const char* password = "123456789";

const char* apiKey = "a06djzma_BsxY46hV59bvYfHQdHXYxi314YyJADJS";
const char* detaID = "a06djzma";
const char* detaBaseName = "Test";

//WiFiClientSecure client;
//DetaBaseObject detaObj(client, detaID, detaBaseName, apiKey, true);

bool estadoActual = false;
bool estadoAnterior = false;
bool detectedTarget = false;
int contador = 0;
String get = "";

void init_WiFi() {
  WiFi.begin(ssid, password);
  Serial.println("Connecting");
  while(WiFi.waitForConnectResult() != WL_CONNECTED) { 
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());
}

void wifi_scanner()
{
    Serial.println("scan start");
    
    // WiFi.scanNetworks will return the number of networks found
    int n = WiFi.scanNetworks();
    Serial.println("scan done");
    if (n == 0) {
        Serial.println("no networks found");
    } else {
      Serial.print(n);
      Serial.println(" networks found");
      for (int i = 0; i < n; ++i) {
        // Print SSID and RSSI for each network found
        Serial.print(i + 1);
        Serial.print(": ");
        Serial.print(WiFi.SSID(i));
        Serial.print(" (");
        Serial.print(WiFi.RSSI(i));
        Serial.print(")");
        Serial.println((WiFi.encryptionType(i) == WIFI_AUTH_OPEN)?" ":"*");
        delay(10);
      }
    }
    Serial.println("");
    
    // Wait a bit before scanning again
    delay(5000);
}

//ServverName = URL
String httpGETRequest(const char* serverName) {
  WiFiClient client;
  HTTPClient http;
    
  // Your Domain name with URL path or IP address with path
  http.begin(client, serverName);
  
  // Send HTTP POST request
  int httpResponseCode = http.GET();
  
  String payload = "--"; 
  
  if (httpResponseCode>0) {
    //Serial.print("HTTP Response code: ");
    //Serial.println(httpResponseCode);
    payload = http.getString();
  }
  else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }
  // Free resources
  http.end();

  return payload;  
}

bool connect() {
  
  nfc.begin();

  // Connected, show version
  uint32_t versiondata = nfc.getFirmwareVersion();
  if (!versiondata)
  {
    Serial.println("PN53x card not found!");
    return false;
  }

  //port
  Serial.print("Found chip PN5"); Serial.println((versiondata >> 24) & 0xFF, HEX);
  Serial.print("Firmware version: "); Serial.print((versiondata >> 16) & 0xFF, DEC);
  Serial.print('.'); Serial.println((versiondata >> 8) & 0xFF, DEC);

  // Set the max number of retry attempts to read from a card
  // This prevents us from waiting forever for a card, which is
  // the default behaviour of the PN532.
  nfc.setPassiveActivationRetries(0xFF);

  // configure board to read RFID tags
  nfc.SAMConfig();

  Serial.println("Waiting for card (ISO14443A Mifare)...");
  Serial.println("");

  return true;
}

void NFCdetection()
{
  boolean success;
  // Buffer to store the UID
  uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };
  // UID size (4 or 7 bytes depending on card type)
  uint8_t uidLength;
  String uidString = "";
  while (!connected) {
    connected = connect();
  }

  // Wait for an ISO14443A type cards (Mifare, etc.).  When one is found
  // 'uid' will be populated with the UID, and uidLength will indicate
  // if the uid is 4 bytes (Mifare Classic) or 7 bytes (Mifare Ultralight)
  success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, &uid[0], &uidLength);

  // If the card is detected, print the UID
  if (success)
  {
    Serial.println("Card Detected");
    Serial.print("Size of UID: "); Serial.print(uidLength, DEC);
    Serial.println(" bytes");
    Serial.print("UID: ");
    for (uint8_t i = 0; i < uidLength; i++)
    {
      Serial.print(" 0x"); Serial.print(uid[i], HEX);
      uidString += String(uid[i]);
    }
    Serial.println("");
    Serial.println("");
    
    delay(1000);
  }
}
/*
void reconnect_wifi(){
    unsigned long currentMillis = millis();
    // if WiFi is down, try reconnecting
    if ((WiFi.status() != WL_CONNECTED) && (currentMillis - previousMillis >=interval)) {
      Serial.print(millis());
      Serial.println("Reconnecting to WiFi...");
      WiFi.disconnect();
      WiFi.reconnect();
      previousMillis = currentMillis;
    }
}
*/


void altavozStartup(){
    mySoftwareSerial.begin(9600);
    Serial.begin(115200); 
    Serial.println(F("DFRobot DFPlayer Mini Demo"));
    Serial.println(F("Initializing DFPlayer ... (May take 3~5 seconds)"));
  
    if (!altavoz.begin(mySoftwareSerial)) {  //Use softwareSerial to communicate with mp3.
      Serial.println(F("Unable to begin:"));
      Serial.println(F("1.Please recheck the connection!"));
      Serial.println(F("2.Please insert the SD card!"));
      altavoz.reset();     //Reset the module
      
    }
    Serial.println(F("DFPlayer Mini online."));
  
    altavoz.volume(30);
}

void setup() {
    Serial.begin(115200);
    Serial.print("Initialization begin");
    //altavozStartup();

    Serial.println();
    //Inicialización wifi
    init_WiFi();
    Serial.print("Conencted to wifi");
    //Inicialización altavoz
    Serial.print("Test 1 Passed");
    //Inicializacion del sensor
    Serial.print("Test 2 Passed");
}

void loop() {

    if(WiFi.status() == WL_CONNECTED)
    {
      if (detectedTarget == false){
        NFCdetection();
        detectedTarget = true;
      }
      else{
      get = httpGETRequest(serverName);
      //Serial.print(typeid(get).name());
      //get = bool(get);
      int result = get.compareTo("0");
      if(result == 0){
        estadoActual = false;
      }
      else{
        estadoActual = true;
      }
      if (estadoActual == true && estadoActual == estadoAnterior){
        contador = contador + 1;
        estadoAnterior = !estadoAnterior;
        Serial.print(contador);
        //altavoz.play(1);

        //printResult(detaObj.putObject("{\"item\":{\"key\":\"Nombre\",\"Dominadas\":4}}"));
      }
      if (estadoActual == false){
        estadoAnterior = true;
      }
    }
    }
 
    //Muestra en pantalla el valor de la distancia obtenida por el sensor

    //wifi_scanner();
}
