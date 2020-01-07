#ifdef F_CPU
#undef F_CPU
#define F_CPU 8000000UL
#endif

//System libraries
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/sleep.h>
#include <avr/wdt.h>

//Modified RadioHead library
#include "./radiohead-ask-atmega328p/RadioHead.h"
#include "./radiohead-ask-atmega328p/RH_ASK.h"
#include "./radiohead-ask-atmega328p/RH_ASK.cpp"
#include "./radiohead-ask-atmega328p/RHCRC.h"
#include "./radiohead-ask-atmega328p/RHCRC.cpp"
#include "./radiohead-ask-atmega328p/RHGenericDriver.cpp"
#include "./radiohead-ask-atmega328p/RHGenericDriver.h"

#include "./pwmservo-atmega328p/PWMServo.h"
#include "./pwmservo-atmega328p/PWMServo.cpp"
//#include <SPI.h> // Not actually used but needed to compile RH_ASK
#include <Wire.h>

//User files
#include "Constants.h"
#include "BoardV2.h"

//From documentation: RH_ASK (uint16_t speed=2000, uint8_t rxPin=11, uint8_t txPin=12, uint8_t pttPin=10, bool pttInverted=false)
//pttPin = -1 will disable push to talk functionality
RH_ASK ask_driver = RH_ASK(500, RECEIVER, TRANSMITTER, -1, false);

PWMServo servo;

volatile long lastButtonPressedTime = 0;
volatile bool lastButtonState = false;

volatile bool switchOn = false;
volatile bool debugOn = false;           

//ISR Toggles
volatile bool requestSwitchToggle = false;
volatile bool requestSwitchOn = false;
volatile bool requestSwitchOff = false;

volatile bool requestSerialOn = false;
volatile bool requestSerialOff = false;

int main (void){
  initialize();

  pinMode(BUZZER, OUTPUT);
  //beep(1,1000);
  
  while(true){
    beep(1,1000);
    Serial.println("1");
    //delay(5000);
    sleep();
  }
  
  /*
  while(true){
    //printAllREG();
    processISRUpdates();                    //Check for changes generated by the ISR
    
    transmitASKByte(WAKE_UP);               //WAKE_UP
    while(receiveASKByte() != ACK);         //Wait for ACK (Blocking with timeout)

    while(true){
      byte command = receiveASKByte();
      if(command == SLEEP){
        break;
      }else{
        if(processASKCommand(command)){
          transmitASKByte(ACK); 
        }else{
          transmitASKByte(NACK); 
        }
      }
    }
    transmitASKByte(ACK);


    if(debugOn) Serial.println("sleeping");
    delay(5000);
    //sleep();
  }
  */
}

void initialize() {
  MCUSR &= ~(1<<WDRF); //Always clear the watchdog system reset flag on startup, p45
  WDTCSR &= ~(1<<WDE); //Always clear the watchdog system reset mode on startup, p45

  //***************************Initialize IO***************************//
  pinMode(BUZZER, OUTPUT);
  pinMode(BUTTON, INPUT);
  attachInterrupt(digitalPinToInterrupt(BUTTON), buttonISR, CHANGE);
 
  enableSerial();                      //*********************FIX ME Should be disabled by default**********************
  
  pinMode(SERVO_ENABLE, OUTPUT);
  digitalWrite(SERVO_ENABLE, LOW);

  
 
  //***************************Initialize Radio***************************//
  pinMode(TRANSMITTER, OUTPUT);
  pinMode(RECEIVER, INPUT);

 
  //ask_driver.init();
  /*
  bool ask_init = ask_driver.init();
  if(debugOn && !ask_init){
    Serial.println("RH_ASK init failed");
  }
  */
  
  //***************************Initialize BMS***************************//
  /*
  Wire.begin();
  pinMode(BMS_INTERRUPT,INPUT);
  attachInterrupt(digitalPinToInterrupt(BMS_INTERRUPT), bmsISR, RISING);
  pinMode(BMS_OTG, OUTPUT);
  digitalWrite(BMS_OTG, HIGH);        //*********************FIX ME (OTG shouldn't be on by default)**********************
  
  setOTG_CONFIG(true);    //enable boost mode
  setCHG_CONFIG(false);   //Disable charging
  setCONV_RATE(true);     //enable continuous ADC for testing
  enableWatchdog(false);  //Disable watchdog
  writeRegister(0x0A, 0x77); //Set BOOST_LIM value to 111 = 2.45A

  initializeTimer2Sleep();
  SMCR|=(1<<SM1)|(1<<SM0); //Set sleep mode control register to PowerSave Mode (Bits SM2 SM1 SM0 = 011 , p38)
  sei();   //Enable global interrupts

  //setSTAT_DIS(true);
  */
}

