#ifdef F_CPU
#undef F_CPU
#define F_CPU 8000000UL
#endif

#define BUZZER 13        //PB5

#include <Wire.h>
#include <PWMServo.h>

#define SERIAL_ENABLE 4  //PD4
#define BMS_ADDRESS 0x6A
#define BMS_OTG A3       //PC3
//#define READ_MASK 0x01
//#define WRITE_MASK 0x00
//#define WATCHDOG_TIME 0x01 //01 for 40s, 02 for 80s and 11 for 160s

PWMServo servo;

void setup() {
  Wire.begin();
  Serial.begin(115200);
  pinMode(BUZZER, OUTPUT);
  
  pinMode(SERIAL_ENABLE, OUTPUT);
  digitalWrite(SERIAL_ENABLE, HIGH); 

  pinMode(BMS_OTG, OUTPUT);
  digitalWrite(BMS_OTG, HIGH); 

  servo.attach(9);

  setOTG_CONFIG(true); //enable boost mode
  setCHG_CONFIG(false); //Disable charging

  setCONV_RATE(true); //enable continuous ADC for testing

  enableWatchdog(false);

  writeRegister(0x0A, 0x77); //Set BOOST_LIM value to 111 = 2.45A
}


void loop() {
  printAllREG();
  delay(500);

  setWD_RST(true); //Keep the watchdog timer from expiring

  servo.write(30);
  delay(5000);
  servo.write(150);
  delay(5000);
  
}

void beep(int num, int timeMillis){
    for(int i = 0; i < num; i++){
        digitalWrite(BUZZER, HIGH);
        delay(timeMillis);
        digitalWrite(BUZZER, LOW);
        delay(timeMillis*2);
    }
}

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
  Serial.print("Register ");
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
  Serial.print("Charger status: ");
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
  Serial.print("Charging mode: ");
  byte chargeStatus = data << 3;
  chargeStatus = chargeStatus >> 6;
  if      (chargeStatus == 0) Serial.println(F("Not Charging"));
  else if (chargeStatus == 1) Serial.println(F("Pre-charge"));
  else if (chargeStatus == 2) Serial.println(F("Fast Charging"));
  else if (chargeStatus == 3) Serial.println(F("Charge Termination Done"));

  //print power good
  Serial.print("Power status: ");
  byte powerStatus = data << 4;
  powerStatus = powerStatus >> 7;
  if      (powerStatus == 0) Serial.println(F("Error"));
  else if (powerStatus == 1) Serial.println(F("Good"));

  //print power good
  Serial.print("VSYS Regulation Status: ");
  byte vsysStatus = data << 7;
  vsysStatus = vsysStatus >> 7;
  if      (powerStatus == 0) Serial.println(F("NOT in VSYSMIN regulation (BAT > VSYSMIN)"));
  else if (powerStatus == 1) Serial.println(F("Good (BAT < VSYSMIN)"));
}


void printREG0C(){
  byte data = readRegister(0x0C);
  Serial.println();
  //print watchdog status
  Serial.print("Watchdog status: ");
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
  Serial.print("Charger status: ");
  byte charger = data << 2;
  charger = charger >> 6;
  if      (charger == 0) Serial.println(F("Normal"));
  else if (charger == 1) Serial.println(F("Input fault"));
  else if (charger == 2) Serial.println(F("Thermal shutdown"));
  else if (charger == 3) Serial.println(F("Charge safety timer expiration"));

  //print battery status
  Serial.print("Battery status: ");
  byte battery = data << 4;
  battery = battery >> 7;
  if      (battery == 0) Serial.println(F("Normal"));
  else if (battery == 1) Serial.println(F("Battery over-voltage"));

  //print temperature sensor status
  Serial.print("NTC status: ");
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
  Serial.print("Thermal status: ");
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
  Serial.println("mV");
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
  Serial.println("mV");
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
  Serial.println("%");
}

void printREG11(){
  byte voltage = readRegister(0x11);
  Serial.println();

  //print vbus voltage
  Serial.print("VBUS voltage: ");
  unsigned int value = ((voltage & 0b01000000) >> 6) * 6400 + ((voltage & 0b00100000) >> 5) * 3200 + ((voltage & 0b00010000) >> 4) * 1600 
              + ((voltage & 0b00001000) >> 3) * 800 + ((voltage & 0b00000100) >> 2) * 400 + ((voltage & 0b00000010) >> 1) * 200
              + (voltage & 0b00000001) * 100 + 2600;
  Serial.print(value, DEC);
  Serial.println("mV");
}

void printREG12(){
  byte current = readRegister(0x12);
  Serial.println();
  
  //print charge current
  Serial.print("Charge current (Ibat): ");
  int value = ((current & 0b01000000) >> 6) * 3200 + ((current & 0b00100000) >> 5) * 1600 + ((current & 0b00010000) >> 4) * 800 
              + ((current & 0b00001000) >> 3) * 400 + ((current & 0b00000100) >> 2) * 200 + ((current & 0b00000010) >> 1) * 100
              + (current & 0b00000001) * 50;
  Serial.print(value);
  Serial.println("mA");
}

void printREG13(){
  byte data = readRegister(0x13);
  Serial.println();
  //print VINDPM status
  Serial.print(F("VINDPM: "));
  byte vindpm = data >> 7;
  if      (vindpm == 0) Serial.println("No");
  else if (vindpm == 1) Serial.println("Yes");

  Serial.print(F("IINDPM: "));
  byte iindpm = data << 1;
  iindpm = iindpm >> 7;
  if      (iindpm == 0) Serial.println("No");
  else if (iindpm == 1) Serial.println("Yes");

  //print input current limit
  Serial.print(F("Input Current Limit (if ICO enabled): "));
  byte current = data;
  int value = ((current & 0b00100000) >> 5) * 1600 + ((current & 0b00010000) >> 4) * 800 + ((current & 0b00001000) >> 3) * 400
              + ((current & 0b00000100) >> 2) * 200 + ((current & 0b00000010) >> 1) * 100 + (current & 0b00000001) * 50 + 100;
  Serial.print(value);
  Serial.println("mA");
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
