//https://embedds.com/programming-avr-i2c-interface/

//I2C Slave Address: 6AH (1101010B + R/W) (bq25890)
//For read:  0b11010101  -> 0xD5
//For write: 0b11010100  -> 0xD4

#ifdef F_CPU
#undef F_CPU
#define F_CPU 8000000UL
#endif

#define BMS_READ_ADDR 0xD5
#define BMS_WRITE_ADDR 0xD4

void setup() {
    twiBegin();
    Serial.begin(9600);
    Serial.println("Started");
}

void loop() {
    Serial.println("Reading...");
    twiStart();
    twiWrite(BMS_WRITE_ADDR);  //Select BMS in write mode
    twiWrite(0x07);       //Select status led register
    
    twiStart();
    twiWrite(BMS_READ_ADDR);  //Select BMS in read mode
    uint8_t data = twiReadACK();
    twiStop();
    Serial.println(data, HEX);

    data = data | 0b01000000;

    twiStart();
    twiWrite(BMS_WRITE_ADDR);  //Select BMS in read mode
    twiWrite(0x07);       //Select status led register
    twiWrite(data);
    twiStop();
    
    delay(1000);

}

uint8_t twiReadRegister(uint8_t address){
    twiStart();
    twiWrite(BMS_WRITE_ADDR);  //Select BMS in write mode
    twiWrite(address);       //Select register
    twiStart();
    twiWrite(BMS_READ_ADDR);  //Select BMS in read mode
    uint8_t data = twiReadACK();
    twiStop();
    return data;
}

void twiWriteRegister(uint8_t address, uint8_t data){
    twiStart();
    twiWrite(BMS_WRITE_ADDR);  //Select BMS in write mode
    twiWrite(address);         //Select register
    twiWrite(data);            //Write data 
    twiStop();
}


void twiBegin(){
    PRR &= ~(1<<PRTWI); //Enable TWI clock from sleep
    TWSR = 0x00;        //Don't need prescaler so set to 0
    TWBR = 0x0C;        //set SCL to 400kHz
    TWCR = (1<<TWEN);   //enable TWI
}

void twiEnd(){
    TWCR &= ~(1<<TWEN); //Disable TWI
    PRR |= (1<<PRTWI);  //Disable TWI clock for sleep
}

void twiStart(){
    TWCR = (1<<TWINT)|(1<<TWSTA)|(1<<TWEN);
    while ((TWCR & (1<<TWINT)) == 0);
}

void twiStop(){
    TWCR = (1<<TWINT)|(1<<TWSTO)|(1<<TWEN);
}

void twiWrite(uint8_t u8data){
    TWDR = u8data;
    TWCR = (1<<TWINT)|(1<<TWEN);
    while ((TWCR & (1<<TWINT)) == 0);
}

uint8_t twiReadACK(){
    TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWEA);
    while ((TWCR & (1<<TWINT)) == 0);
    return TWDR;
}

uint8_t twiReadNACK(){
    TWCR = (1<<TWINT)|(1<<TWEN);
    while ((TWCR & (1<<TWINT)) == 0);
    return TWDR;
}

uint8_t twiGetStatus(){
    uint8_t status;
    status = TWSR & 0xF8; //mask status
    return status;
}
