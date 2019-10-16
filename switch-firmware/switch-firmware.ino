#include <PWMServo.h>
#include <RH_ASK.h>
#include <SPI.h> // Not actually used but needed to compile RH_ASK

#include "Constants.h"
#include "BoardV1.h"


//From documentation: RH_ASK (uint16_t speed=2000, uint8_t rxPin=11, uint8_t txPin=12, uint8_t pttPin=10, bool pttInverted=false)
//pttPin = -1 will disable push to talk functionality
RH_ASK driver(2000, RECEIVER, TRANSMITTER, -1, false);

PWMServo servo;

volatile long lastButtonPressedTime = 0;
volatile bool lastButtonState = false;

volatile bool switchOn = false;
volatile bool debugOn = false;


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
  //digitalWrite(SERIAL_ENABLE, LOW);
  digitalWrite(SERIAL_ENABLE, HIGH);   //*********************FIX ME **********************
  
  pinMode(SERVO_ENABLE, OUTPUT);
  digitalWrite(SERVO_ENABLE, LOW);
  
  pinMode(RECEIVER, INPUT);
  pinMode(BUTTON, INPUT);
  pinMode(BMS_INTERRUPT,INPUT);

  attachInterrupt(digitalPinToInterrupt(BUTTON), buttonISR, CHANGE);
  attachInterrupt(BMS_INTERRUPT, bmsISR, RISING);

  servo.attach(SERVO);

  bool ask_init = driver.init();
  if(debugOn && !ask_init){
    Serial.println("RH_ASK init failed");
  }
  
}

void loop() {

  processASKCommands(checkASKByte()); //Check for wireless commands
  //processISRUpdates();                //Check for changes generated by the ISR
  
  
  //Serial.println("Test");
  delay(10);
}

void processASKCommands(byte command){
    if(command == NULL){
        return;
    }else if(command == BEEP){
      digitalWrite(BUZZER, HIGH);
      delay(10);
      digitalWrite(BUZZER, LOW);
    }
}

void processISRUpdates(){
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
  servo.write(ONPOSITION - OVERSHOOT);
  delay(100);
  servo.write(ONPOSITION);
  delay(1000);
  digitalWrite(SERVO_ENABLE, LOW);

  switchOn = true;
}

void turnOff(){
  digitalWrite(SERVO_ENABLE, HIGH);
  servo.write(OFFPOSITION + OVERSHOOT);
  delay(100);
  servo.write(OFFPOSITION);
  delay(1000);
  digitalWrite(SERVO_ENABLE, LOW);

  switchOn = false;
}

void enableSerial(){
  digitalWrite(SERIAL_ENABLE, HIGH);
  Serial.begin(SERIAL_BAUD);
  Serial.println("Serial enabled");
  debugOn = true;
}

byte checkASKByte(){
    uint8_t buf[1];
    uint8_t buflen = sizeof(buf);
    if(driver.recv(buf, &buflen)){
        return(buf[0]); 
    }else{
        return NULL;
    }
}


//RH_ASK wrapper functions
byte receiveASKByte(){         //blocking
    uint8_t buf[1];
    uint8_t buflen = sizeof(buf);
    while(!driver.recv(buf, &buflen)){    // Non-blocking  
      if(debugOn){
        Serial.print(".");
      }
      delay(100);
    }
    if(debugOn){
      Serial.print("\n");
    }
    return(buf[0]); 
}

void transmitByte(byte data){
    driver.send(&data, 1);
    driver.waitPacketSent();
    delay(1000);
}
