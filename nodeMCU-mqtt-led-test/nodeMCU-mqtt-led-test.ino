//https://www.hackster.io/Isaac_LL/adafruit-io-nodemcu-or-any-esp8266-4988bc


#define DEBUG 1

#include <ESP8266WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

// WiFi Sign-in information
//#define WLAN_SSID       "jPoole"
//#define WLAN_PASS       "Love4wifi!"
#define WLAN_SSID       "Zoo Guest"
#define WLAN_PASS       "Zookeeper"

// Adafruit IO Sign-in information
#define AIO_SERVER      "jservices.site"
#define AIO_SERVERPORT  1883
#define AIO_USERNAME    "jPoole"
#define AIO_KEY         "1a88698778da49bcbf8c3425bc5fb84f"
#define AIO_FEED        "/feeds/Digital"

WiFiClient client;

const char MQTT_SERVER[] PROGMEM    = AIO_SERVER;
const char MQTT_CLIENTID[] PROGMEM  = AIO_KEY __DATE__ __TIME__;
const char MQTT_USERNAME[] PROGMEM  = AIO_USERNAME;
const char MQTT_PASSWORD[] PROGMEM  = AIO_KEY;
Adafruit_MQTT_Client mqtt(&client, MQTT_SERVER, AIO_SERVERPORT, MQTT_CLIENTID, MQTT_USERNAME, MQTT_PASSWORD);

const char ONOFF_FEED[] PROGMEM = AIO_USERNAME AIO_FEED;

//Adafruit_MQTT_Subscribe OnOff = Adafruit_MQTT_Subscribe(&mqtt, ONOFF_FEED);
Adafruit_MQTT_Subscribe OnOff = Adafruit_MQTT_Subscribe(&mqtt, ONOFF_FEED, MQTT_QOS_1); //Attempts to connect with QOS level 1

//Interrupt Stuff
int interruptPin = 2;

void setup() {
  #ifdef DEBUG
    Serial.begin(9600);
 #endif
  
  pinMode(interruptPin, OUTPUT);
  
  #ifdef DEBUG
    Serial.println(F("Wifi Module Started"));
    Serial.println(); Serial.println();
    Serial.print(F("Connecting to "));
    Serial.println(WLAN_SSID);
  #endif

  // Connect to WiFi access point.
  delay(10);

  WiFi.persistent(false);
  WiFi.mode(WIFI_STA);
  
  WiFi.begin(WLAN_SSID, WLAN_PASS);
      
  #ifdef DEBUG
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(F("."));
    }
    Serial.println();
    Serial.println(F("WiFi connected"));
    Serial.println(F("IP address: "));
    Serial.println(WiFi.localIP());
  #endif
  
  #ifndef DEBUG
    while (WiFi.status() != WL_CONNECTED) {
      WiFi.begin(WLAN_SSID, WLAN_PASS);
      delay(1000);
    }
  #endif

  mqtt.subscribe(&OnOff);
  connect();
  delay(500);
}

void loop() {
  #ifdef DEBUG
    Serial.println(F("Waiting"));
  #endif
  
  Adafruit_MQTT_Subscribe *subscription;

  
  // ping adafruit io a few times to make sure we remain connected
  if(! mqtt.ping(3)) {
    // reconnect to adafruit io
    if(! mqtt.connected())
      connect();
  }
    
  // this is our 'wait for incoming subscription packets' busy subloop
  while (subscription = mqtt.readSubscription(1000)) {

    // we only care about the OnOff events
    if (subscription == &OnOff) {
      
      #ifdef DEBUG
        Serial.println(F("Received"));
        delay(500);
      #endif
      
      digitalWrite(interruptPin, HIGH);
      delay(20);
      digitalWrite(interruptPin, LOW);
      /*
      // convert mqtt ascii payload to int
      char *value = (char *)OnOff.lastread;
      Serial.print(F("Received: "));
      Serial.println(value);
      // Apply message to OnOff
      String message = String(value);
      message.trim();
      if (message == "ON") {
        //digitalWrite(interruptPin, HIGH);
        Serial.println(F("ON"));
        }
      if (message == "OFF") {
        //digitalWrite(interruptPin, LOW);
        Serial.println(F("OFF"));
      }
      */

    }

  }

}


void connect() {
  #ifdef DEBUG
  Serial.print(F("Connecting to Adafruit IO... "));
  #endif
  
  int8_t ret;
  
  while ((ret = mqtt.connect()) != 0) {
    #ifdef DEBUG
    switch (ret) {
      case 1: Serial.println(F("Wrong protocol")); break;
      case 2: Serial.println(F("ID rejected")); break;
      case 3: Serial.println(F("Server unavail")); break;
      case 4: Serial.println(F("Bad user/pass")); break;
      case 5: Serial.println(F("Not authed")); break;
      case 6: Serial.println(F("Failed to subscribe")); break;
      default: Serial.println(F("Connection failed")); break;
    }
    #endif

    if(ret >= 0)
      mqtt.disconnect();

    #ifdef DEBUG
      Serial.println(F("Retrying connection..."));
    #endif
    delay(5000);

  }
  #ifdef DEBUG
  Serial.println(F("Adafruit IO Connected!"));
  #endif
}