void initializeTimer2Sleep(){
    //Disable timer2 interrupts by zeroing OCIE2A, OCIE2B and TOIE2
    TIMSK2  = 0;
    //Enable asynchronous mode, Only other R/W bit in ASSR is EXCLK which should be 0
    ASSR  = (1<<AS2);
    //set initial counter value
    TCNT2=0;
    //set OCR2B to 255, This value should set the frequency of the interrupts
    OCR2B = 0b11111111;
    //set prescaller 1024
    TCCR2B |= (1<<CS22) | (1<<CS21) | (1<<CS20);
    //wait for registers TCN2UB, OCR2BUB and TCR2BUB to update by checking status is ASSR
    while ( (1<<TCN2UB)&ASSR | (1<<TCR2BUB)&ASSR | (1<<TCR2BUB)&ASSR);
    //clear interrupt flags;  "Output Compare Flag 2 B" and "Timer/Counter2 Overflow Flag"
    TIFR2  = (1<<TOV2) | (1<<OCF2B);
    //enable OCIE2B interrupt to trigger when compare B matches
    TIMSK2  = (1<<OCIE2B);
}

void sleep(){
  initializeTimer2Sleep();
  cli();           //Disable global interrupts because timed sequence follows
  SMCR |= (1<<SE); //Set "Sleep Mode Control Register" Sleep Enable, SE, Bit = 1, p38
  sei();           //Garentees that the next instruction is executed
  sleep_cpu();     //Issue sleep instruction. This cannot be done by setting a register
  //asm volatile("sleep"::); //Assembly version of sleep_cpu();

        //Sleeping until interrupt

  SMCR &= ~(1<<SE); //Disable sleep enable after wakeup
}
ISR(TIMER2_COMPB_vect){    //fires on wakeup
    asm volatile("nop"::);
}

void processISRUpdates(){
  if(requestSwitchToggle){
    toggleSwitch();
    requestSwitchToggle = false;
  }
  if(requestSwitchOn){
    turnOn();
    requestSwitchOn = false;
  }
  if(requestSwitchOff){
    turnOff();
    requestSwitchOff = false;
  }
  if(requestSerialOn){
    enableSerial();
    requestSerialOn = false;
  }
}
void buttonISR(){
  /*
  bool buttonState = digitalRead(BUTTON);

  //digitalWrite(SERVO_ENABLE, !digitalRead(SERVO_ENABLE)); // Toggle the LED for testing

  if(buttonState == true && lastButtonState == false){  //RISING
    lastButtonPressedTime = millis();
  }
  if(buttonState == false && lastButtonState == true){  //FALLING
    if ((unsigned long)(millis() - lastButtonPressedTime) >= SERIAL_ENABLE_TIME){
        requestSerialOn = true;
    }else{
      requestSwitchToggle = true;
    }
  }
  lastButtonState = buttonState;
  */
}
void bmsISR(){
  
}
void toggleSwitch(){
  if(switchOn){
    beep(1, 10);
    //turnOff();
  }else{
    beep(2, 10);
    //turnOn();
  }
}
void turnOn(){
  servo.attach(SERVO);
  _delay_ms(15);
  digitalWrite(SERVO_ENABLE, HIGH);
  //servo.write(ONPOSITION - OVERSHOOT);
  //delay(OVERSHOOT_TIME);
  servo.write(ONPOSITION);
  _delay_ms(MOVE_TIME);
  digitalWrite(SERVO_ENABLE, LOW);
  servo.detach();
  switchOn = true;
}
void turnOff(){
  servo.attach(SERVO);
  _delay_ms(15);
  digitalWrite(SERVO_ENABLE, HIGH);
  //servo.write(OFFPOSITION + OVERSHOOT);
  //delay(OVERSHOOT_TIME);
  servo.write(OFFPOSITION);
  _delay_ms(MOVE_TIME);
  digitalWrite(SERVO_ENABLE, LOW);
  servo.detach();
  switchOn = false;
}
void enableSerial(){
  Serial.begin(SERIAL_BAUD);
  Serial.println(F("Serial enabled"));
  debugOn = true;
}
void disableSerial(){
  Serial.println(F("Goodbye"));
  Serial.end();
  debugOn = false;
}
void beep(int num, int timeMillis){
    for(int i = 0; i < num; i++){
        digitalWrite(BUZZER, HIGH);
        _delay_ms(timeMillis);
        digitalWrite(BUZZER, LOW);
        if (i < num -1){
            _delay_ms(timeMillis*2);
        }
    }
}

