
#include <IotWebConf.h>
#include <FS.h>
#include <MQTT.h>
#include <RH_ASK.h>
#include "constants.h"

//#define F_CPU 8000000UL

//RH_ASK module PINS
#define TRANSMITTER 2
#define RECEIVER 0
#define UserID "jaredpoole"

//RH_ASK driver(2000, RECEIVER, TRANSMITTER, -1, false);
RH_ASK ask_driver(500, RECEIVER, TRANSMITTER, -1, false);

volatile long transmitTimer = 0;

// Initial name of the Thing. Used e.g. as SSID of the own Access Point.
const char thingName[] = "jSwitch";

// Initial password to connect to the Thing, when it creates an own Access Point.
const char wifiInitialApPassword[] = "password";

#define STRING_LEN 128
#define NUMBER_LEN 32

// When CONFIG_PIN is pulled to ground on startup, initial password is used to create the AP
#define CONFIG_PIN 3 //GPIO3 is also RX pin
// -- Status indicator pin. First it will light up (kept LOW), on Wifi connection it will blink, when connected to the Wifi it will turn off (kept HIGH).
#define STATUS_PIN 1 //GPIO1 is also TX pin

// Configuration specific key. The value should be modified if config structure was changed.
#define CONFIG_VERSION "v1.7"

// Callback method declarations.
void configSaved();
boolean formValidator();

DNSServer dnsServer;
WebServer server(80);
WiFiClient net;
MQTTClient client;

char stringParamValue[STRING_LEN];
char intParamValue[NUMBER_LEN];

IotWebConf *iotWebConf; //Can't construct until after we've switched the pins to GPIO
IotWebConfSeparator *separator1;
IotWebConfParameter *userIDParam;
IotWebConfSeparator *separator2;
IotWebConfParameter *pollingRateParam;

bool requestSwitchOn = false;
bool requestSwitchOff = false;
bool requestSwitchToggle = false;
bool requestEnableSerial = false;
bool requestDisableSerial = false;

bool requestBeep = false;
byte numBeeps = 0;
bool requestNewPollRate = false;
byte pollRate = 0;

void setup()
{ 
  pinMode(CONFIG_PIN, FUNCTION_3); //Call Function0 to restore back to orginal state
  pinMode(STATUS_PIN, FUNCTION_3);

  iotWebConf = new IotWebConf(thingName, &dnsServer, &server, wifiInitialApPassword, CONFIG_VERSION);
  separator1 = new IotWebConfSeparator("User Settings");
  userIDParam = new IotWebConfParameter("User ID", "stringParam", stringParamValue, STRING_LEN, "text", "e.g. jaredpoole");
  separator2 = new IotWebConfSeparator("Device Settings");
  pollingRateParam = new IotWebConfParameter("Polling rate", "intParam", intParamValue, NUMBER_LEN, "number", "0-8sec", NULL, "min='0' max='8' step='1'");

  
  //Serial.begin(115200);
  Serial.println();
  Serial.println("Booting up...");

  if(!SPIFFS.begin()){
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }
  iotWebConf->setStatusPin(STATUS_PIN);
  iotWebConf->setConfigPin(CONFIG_PIN);
  
  iotWebConf->addParameter(separator1);
  iotWebConf->addParameter(userIDParam);
  iotWebConf->addParameter(separator2);
  iotWebConf->addParameter(pollingRateParam);
  iotWebConf->setConfigSavedCallback(&configSaved);
  iotWebConf->setFormValidator(&formValidator);
  iotWebConf->getApTimeoutParameter()->visible = true;
  
  iotWebConf->init();

  server.on("/", handleRoot);
  server.on("/config", []{ iotWebConf->handleConfig(); });
  server.onNotFound([](){ handleFileRead(); });
  //server.onNotFound([](){ iotWebConf->handleNotFound(); });

  //RH_ASK
  if (!ask_driver.init())
    Serial.println("RH_ASK init failed");

  //MQTT
  // Note: Local domain names (e.g. "Computer.local" on OSX) are not supported by Arduino.
  // You need to set the IP address directly.
  client.begin("jservices.site", net);
  client.onMessage(messageReceived);
  //connect();
  
  Serial.println("Ready.");
}

void loop()
{
  //RH_ASK
  processRF();
  
  //WebServer
  iotWebConf->doLoop();

  //MQTT
  if (WiFi.status() == WL_CONNECTED){
    if (!client.connected()) { 
      client.connect(UserID); 
      client.subscribe("light-switch/state");
    }
    client.loop();
    //delay(10);  //fixes some issues with WiFi stability
  }
}


