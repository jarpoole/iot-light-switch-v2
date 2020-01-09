//https://embedds.com/programming-avr-i2c-interface/

//I2C Slave Address: 6AH (1101010B + R/W) (bq25890)
//For read:  0b11010101  -> 0xD5
//For write: 0b11010100  -> 0xD4

#ifdef F_CPU
#undef F_CPU
#define F_CPU 8000000UL
#endif

#define READ_ADDR 0xD5
#define WRITE_ADDR 0xD4

void setup() {
    twiBegin();
    Serial.begin(9600);
    Serial.println("Started");
}

void loop() {
    Serial.println("Reading...");
    twiStart();
    twiWrite(WRITE_ADDR);  //Select BMS in read mode
    twiWrite(0x07);       //Select status led register
    
    twiStart();
    twiWrite(READ_ADDR);  //Select BMS in read mode
    uint8_t data = twiReadACK();
    twiStop();
    Serial.println(data, HEX);

    data = data | 0b01000000;

    twiStart();
    twiWrite(WRITE_ADDR);  //Select BMS in read mode
    twiWrite(0x07);       //Select status led register
    twiWrite(data);
    twiStop();
    
    delay(1000);

}

void twiBegin(){
    TWSR = 0x00;      //Don't need prescaler so set to 0
    TWBR = 0x0C;      //set SCL to 400kHz
    TWCR = (1<<TWEN); //enable TWI
}

void twiStart(){
    TWCR = (1<<TWINT)|(1<<TWSTA)|(1<<TWEN);
    while ((TWCR & (1<<TWINT)) == 0);
}

//send stop signal
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

//read byte with NACK
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