//*********************************RH_ASK wrapper functions****************************************//
bool processASKCommand(byte command){
    if(command == BEEP){
        beep(1, 10);
        return true;
    }else if(command == SWITCH_ON){
        turnOn();
        return true;
    }else if(command == SWITCH_OFF){
        turnOff();
        return true;
    }else if(command == SWITCH_TOGGLE){
        toggleSwitch();
        return true;
    }else{
        return false;
    }
}

byte receiveASKByte(){ //blocking
    uint8_t buf[1];
    uint8_t buflen = sizeof(buf);
    //if(debugOn) Serial.print("Receiving");
    
    long timeout = millis() + TIMEOUT;    
    while(!ask_driver.recv(buf, &buflen) && millis() < timeout){    // Non-blocking  
      if(debugOn){
        Serial.print(".");
        _delay_ms(100);
      }
    }
    //if(debugOn) Serial.print("\n");
    return(buf[0]); 
}

void transmitASKByte(byte data){
    ask_driver.send(&data, 1);
    ask_driver.waitPacketSent();
}

byte checkASKByte(){
    uint8_t buf[1];
    uint8_t buflen = sizeof(buf);
    if(ask_driver.recv(buf, &buflen)){
        return(buf[0]); 
    }else{
        return NULL;
    }
}

//*********************************BMS functions****************************************//
byte readRegister(byte address){
  Wire.beginTransmission(BMS_ADDRESS);
  Wire.write(address);
  Wire.endTransmission();
  
  Wire.requestFrom(BMS_ADDRESS, 1);
  while(Wire.available()>1);
  
  return(Wire.read());
}
void writeRegister(byte address, byte value){
  Wire.beginTransmission(BMS_ADDRESS);
  Wire.write(address);
  Wire.write(value);
  Wire.endTransmission();
}

void writeRegisterBit(byte address, int pos, bool value){
  byte currentData = readRegister(address);         //Read the current state
  currentData = bitWrite(currentData, pos, value);  //Change the specified bit
  writeRegister(address, currentData);              //Write the changes
}

void setCONV_RATE(bool value){ //Configure continuous ADC updates = TRUE, one shot mode = FALSE
  writeRegisterBit(0x02, 6, value);
}
void setOTG_CONFIG(bool value){
  writeRegisterBit(0x03, 5, value);
}
void setCHG_CONFIG(bool value){
  writeRegisterBit(0x03, 4, value);
}
void setSTAT_DIS(bool value){
  writeRegisterBit(0x07, 6, value);
}
void setWD_RST(bool value){
  writeRegisterBit(0x03, 6, value);
}


void enableWatchdog(bool value){ //00 for disabled, 01 for 40s, 10 for 80s and 11 for 160s
  int bit5;
  int bit4;
  if(value){ 
     bit5 = 0;
     bit4 = 1;
  }else{
    bit5 = 0;
    bit4 = 0;
  }
  byte currentData = readRegister(0x07);         
  currentData = bitWrite(currentData, 5, bit5);  
  currentData = bitWrite(currentData, 4, bit4); 
  writeRegister(0x07, currentData);
}

//**********************************Print methods**********************************************//
void printAllREG(){
  Serial.println();
  for(int i = 0; i < 50; i++) Serial.print("*");

  printREGBinary(0x00);
  printREGBinary(0x01);
  printREGBinary(0x02);
  printREGBinary(0x03);
  printREGBinary(0x04);
  printREGBinary(0x05);
  printREGBinary(0x06);
  printREGBinary(0x07);
  printREGBinary(0x08);
  printREGBinary(0x09);
  printREGBinary(0x0A);

  //Registers 0B to 14 are primarily read only,
  printREG0B();
  printREG0C();
  printREG0D();
  printREG0E();
  printREG0F();
  printREG10();
  printREG11();
  printREG12();
  printREG13();
  printREG14();
  Serial.println();
  for(int i = 0; i < 50; i++) Serial.print("*");
}

void printREGBinary(byte address){
  Serial.println();
  byte data = readRegister(address);
  byte mask = 0b10000000;
  Serial.print(F("Register "));
  Serial.print(address, HEX);
  Serial.print(": ");
  
  for(int i = 0; i<8; i++){
    if(((mask >> i) & data) == (mask>>i)){
      Serial.print("1");
    }else{
      Serial.print("0");
    }
  }
}

