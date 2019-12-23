#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

//Prepare LED pin
void PortInit(void)
{
    //Initlally LED OFF
    PORTD&=~(1<<PD0);
    //PD0 as output
    DDRD|=(1<<PD0);
}
//Timer2 init acording datasheet
void RTCInit(void)
{
    //Disable timer2 interrupts
    TIMSK2  = 0;
    //Enable asynchronous mode
    ASSR  = (1<<AS2);
    //set initial counter value
    TCNT2=0;
    //set prescaller 128
    TCCR2B |= (1<<CS22)|(1<<CS00);
    //wait for registers update
    while (ASSR & ((1<<TCN2UB)|(1<<TCR2BUB)));
    //clear interrupt flags
    TIFR2  = (1<<TOV2);
    //enable TOV2 interrupt
    TIMSK2  = (1<<TOIE2);
}
//Overflow ISR
ISR(TIMER2_OVF_vect)
{
    //Toggle pin PD0 every second
    PIND=(1<<PD0);
    asm volatile("nop"::);
    //_delay_us(10);
}
int main (void)
{

    MCUSR &= ~(1<<WDRF); //Always clear the watchdog system reset flag on startup, p45
    WDTCSR &= ~(1<<WDE); //Always clear the watchdog system reset mode on startup, p45
  
    pinMode(0, OUTPUT);
    digitalWrite(0, HIGH);
    delay(500);
    digitalWrite(0, LOW);
  
    SMCR|=(1<<SM1)|(1<<SM0); //Set sleep mode control register to PowerSave Mode (Bits SM2 SM1 SM0 = 011 , p38)
    //Initialize port
    PortInit();
    //Initialise the timer2
    RTCInit();
    //Enable global interrupts
    sei();
    while(1)
    {
        SMCR|=(1<<SE); //Set sleep mode control register Sleep Enable, SE, Bit = 1, p38
        asm volatile("sleep"::);
        SMCR&=~(1<<SE);
        //Do nothing
         
    }
}