// Handle web requests to "/" path.
void handleRoot(){ 
  if (iotWebConf->handleCaptivePortal()){  // Let IotWebConf test and handle captive portal requests.   
    return;                               //Captive portal request were already served.
  }
  File file = SPIFFS.open("/root.html", "r");
  server.streamFile(file, "text/html");
  file.close();
}

String getContentType(String filename){
  if(filename.endsWith(".htm"))       return "text/html";
  else if(filename.endsWith(".html")) return "text/html";
  else if(filename.endsWith(".css"))  return "text/css";
  else if(filename.endsWith(".js"))   return "application/javascript";
  else if(filename.endsWith(".png"))  return "image/png";
  else if(filename.endsWith(".gif"))  return "image/gif";
  else if(filename.endsWith(".jpg"))  return "image/jpeg";
  else if(filename.endsWith(".ico"))  return "image/x-icon";
  else if(filename.endsWith(".xml"))  return "text/xml";
  else if(filename.endsWith(".pdf"))  return "application/x-pdf";
  else if(filename.endsWith(".zip"))  return "application/x-zip";
  else if(filename.endsWith(".gz"))   return "application/x-gzip";
  return "text/plain";
}

//Serve the right file to the client (if it exists)
bool handleFileRead() { 
  if (iotWebConf->handleCaptivePortal()){
    return true;
  }
  String path = server.uri();
  Serial.println("handleFileRead: " + path);
  String contentType;
  if (path.endsWith("/") || !SPIFFS.exists(path) ){   // If a folder is requested or SPIFFS can't find the file, send the 404 file
    path = "/404.html";     
    contentType = "text/html";
    Serial.println("\tFile Not Found");
  }else{
    contentType = getContentType(path);               // Get the MIME type
  }
  File file = SPIFFS.open(path, "r");                 // Open it
  size_t sent = server.streamFile(file, contentType); // And send it to the client
  file.close();                                       // Then close the file again
  return true;
}

void configSaved(){
  Serial.println("Configuration was updated.");
}

boolean formValidator(){
  Serial.println("Validating form.");
  boolean valid = true;

  int l = server.arg(userIDParam->getId()).length();
  if (l < 1){
    userIDParam->errorMessage = "User ID cannot be empty";
    valid = false;
  }
  return valid;
}
void messageReceived(String &topic, String &payload) {
  Serial.println("incoming: " + topic + " - " + payload);
  if(payload == "on"){
    //digitalWrite(15, HIGH);
    requestSwitchOn = true;
    Serial.println("Turning on");
  }else if(payload == "off"){
    //digitalWrite(15, LOW);
    requestSwitchOff = true;
    Serial.println("Turning off");
  }
}


void transmitASKByte(byte data){
    ask_driver.send(&data, 1);
    ask_driver.waitPacketSent();   
}
void transmitASKBytes(byte* data, int len){
    ask_driver.send(data, len);
    ask_driver.waitPacketSent();   
}
byte receiveASKByte(){
    uint8_t buf[1] = {0};
    uint8_t buflen = sizeof(buf);
    long timeout = millis() + TIMEOUT;
    while(!ask_driver.recv(buf, &buflen) && millis() < timeout);
    //while(!ask_driver.recv(buf, &buflen) );
    return(buf[0]); 
}

void processRF(){

  if(transmitTimer < millis() ){
    transmitTimer = millis() + RETRANSMIT_TIME;  //Reset timer
    
    //Recieve WAKE_UP
    //Send [SWITCH_ON/SWITCH_OFF/SWITCH_TOGGLE, SET_POLL_RATE, ... , SLEEP]
    //Resend above until Recieve ACK
  
    byte response[6] = {0,0,0,0,0,0};
    int len = 0;
    if(requestSwitchOn){
      response[len] = SWITCH_ON;
      len++;
    }
    if(requestSwitchOff){
      response[len] = SWITCH_OFF;
      len++;
    }
    if(requestSwitchToggle){
      response[len] = SWITCH_TOGGLE;
      len++;
    }
    if(requestBeep){
      response[len] = BEEP;
      len++;
      response[len] = numBeeps;
      len++;
    }
    if(requestNewPollRate){
      response[len] = SET_POLL_RATE;
      len++;
      response[len] = pollRate;
      len++;
    }
    response[len] = SLEEP;
  
    //byte response[6] = {0XFF,0x02,0x03,0x04,0x05,0x06};
    transmitASKBytes(response, 6);
  
    if(receiveASKByte() == ACK){ //blocking with timeout
      requestSwitchOn = false;
      requestSwitchOff = false;
      requestSwitchToggle = false;
      requestBeep = false;
      requestNewPollRate = false;
    }
    
    //delay(100);
  }
}
