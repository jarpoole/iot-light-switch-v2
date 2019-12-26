

#include <RH_ASK.h>
#include <SPI.h> // Not actually used but needed to compile
#include <avr/io.h>
#include <avr/interrupt.h>

#define TRANSMITTER 5
#define RECEIVER 6

//RH_ASK driver(2000, RECEIVER, TRANSMITTER, -1, false);
RH_ASK driver(500, RECEIVER, TRANSMITTER, -1, false);

bool buttonState;

int main (void){
  Serial.begin(115200);    // Debugging only
  if (!driver.init()){
    Serial.println("init failed");
  }
  DDRD |= (1<<PD4); //Enable PD4 (Pin4) as output
  DDRD &= ~(1<<PD2); //Enable PD2 (Pin2) as input

  EICRA |= (1<<ISC01) | (1<<ISC00); //Configure rising edge trigger (11) for INT0
  EIMSK |= (1<<INT0); //Enable external interrupt 0 (On PD2)
  sei(); //global interrupt enable

  while(true);
}

ISR(INT0_vect){
  PORTD ^= (1<<PD4); //Toggle pin 4 on interrupt
  transmit();
}


void transmit(){
    //Serial.println("0");
    const uint8_t data[1] = {0x12}; //SWITCH_TOGGLE
    driver.send(&data[0], 1);
    //Serial.println("1");
    
    driver.waitPacketSent();
    //Serial.println("2");
    
    delay(200);
    //Serial.println("3");
}
