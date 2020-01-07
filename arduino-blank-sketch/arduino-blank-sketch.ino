#ifdef F_CPU
#undef F_CPU
#define F_CPU 8000000UL
#endif


void setup() {
    DDRD |= (1<<PD7);

    PORTD &= ~(1<<PD7);
    

}

void loop() {
  // put your main code here, to run repeatedly:

}