void printREG0B(){
  byte data = readRegister(0x0B);
  Serial.println();
  //print charger information
  Serial.print(F("Charger status: "));
  byte charger = data >> 5;
  if      (charger == 0) Serial.println(F("No charger"));
  else if (charger == 1) Serial.println(F("USB Host SDP"));
  else if (charger == 2) Serial.println(F("USB CDP (1.5A)"));
  else if (charger == 3) Serial.println(F("USB DCP (3.25A)"));
  else if (charger == 4) Serial.println(F("Adjustable High Voltage DCP (MaxCharge) (1.5A)"));
  else if (charger == 5) Serial.println(F("Unknown Adapter (500mA)"));
  else if (charger == 6) Serial.println(F("Non-Standard Adapter (1A/2A/2.1A/2.4A)"));
  else if (charger == 7) Serial.println(F("OTG"));

  //print charging status
  Serial.print(F("Charging mode: "));
  byte chargeStatus = data << 3;
  chargeStatus = chargeStatus >> 6;
  if      (chargeStatus == 0) Serial.println(F("Not Charging"));
  else if (chargeStatus == 1) Serial.println(F("Pre-charge"));
  else if (chargeStatus == 2) Serial.println(F("Fast Charging"));
  else if (chargeStatus == 3) Serial.println(F("Charge Termination Done"));

  //print power good
  Serial.print(F("Power status: "));
  byte powerStatus = data << 4;
  powerStatus = powerStatus >> 7;
  if      (powerStatus == 0) Serial.println(F("Error"));
  else if (powerStatus == 1) Serial.println(F("Good"));

  //print power good
  Serial.print(F("VSYS Regulation Status: "));
  byte vsysStatus = data << 7;
  vsysStatus = vsysStatus >> 7;
  if      (powerStatus == 0) Serial.println(F("NOT in VSYSMIN regulation (BAT > VSYSMIN)"));
  else if (powerStatus == 1) Serial.println(F("Good (BAT < VSYSMIN)"));
}


void printREG0C(){
  byte data = readRegister(0x0C);
  Serial.println();
  //print watchdog status
  Serial.print(F("Watchdog status: "));
  byte watchdog = data >> 7;
  if      (watchdog == 0) Serial.println(F("Normal"));
  else if (watchdog == 1) Serial.println(F("Timer expired"));

  //print boost status
  Serial.print("Boost status: ");
  byte boost = data << 1;
  boost = boost >> 7;
  if(boost == 0) Serial.println(F("Normal"));
  else if (boost == 1) {
    Serial.println(F("VBUS overloaded in OTG, or VBUS OVP, or battery is too low in boost mode"));
    beep(2,10);
  }

  //print charger status
  Serial.print(F("Charger status: "));
  byte charger = data << 2;
  charger = charger >> 6;
  if      (charger == 0) Serial.println(F("Normal"));
  else if (charger == 1) Serial.println(F("Input fault"));
  else if (charger == 2) Serial.println(F("Thermal shutdown"));
  else if (charger == 3) Serial.println(F("Charge safety timer expiration"));

  //print battery status
  Serial.print(F("Battery status: "));
  byte battery = data << 4;
  battery = battery >> 7;
  if      (battery == 0) Serial.println(F("Normal"));
  else if (battery == 1) Serial.println(F("Battery over-voltage"));

  //print temperature sensor status
  Serial.print(F("NTC status: "));
  byte ntc = data << 5;
  ntc = ntc >> 5;
  if      (ntc == 0) Serial.println(F("Normal"));
  else if (ntc == 2) Serial.println(F("TS Warm"));
  else if (ntc == 3) Serial.println(F("TS Cool"));
  else if (ntc == 5) Serial.println(F("TS Cold"));
  else if (ntc == 6) Serial.println(F("TS Hot")); 
}


void printREG0D(){
  byte data = readRegister(0x0D);
  Serial.println();
  //print VINDPM status
  Serial.print(F("VINDPM Mode: "));
  byte mode = data >> 7;
  if      (mode == 0) Serial.println(F("Relative VINDPM Threshold"));
  else if (mode == 1) Serial.println(F("Absolute VINDPM Threshold"));

  //print VINDPM value
  Serial.println(F("VINDPM value available but not implemented"));
}


