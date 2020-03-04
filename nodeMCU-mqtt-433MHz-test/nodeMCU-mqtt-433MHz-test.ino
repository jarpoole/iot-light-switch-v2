
#include "ESP8266WiFi.h"
#include <MQTT.h>
#include <RH_ASK.h>

#define F_CPU 8000000UL

char ssid[] = "Zoo Guest";
char pass[] = "Zookeeper";
//char ssid[] = "jPoole";
//char pass[] = "Love4wifi!";

//ESP8266 PINS
#define TRANSMITTER 2
#define RECEIVER 0

//RH_ASK driver(2000, RECEIVER, TRANSMITTER, -1, false);
RH_ASK driver(500, RECEIVER, TRANSMITTER, -1, false);

WiFiClient net;
MQTTClient client;

unsigned long lastMillis = 0;

void connect() {
  Serial.print("checking wifi...");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }

  Serial.print("\nconnecting...");
  while (!client.connect("arduino", "try", "try")) {
    Serial.print(".");
    delay(1000);
  }

  Serial.println("\nconnected!");

  client.subscribe("light-switch/state");
  // client.unsubscribe("/hello");
}

void messageReceived(String &topic, String &payload) {
  Serial.println("incoming: " + topic + " - " + payload);
  if(payload == "on"){
    //digitalWrite(15, HIGH);
    transmit();
    Serial.println("Turning on");
  }else if(payload == "off"){
    //digitalWrite(15, LOW);
    transmit();
    Serial.println("Turning off");
  }
}

void setup() {
  pinMode(15, OUTPUT);
  
  Serial.begin(115200);
  Serial.println("Started...");

  if (!driver.init())
    Serial.println("init failed");

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
  
  WiFi.begin(ssid, pass);
  
  // Note: Local domain names (e.g. "Computer.local" on OSX) are not supported by Arduino.
  // You need to set the IP address directly.
  client.begin("jservices.site", net);
  client.onMessage(messageReceived);

  connect();
}

void loop() {
  client.loop();
  delay(10);  // <- fixes some issues with WiFi stability

  if (!client.connected()) {
    connect();
  }

  /*
  // publish a message roughly every second.
  if (millis() - lastMillis > 1000) {
    lastMillis = millis();
    client.publish("/hello", "world");
  }
 */
}

void transmit(){
    //Serial.println("0");
    const uint8_t data[1] = {0x40}; //BEEP 
    //const uint8_t data[1] = {0x12}; //Toggle switch
    driver.send(&data[0], 1);
    //Serial.println("1");
    
    driver.waitPacketSent();
    //Serial.println("2");
    
    //_delay_ms(200);
    delay(200); 
    //Serial.println("3");
}
