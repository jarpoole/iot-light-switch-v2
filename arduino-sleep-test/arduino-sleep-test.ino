/*
 * Information from: http://www.gammon.com.au/power
 * 
 * Fuse bit changes
 *  -Internal clock to 8Mhz
 *  -Disable Brown Out Detection
 *  
 * Strategies
 *  -All pins should be INPUT (pull down) when not used
 *   
 */

#include <avr/sleep.h>

void setup() {
  // put your setup code here, to run once:

}

void loop() {
  // put your main code here, to run repeatedly:

}

ADCSRA &= ~bit(ADEN); //Disable ADC
  MCUSR = 0; //Clear reset flags

  set_sleep_mode(SLEEP_MODE_PWR_SAVE); //Set sleep mode
  noInterrupts(); //Disable interrupts
  sleep_enable(); //Enable sleep

  //Disable BOD
  MCUCR = bit(BODS) | bit(BODSE);
  MCUCR = bit(BODS);
 
  interrupts(); //Enable interrupts
  sleep_cpu(); //Go to sleep
  sleep_disable(); //Disable sleep mode after wakeup
}

void sleep(){
  
  
  ADCSRA = 0;  // disable ADC

  
  MCUSR = 0;   // clear various "reset" flags 
  
  // allow changes, disable reset
  WDTCSR = bit (WDCE) | bit (WDE);
  // set interrupt mode and an interval 
  WDTCSR = bit (WDIE) | bit (WDP3) | bit (WDP0);    // set WDIE, and 8 seconds delay
  wdt_reset();  // pat the dog
  
  set_sleep_mode (SLEEP_MODE_PWR_SAVE);  
  noInterrupts ();           // timed sequence follows
  sleep_enable();
  interrupts ();             // guarantees next instruction executed
  sleep_cpu ();  

  //Processor is sleeping

  sleep_disable(); // cancel sleep as a precaution
}