void printREG0E(){
  byte data = readRegister(0x0E);
  Serial.println();
  //print Thermal status
  Serial.print(F("Thermal status: "));
  byte thermal = data >> 7;
  if      (thermal == 0) Serial.println(F("Normal"));
  else if (thermal == 1) Serial.println(F("Thermal throttling"));

  //print Vbat voltage
  Serial.print(F("BAT voltage: "));
  byte voltage = data << 1;
  voltage = voltage >> 1;
  int value = ((voltage & 0b01000000) >> 6) * 1280 + ((voltage & 0b00100000) >> 5) * 640 + ((voltage & 0b00010000) >> 4) * 320 
              + ((voltage & 0b00001000) >> 3) * 160 + ((voltage & 0b00000100) >> 2) * 80 + ((voltage & 0b00000010) >> 1) * 40
              + (voltage & 0b00000001) * 20 + 2304;
  Serial.print(value);
  Serial.println(F("mV"));
}


void printREG0F(){
  byte voltage = readRegister(0x0F);
  Serial.println();

  //print SYS voltage
  Serial.print(F("SYS voltage: "));
  int value = ((voltage & 0b01000000) >> 6) * 1280 + ((voltage & 0b00100000) >> 5) * 640 + ((voltage & 0b00010000) >> 4) * 320 
              + ((voltage & 0b00001000) >> 3) * 160 + ((voltage & 0b00000100) >> 2) * 80 + ((voltage & 0b00000010) >> 1) * 40
              + (voltage & 0b00000001) * 20 + 2304;
  Serial.print(value);
  Serial.println(F("mV"));
}

void printREG10(){
  byte data = readRegister(0x10);
  Serial.println();

  //print TS voltage percentage of REGN
  byte voltage = data << 1;
  voltage = voltage >> 1;
  Serial.print(F("TS voltage vs REGN: "));
  int value = ((voltage & 0b01000000) >> 6) * 2976 + ((voltage & 0b00100000) >> 5) * 1488 + ((voltage & 0b00010000) >> 4) * 744 
              + ((voltage & 0b00001000) >> 3) * 372 + ((voltage & 0b00000100) >> 2) * 186 + ((voltage & 0b00000010) >> 1) * 93
              + (voltage & 0b00000001) * 46 + 2100;
  Serial.print( (float) value / 100);
  Serial.println(F("%"));
}

void printREG11(){
  byte voltage = readRegister(0x11);
  Serial.println();

  //print vbus voltage
  Serial.print(F("VBUS voltage: "));
  unsigned int value = ((voltage & 0b01000000) >> 6) * 6400 + ((voltage & 0b00100000) >> 5) * 3200 + ((voltage & 0b00010000) >> 4) * 1600 
              + ((voltage & 0b00001000) >> 3) * 800 + ((voltage & 0b00000100) >> 2) * 400 + ((voltage & 0b00000010) >> 1) * 200
              + (voltage & 0b00000001) * 100 + 2600;
  Serial.print(value, DEC);
  Serial.println(F("mV"));
}

void printREG12(){
  byte current = readRegister(0x12);
  Serial.println();
  
  //print charge current
  Serial.print(F("Charge current (Ibat): "));
  int value = ((current & 0b01000000) >> 6) * 3200 + ((current & 0b00100000) >> 5) * 1600 + ((current & 0b00010000) >> 4) * 800 
              + ((current & 0b00001000) >> 3) * 400 + ((current & 0b00000100) >> 2) * 200 + ((current & 0b00000010) >> 1) * 100
              + (current & 0b00000001) * 50;
  Serial.print(value);
  Serial.println(F("mA"));
}

void printREG13(){
  byte data = readRegister(0x13);
  Serial.println();
  //print VINDPM status
  Serial.print(F("VINDPM: "));
  byte vindpm = data >> 7;
  if      (vindpm == 0) Serial.println(F("No"));
  else if (vindpm == 1) Serial.println(F("Yes"));

  Serial.print(F("IINDPM: "));
  byte iindpm = data << 1;
  iindpm = iindpm >> 7;
  if      (iindpm == 0) Serial.println(F("No"));
  else if (iindpm == 1) Serial.println(F("Yes"));

  //print input current limit
  Serial.print(F("Input Current Limit (if ICO enabled): "));
  byte current = data;
  int value = ((current & 0b00100000) >> 5) * 1600 + ((current & 0b00010000) >> 4) * 800 + ((current & 0b00001000) >> 3) * 400
              + ((current & 0b00000100) >> 2) * 200 + ((current & 0b00000010) >> 1) * 100 + (current & 0b00000001) * 50 + 100;
  Serial.print(value);
  Serial.println(F("mA"));
}

void printREG14(){
  byte data = readRegister(0x14);
  Serial.println();
  //print ICO optimization status
  Serial.print(F("ICO status: "));
  byte ico = data << 1;
  ico = ico >> 7;
  if      (ico == 0) Serial.println(F("Optimizing..."));
  else if (ico == 1) Serial.println(F("Max current detected"));
}
