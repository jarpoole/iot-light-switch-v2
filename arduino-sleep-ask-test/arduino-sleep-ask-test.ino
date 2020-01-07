#include "./radiohead-ask-atmega328p/RadioHead.h"
#include "./radiohead-ask-atmega328p/RH_ASK.h"
#include "./radiohead-ask-atmega328p/RH_ASK.cpp"
#include "./radiohead-ask-atmega328p/RHCRC.h"
#include "./radiohead-ask-atmega328p/RHCRC.cpp"
#include "./radiohead-ask-atmega328p/RHGenericDriver.cpp"
#include "./radiohead-ask-atmega328p/RHGenericDriver.h"

#define TRANSMITTER 5
#define RECEIVER 6

#ifdef F_CPU
#undef F_CPU
#define F_CPU 8000000UL
#endif

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/sleep.h>
#include <avr/wdt.h>

//From documentation: RH_ASK (uint16_t speed=2000, uint8_t rxPin=11, uint8_t txPin=12, uint8_t pttPin=10, bool pttInverted=false)
//pttPin = -1 will disable push to talk functionality
RH_ASK driver(500, RECEIVER, TRANSMITTER, -1, false);

void setup()
{
    MCUSR &= ~(1<<WDRF); //Always clear the watchdog system reset flag on startup, p45
    WDTCSR &= ~(1<<WDE); //Always clear the watchdog system reset mode on startup, p45

    SMCR|=(1<<SM1)|(1<<SM0); //Set sleep mode control register to PowerSave Mode (Bits SM2 SM1 SM0 = 011 , p38)
    //Initialize port
    PortInit();
    
    //Initialise the timer2
    RTCInit();
    
    //Enable global interrupts
    sei();
}

void loop()
{
    wdt_reset();

    if(!driver.init()){
       PORTD |= (1<<PD0);
    }
    //_delay_ms(500);
    transmitByte(0x40);
    //_delay_ms(500);
    
    /*
    for(int i = 0; i < 3; i++){
       PORTD |= (1<<PD0);
       _delay_ms(250);
       PORTD &= ~(1<<PD0);
       _delay_ms(250);
     }
     */

    
    //Sleep
    RTCInit();
    PORTD &= ~(1<<PD1); //Turn off awake led
    TCNT2=0; //RESET timer 2
    SMCR|=(1<<SE); //Set "Sleep Mode Control Register" Sleep Enable, SE, Bit = 1, p38
    sleep_cpu();   //Issue sleep instruction. This cannot be done by setting a register
    //asm volatile("sleep"::); //Assembly version of sleep_cpu();


    //Sleeping until interrupt

        
    SMCR &= ~(1<<SE); //Disable sleep enable after wakeup
    
}

byte receiveByte() //blocking
{
    uint8_t buf[1];
    uint8_t buflen = sizeof(buf);
    while(!driver.recv(buf, &buflen)){    // Non-blocking  
      Serial.print(".");
      delay(100);
    }
    Serial.print("\n");
    return(buf[0]); 
}

void transmitByte(byte data){
    driver.send(&data, 1);
    driver.waitPacketSent();
    delay(1000);
}


//Prepare LED pin
void PortInit(void){
    //Initially LEDs OFF
    PORTD &= ~((1<<PD0) | (1<<PD1));
    //PD0 as output
    DDRD |= (1<<PD0);
    //PD1 as output
    DDRD |= (1<<PD1);
}
//Timer2 init acording datasheet
void RTCInit(void)
{
    //Disable timer2 interrupts by zeroing OCIE2A, OCIE2B and TOIE2
    TIMSK2  = 0;
    //Enable asynchronous mode, Only other R/W bit in ASSR is EXCLK which should be 0
    ASSR  = (1<<AS2);
    //set initial counter value
    TCNT2=0;
    //set OCR2B to 128, This value should set the frequency of the interrupts
    OCR2B = 0b10000000;
    //set prescaller 1024
    TCCR2B |= (1<<CS22) | (1<<CS21) | (1<<CS20);
    //wait for registers TCN2UB, OCR2BUB and TCR2BUB to update by checking status is ASSR
    while ( (1<<TCN2UB)&ASSR | (1<<TCR2BUB)&ASSR | (1<<TCR2BUB)&ASSR);
    //clear interrupt flags;  "Output Compare Flag 2 B" and "Timer/Counter2 Overflow Flag"
    TIFR2  = (1<<TOV2) | (1<<OCF2B);
    //enable OCIE2B interrupt to trigger when compare B matches
    TIMSK2  = (1<<OCIE2B);
}
//Timer 2 compare B ISR
ISR(TIMER2_COMPB_vect)
{
    PORTD |= (1<<PD1);
    asm volatile("nop"::);
}
