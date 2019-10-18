#include <Wire.h>

#define SERIAL_ENABLE 4  //PD4
#define BMS_ADDRESS 0x6A
#define BMS_OTG A3       //PC3
#define READ_MASK 0x01
#define WRITE_MASK 0x00

void setup() {
  Wire.begin();
  Serial.begin(115200);
  
  pinMode(SERIAL_ENABLE, OUTPUT);
  digitalWrite(SERIAL_ENABLE, HIGH); 

  pinMode(BMS_OTG, OUTPUT);
  digitalWrite(BMS_OTG, HIGH); 

  //Wire.beginTransmission(BMS_ADDRESS << 1 | WRITE_MASK);
  Wire.beginTransmission(BMS_ADDRESS);
  Wire.write(0x03);                  //Select Register with boost control (REG03)
  Wire.write(0b00111010);    //Change bit D5 to on to set OTG_CONFIG (Rest are defaulted)
  Wire.endTransmission();


}


void loop() {
  printREG0B();
  printREG0C();
  printREG0D();
  printREG0E();
  printREG0F();
  
  delay(5000);
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


void setOTG_CONFIG(bool value){
  writeRegisterBit(0x03, 5, value);
}
void setCHG_CONFIG(bool value){
  writeRegisterBit(0x03, 4, value);
}
void setSTAT_DIS(bool value){
  writeRegisterBit(0x07, 6, value);
}

//**********************************Print methods**********************************************//
void printREG0B(){
  byte data = readRegister(0x0B);
  Serial.println();
  //print charger information
  Serial.print("Charger status: ");
  byte charger = data >> 5;
  if      (charger == 0) Serial.println("No charger");
  else if (charger == 1) Serial.println("USB Host SDP");
  else if (charger == 2) Serial.println("USB CDP (1.5A)");
  else if (charger == 3) Serial.println("USB DCP (3.25A)");
  else if (charger == 4) Serial.println("Adjustable High Voltage DCP (MaxCharge) (1.5A)");
  else if (charger == 5) Serial.println("Unknown Adapter (500mA)");
  else if (charger == 6) Serial.println("Non-Standard Adapter (1A/2A/2.1A/2.4A)");
  else if (charger == 7) Serial.println("OTG");

  //print charging status
  Serial.print("Charging mode: ");
  byte chargeStatus = data << 3;
  chargeStatus = chargeStatus >> 6;
  if      (chargeStatus == 0) Serial.println("Not Charging");
  else if (chargeStatus == 1) Serial.println("Pre-charge");
  else if (chargeStatus == 2) Serial.println("Fast Charging");
  else if (chargeStatus == 3) Serial.println("Charge Termination Done");

  //print power good
  Serial.print("Power status: ");
  byte powerStatus = data << 4;
  powerStatus = powerStatus >> 7;
  if      (powerStatus == 0) Serial.println("Error");
  else if (powerStatus == 1) Serial.println("Good");

  //print power good
  Serial.print("VSYS Regulation Status: ");
  byte vsysStatus = data << 7;
  vsysStatus = vsysStatus >> 7;
  if      (powerStatus == 0) Serial.println("Not in VSYSMIN regulation (BAT > VSYSMIN)");
  else if (powerStatus == 1) Serial.println("In VSYSMIN regulation (BAT < VSYSMIN)");
}


void printREG0C(){
  byte data = readRegister(0x0C);
  Serial.println();
  //print watchdog status
  Serial.print("Watchdog status: ");
  byte watchdog = data >> 7;
  if      (watchdog == 0) Serial.println("Normal");
  else if (watchdog == 1) Serial.println("Timer expired");

  //print boost status
  Serial.print("Boost status: ");
  byte boost = data << 1;
  boost = boost >> 7;
  if      (boost == 0) Serial.println("Normal");
  else if (boost == 1) Serial.println("VBUS overloaded in OTG, or VBUS OVP, or battery is too low in boost mode");

  //print charger status
  Serial.print("Charger status: ");
  byte charger = data << 2;
  charger = charger >> 6;
  if      (charger == 0) Serial.println("Normal");
  else if (charger == 1) Serial.println("Input fault");
  else if (charger == 2) Serial.println("Thermal shutdown");
  else if (charger == 3) Serial.println("Charge safety timer expiration");

  //print battery status
  Serial.print("Battery status: ");
  byte battery = data << 4;
  battery = battery >> 7;
  if      (battery == 0) Serial.println("Normal");
  else if (battery == 1) Serial.println("Battery over-voltage");

  //print temperature sensor status
  Serial.print("NTC status: ");
  byte ntc = data << 5;
  ntc = ntc >> 5;
  if      (ntc == 0) Serial.println("Normal");
  else if (ntc == 2) Serial.println("TS Warm");
  else if (ntc == 3) Serial.println("TS Cool");
  else if (ntc == 5) Serial.println("TS Cold");
  else if (ntc == 6) Serial.println("TS Hot"); 
}


void printREG0D(){
  byte data = readRegister(0x0D);
  Serial.println();
  //print VINDPM status
  Serial.print("VINDPM Mode: ");
  byte mode = data >> 7;
  if      (mode == 0) Serial.println("Relative VINDPM Threshold");
  else if (mode == 1) Serial.println("Absolute VINDPM Threshold");

  //print VINDPM value
  Serial.println("VINDPM value available but not implemented");
}


void printREG0E(){
  byte data = readRegister(0x0E);
  Serial.println();
  //print Thermal status
  Serial.print("Thermal status: ");
  byte thermal = data >> 7;
  if      (thermal == 0) Serial.println("Normal");
  else if (thermal == 1) Serial.println("Thermal throttling");

  //print Vbat voltage
  Serial.print("BAT voltage: ");
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

  //print Vbat voltage
  Serial.print("SYS voltage: ");
  int value = ((voltage & 0b01000000) >> 6) * 1280 + ((voltage & 0b00100000) >> 5) * 640 + ((voltage & 0b00010000) >> 4) * 320 
              + ((voltage & 0b00001000) >> 3) * 160 + ((voltage & 0b00000100) >> 2) * 80 + ((voltage & 0b00000010) >> 1) * 40
              + (voltage & 0b00000001) * 20 + 2304;
  Serial.print(value);
  Serial.println("mV");
}
