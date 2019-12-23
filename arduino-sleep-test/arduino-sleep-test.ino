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
#include <avr/power.h>
#include <avr/wdt.h>


void setup() {
  pinMode(13, OUTPUT);
  Serial.begin(115200);
}

void loop() {
  
  for(int i = 0; i < 5; i++){
      digitalWrite(13, HIGH);
      delay(500);
      digitalWrite(13, LOW);
      delay(500);
  }

  sleep();

}

// Interrupt triggered on Timer2 overflow
ISR(TIMER2_COMPA_vect){

}
void sleep(){

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

  /*
  ASSR = (1<<AS2);                        // Make Timer2 asynchronous using Asynchronous Status Register
  TCCR2A = (1<<WGM21);                    // CTC mode
  TCCR2B = (1<<CS22)|(1<<CS21)|(1<<CS20); // Prescaler of 1024
  OCR2A = 239;                            // Count up to 240 (zero relative!)
  TIMSK2 = (1<<OCIE2A);                   // Enable compare match interrupt
  */
  
  //ADCSRA = 0;  // disable ADC
  // Disable unneeded peripherals
  power_adc_disable(); // Disable Analog to digital converter
  //power_twi_disable(); // Disable I2C
  
  MCUSR = 0;   // clear various "reset" flags 
  wdt_disable();
  
  set_sleep_mode (SLEEP_MODE_PWR_SAVE);  
  noInterrupts ();           // timed sequence follows
  sleep_enable();
  interrupts ();             // guarantees next instruction executed
  sleep_cpu ();  

  //Processor is sleeping

  sleep_disable(); // cancel sleep as a precaution
}
