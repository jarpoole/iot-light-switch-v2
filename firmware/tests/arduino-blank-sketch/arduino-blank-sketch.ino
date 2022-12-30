#ifdef F_CPU
#undef F_CPU
#define F_CPU 8000000UL
#endif


void setup() {

    /*
    DDRD |= (1<<PD7);   //Output
    PORTD &= ~(1<<PD7); //Sink current to turn on led
    */

    DDRD &= ~(1<<PD7);  //Input
    

}

void loop() {
  // put your main code here, to run repeatedly:

}
