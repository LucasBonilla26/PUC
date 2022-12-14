#include <Firebase_ESP_Client.h>
#include <WiFi.h>
#include "HTTPClient.h"
#include <Wire.h>
#include <PN532_I2C.h>
#include <PN532.h>
#include <NfcAdapter.h>
#include <DFRobotDFPlayerMini.h>
#include <SoftwareSerial.h>
#include <Time.h>
//Provide the token generation process info.
#include "addons/TokenHelper.h"
//Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"

// Insert your network credentials
//#define WIFI_SSID "realme X3 SuperZoom"
//#define WIFI_PASSWORD "Jotas1234"

// Insert Firebase project API Key
#define API_KEY "AIzaSyCpHZFLFbC_axgeO2lNnbICakesssqNlVc"

// Insert RTDB URLefine the RTDB URL */
#define DATABASE_URL "https://pruebafirebase-5dc4d-default-rtdb.europe-west1.firebasedatabase.app/" 

//Define Firebase Data object
FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

PN532_I2C pn532i2c(Wire);
PN532 nfc(pn532i2c);

SoftwareSerial mySoftwareSerial(26, 25); 
DFRobotDFPlayerMini altavoz;

volatile bool connected = false;


const char* serverName = "http://192.168.1.1/counter";
const char* serverSSID = "http://192.168.1.1/ssid";
const char* serverPass = "http://192.168.1.1/password";
const char* ssid = "ESP32Wifi";
const char* password = "123456789";


bool estadoActual = false;
bool estadoAnterior = false;
bool detectedTarget = false;
int contador = 0;
String get = "";
unsigned long prevMillisData = 0;
unsigned long currentMillisData = 0;
unsigned long sendDataPrevMillis = 0;
bool signupOK = false;
String uidString = "";
int resta = 0;
int result = 0;
String wifiSSID = " ";
String wifiPASS = " ";
String fecha = " ";
struct tm timeinfo;

void init_WiFi() {
  WiFi.begin(ssid, password);
  Serial.println("Connecting");
  while(WiFi.status() != WL_CONNECTED) { 
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());
}

void init_FirebaseWiFi(){
  Serial.begin(115200);
  const char* WIFI_SSID = wifiSSID.c_str();
  const char* WIFI_PASSWORD = wifiPASS.c_str();
  Serial.print(WIFI_SSID);
  Serial.print(WIFI_PASSWORD);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  /* Assign the api key (required) */
  config.api_key = API_KEY;

  /* Assign the RTDB URL (required) */
  config.database_url = DATABASE_URL;

  /* Sign up */
  if (Firebase.signUp(&config, &auth, "", "")){
    Serial.println("ok");
    signupOK = true;
  }
  else{
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }

  /* Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h
  
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
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
  uidString = "";
  connected = false;
  while (!connected) {
    connected = connect();
  }

  // Wait for an ISO14443A type cards (Mifare, etc.).  When one is found
  // 'uid' will be populated with the UID, and uidLength will indicate
  // if the uid is 4 bytes (Mifare Classic) or 7 bytes (Mifare Ultralight)
  success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, &uid[0], &uidLength,60000);

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

void altavozStartup(){
    mySoftwareSerial.begin(9600);
    Serial.begin(115200); 
    Serial.println(F("DFRobot DFPlayer Mini Demo"));
    Serial.println(F("Initializing DFPlayer ... (May take 3~5 seconds)"));
  
    if(!altavoz.begin(mySoftwareSerial)) {  //Use softwareSerial to communicate with mp3.
      Serial.println(F("Unable to begin:"));
      Serial.println(F("1.Please recheck the connection!"));
      Serial.println(F("2.Please insert the SD card!"));
      
      altavoz.reset();     //Reset the module
      
    }
    Serial.println(F("DFPlayer Mini online."));
  
    altavoz.volume(30);
}

void writeFirebase(int total){
  String dBase = uidString + fecha + "/dominadas";
  if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 15000 || sendDataPrevMillis == 0)){
    sendDataPrevMillis = millis();
    // Write an Int number on the database path test/int
    if (Firebase.RTDB.setInt(&fbdo, dBase, total)){
      Serial.println("PASSED");
      Serial.println("PATH: " + fbdo.dataPath());
      Serial.println("TYPE: " + fbdo.dataType());
    }
    else {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }
  }
}

void checkNum(){
      resta = 0;
      get = httpGETRequest(serverName);
      
      result = get.compareTo("0");   
      if(result == 0){
        estadoActual = false;
      }
      else{
             
        estadoActual = true;
      }
      resta = currentMillisData - prevMillisData;
      if((resta) <= 20000){
        currentMillisData = millis();                  
        if (estadoActual == true && estadoActual == estadoAnterior){
          contador = contador + 1;
          estadoAnterior = !estadoAnterior;
          Serial.print(contador);
          Serial.print(" ");    
          prevMillisData = millis();    
          
        }
        if (estadoActual == false){
          estadoAnterior = true;
        }
      }
      else{
        WiFi.disconnect();
        init_FirebaseWiFi();
        initTime("CET-1CEST,M3.5.0,M10.5.0/3");   // Set for Madrid/AU
        printLocalTime();
        writeFirebase(contador);
        
        altavoz.play(33);
        detectedTarget = false;
        WiFi.disconnect();
        init_WiFi();
      }
}

void setTimezone(String timezone){
  Serial.printf("  Setting Timezone to %s\n",timezone.c_str());
  setenv("TZ",timezone.c_str(),1);  //  Now adjust the TZ.  Clock settings are adjusted to show the new local time
  tzset();
}
void initTime(String timezone){

  Serial.println("Setting up time");
  configTime(0, 0, "pool.ntp.org");    // First connect to NTP server, with 0 TZ offset
  if(!getLocalTime(&timeinfo)){
    Serial.println("  Failed to obtain time");
    return;
  }
  Serial.println("  Got the time from NTP");
  // Now we can set the real timezone
  setTimezone(timezone);
}
void printLocalTime(){
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time 1");
    return;
  }
  fecha = "/"+(String)timeinfo.tm_mday+"_"+(String)(timeinfo.tm_mon+1)+"_" +(String)(timeinfo.tm_year-100)+"_" +(String)timeinfo.tm_hour +"_" + (String)timeinfo.tm_min +"_" + (String)timeinfo.tm_sec; 

}

void setup() {
    Serial.begin(115200);
    Serial.print("Initialization begin");
    //select_WiFi();
    Serial.println();
    
    //Inicializaci??n wifi
    init_WiFi();
    Serial.print("Connected to wifi");
    wifiSSID = httpGETRequest(serverSSID);
    wifiPASS = httpGETRequest(serverPass);
    Serial.print(wifiSSID);
    Serial.print(wifiPASS);
    //Inicializaci??n altavoz
    Serial.print("Test 1 Passed");
    altavozStartup();
    delay(300);
    Serial.begin(115200); 
    //Inicializacion del sensor
    Serial.print("Test 2 Passed");
}

void loop() {
    if(WiFi.status() == WL_CONNECTED)
    {
      if (detectedTarget == false){
        NFCdetection();
        detectedTarget = true;
        contador = 0;
        //altavoz.play(32);
        prevMillisData = millis();
      
      }
      else{
        checkNum();
    } 
  }
}
