#include <Servo.h>
#include "Constants.h"

#define BUTTON 3         //PD3, INT1
#define BUZZER 13        //PB5
#define TRANSMITTER 5    //PD5
#define RECEIVER 6       //PD6
#define VAR_RESISTOR A0  //PC0

//Serial Constants
#define SERIAL_ENABLE 4  //PD4
#define SERIAL_ENABLE_TIME 5000
#define SERIAL_BAUD 115200

//BMS Constants
#define BMS_ADDRESS 0x6A
#define BMS_INTERRUPT 2  //PD2, INT0
#define BMS_OTG A3       //PC3
#define BMS_CE A2        //PC2
#define BMS_QON A1       //PC1

//Servo Constants
#define SERVO 9          //PB1
#define SERVO_ENABLE 8   //PB0
#define ONPOSITION 75 
#define OFFPOSITION 115
#define OVERSHOOT 30

Servo servo;
volatile long lastButtonPressedTime = 0;
volatile bool lastButtonState = false;

volatile bool switchOn = false;


//ISR Toggles
volatile bool requestSwitchToggle = false;
volatile bool requestSwitchOn = false;
volatile bool requestSwitchOff = false;

volatile bool requestSerialOn = false;
volatile bool requestSerialOff = false;


void setup() {
  pinMode(BUZZER, OUTPUT);
  pinMode(TRANSMITTER, OUTPUT);
  
  pinMode(SERIAL_ENABLE, OUTPUT);
  digitalWrite(SERIAL_ENABLE, LOW);
  
  pinMode(SERVO_ENABLE, OUTPUT);
  digitalWrite(SERVO_ENABLE, LOW);
  
  pinMode(RECEIVER, INPUT);
  pinMode(BUTTON, INPUT);
  pinMode(BMS_INTERRUPT,INPUT);

  attachInterrupt(digitalPinToInterrupt(BUTTON), buttonISR, CHANGE);
  attachInterrupt(BMS_INTERRUPT, bmsISR, RISING);

  servo.attach(SERVO);
}

void loop() {

  processUpdates();
//toggleSwitch();
  
  Serial.println("Test");
  delay(10);
}

void processUpdates(){
  if(requestSwitchToggle){
    toggleSwitch();
    requestSwitchToggle = false;
  }
  if(requestSwitchOn){
    turnOn();
    requestSwitchOn = false;
  }
  if(requestSwitchOff){
    turnOff();
    requestSwitchOff = false;
  }
  if(requestSerialOn){
    enableSerial();
    requestSerialOn = false;
  }
}

void buttonISR(){
  bool buttonState = digitalRead(BUTTON);

  //digitalWrite(SERVO_ENABLE, !digitalRead(SERVO_ENABLE)); // Toggle the LED for testing

  if(buttonState == true && lastButtonState == false){  //RISING
    lastButtonPressedTime = millis();
  }
  if(buttonState == false && lastButtonState == true){  //FALLING
    if ((unsigned long)(millis() - lastButtonPressedTime) >= SERIAL_ENABLE_TIME){
        requestSerialOn = true;
    }else{
      requestSwitchToggle = true;
    }
  }
  lastButtonState = buttonState;
}

void bmsISR(){
  
}



void toggleSwitch(){
  if(switchOn){
    turnOff();
  }else{
    turnOn();
  }
}

void turnOn(){
  digitalWrite(SERVO_ENABLE, HIGH);
  servo.writeMicroseconds(ONPOSITION - OVERSHOOT);
  delay(100);
  servo.writeMicroseconds(ONPOSITION);
  delay(1000);
  digitalWrite(SERVO_ENABLE, LOW);

  switchOn = true;
}

void turnOff(){
  digitalWrite(SERVO_ENABLE, HIGH);
  servo.writeMicroseconds(OFFPOSITION + OVERSHOOT);
  delay(100);
  servo.writeMicroseconds(OFFPOSITION);
  delay(1000);
  digitalWrite(SERVO_ENABLE, LOW);

  switchOn = false;
}

void enableSerial(){
  digitalWrite(SERIAL_ENABLE, HIGH);
  Serial.begin(SERIAL_BAUD);
  Serial.println("Serial enabled");
}
