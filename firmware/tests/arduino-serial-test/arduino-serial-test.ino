
#include <avr/io.h>
#include <util/delay.h>
#include <avr/pgmspace.h>

#define F_CPU 8000000UL  // 8 MHz oscillator.
#define BaudRate 9600
#define CALC_UBRR (F_CPU / 8 / BaudRate ) - 1

unsigned char serialCheckRxComplete(void){
  return( UCSR0A & _BV(RXC0)) ;   // nonzero if serial data is available to read.
}

unsigned char serialCheckTxReady(void){
  return( UCSR0A & _BV(UDRE0) ) ;   // nonzero if transmit register is ready to receive new data.
}

unsigned char serialReadChar(void){
  while (serialCheckRxComplete() == 0);    // While data is NOT available to read
  return UDR0;
}

void serialWriteChar(unsigned char DataOut){
  while (serialCheckTxReady() == 0);   // while NOT ready to transmit
  UDR0 = DataOut;
}

void serialWriteString(const char* string){
  const char* ptr = string;
  while(*ptr != 0){
    serialWriteChar(*ptr);
    ptr++;
  }
}

void serialWriteProgString(const char* ptr){
  char c;
  if (!ptr) 
    return;
  while ((c = pgm_read_byte(ptr++)))
    serialWriteChar(c);
}

void serialBegin(){
  /*Set baud rate */
  UBRR0H = (unsigned char)(CALC_UBRR>>8);
  UBRR0L = (unsigned char) CALC_UBRR;
  /* Enable receiver and transmitter   */
  UCSR0B = (1<<RXEN0)|(1<<TXEN0);
  /* Frame format: 8data, No parity, 1stop bit */
  UCSR0C = (3<<UCSZ00);


  //Jared Added this
  UCSR0A |= _BV(U2X0); //Enable USE_2X
}


int main (void){
    serialBegin();

    
    //const static char out[] PROGMEM = "A";
    
    while(1){
        //serialWriteChar("Hello, my name is Jared  and I like the color blue\n");
        serialWriteProgString(out);
        _delay_ms(1000);
    }
 
}
