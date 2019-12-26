#include <avr/io.h>
#include <avr/interrupt.h>

int main(void){
  EICRA |= (1<<ISC01) | (1<<ISC00); //Configure rising edge trigger (11) for INT0
  EIMSK |= (1<<INT0); //Enable external interrupt 0
  sei(); //global interrupt enable

  DDRD |= (1<<PD2) | (1<<PD4);
  
  while(1);
}


ISR(INT0_vect){
  PORTD ^= (1<<PD4); //Toggle pin 4 on interrupt
}
