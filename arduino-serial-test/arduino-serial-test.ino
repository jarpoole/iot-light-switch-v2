#define F_CPU 8000000UL
#define BAUD 9600

#include <avr/io.h>
#include <util/delay.h>

#define F_CPU 16000000  // 16 MHz oscillator.
#define BaudRate 9600
#define CALC_UBRR (F_CPU / 16 / BaudRate ) - 1

void delayLong()
{
  unsigned int delayvar;
  delayvar = 0;
  while (delayvar <=  65500U)
  {
    asm("nop");
    delayvar++;
  }
}


unsigned char serialCheckRxComplete(void){
  return( UCSR0A & _BV(RXC0)) ;   // nonzero if serial data is available to read.
}

unsigned char serialCheckTxReady(void){
  return( UCSR0A & _BV(UDRE0) ) ;   // nonzero if transmit register is ready to receive new data.
}

unsigned char serialRead(void){
  while (serialCheckRxComplete() == 0);    // While data is NOT available to read
  return UDR0;
}

void serialWrite(unsigned char DataOut){
  while (serialCheckTxReady() == 0);   // while NOT ready to transmit
  UDR0 = DataOut;
}

void serialBegin(){
  /*Set baud rate */
  UBRR0H = (unsigned char)(CALC_UBRR>>8);
  UBRR0L = (unsigned char) CALC_UBRR;
  /* Enable receiver and transmitter   */
  UCSR0B = (1<<RXEN0)|(1<<TXEN0);
  /* Frame format: 8data, No parity, 1stop bit */
  UCSR0C = (3<<UCSZ00);
}


int main (void){
    serialBegin();
    
    while(1){
        serialWrite('A');
        _delay_ms(1000);
    }
 
}
