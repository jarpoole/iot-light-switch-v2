//Modified RadioHead library
#include "./radiohead-ask-atmega328p/RadioHead.h"
#include "./radiohead-ask-atmega328p/RH_ASK.h"
#include "./radiohead-ask-atmega328p/RH_ASK.cpp"
#include "./radiohead-ask-atmega328p/RHCRC.h"
#include "./radiohead-ask-atmega328p/RHCRC.cpp"
#include "./radiohead-ask-atmega328p/RHGenericDriver.cpp"
#include "./radiohead-ask-atmega328p/RHGenericDriver.h"

#include "Constants.h"

#include <avr/io.h>
#include <avr/interrupt.h>

#define TRANSMITTER 5
#define RECEIVER 6

RH_ASK ask_driver(500, RECEIVER, TRANSMITTER, -1, false);

bool switchState;
bool switchChanged;

int main(){
  initialize();
  
  while(true){
    while(receiveASKByte() != WAKE_UP);
    Serial.println("wakeup");
    transmitASKByte(ACK);

    transmit(SWITCH_TOGGLE);
    Serial.println("toggle");
    /*
    if(switchChanged){
      if(switchState){
        transmit(SWITCH_ON);
      }else{
        transmit(SWITCH_OFF);
      }
      switchChanged = false;
    }else{
      transmit(NO_CHANGE);
    }
    */
    transmit(SLEEP);
    Serial.println("sleep");
  }
}

void initialize(){
  Serial.begin(38400);    // Debugging only
  if (!ask_driver.init()){
    Serial.println("init failed");
  }
  DDRD |= (1<<PD4);     //Enable PD4 (Pin4) as output
  DDRD &= ~(1<<PD2);    //Enable PD2 (Pin2) as input

  EICRA |= (1<<ISC01) | (1<<ISC00);  //Configure rising edge trigger (11) for INT0
  EIMSK |= (1<<INT0);                //Enable external interrupt 0 (On PD2)
  sei();                             //global interrupt enable

  switchState = false;
  switchChanged = false;
}

ISR(INT0_vect){
  PORTD ^= (1<<PD4); //Toggle pin 4 on interrupt
  switchState = !switchState;
  switchChanged = true;
}

void transmit(byte data){
  do{
    transmitASKByte(data);
  }while(receiveASKByte() != ACK);
}

void transmitASKByte(byte data){
    ask_driver.send(&data, 1);
    ask_driver.waitPacketSent();
}
byte receiveASKByte(){  //blocking
    uint8_t buf[1];
    uint8_t buflen = sizeof(buf);
    Serial.print("Receiving");
      
    while(!ask_driver.recv(buf, &buflen)){    // Non-blocking  
        Serial.print(".");
        delay(100);
    }
    Serial.print("\n");
    return(buf[0]); 
}
