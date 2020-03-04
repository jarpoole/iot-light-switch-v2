#include <RH_ASK.h>

//#include <util/delay.h>
#define F_CPU 8000000UL

//NODE_MCU_PINS
//#define TRANSMITTER 4
//#define RECEIVER 5

//ESP8266 PINS
#define TRANSMITTER 2
#define RECEIVER 0

//RH_ASK driver(2000, RECEIVER, TRANSMITTER, -1, false);
RH_ASK driver(500, RECEIVER, TRANSMITTER, -1, false);

bool buttonState;

void setup()
{
    Serial.begin(115200);    // Debugging only
    if (!driver.init())
         Serial.println("init failed");

    //DDRB |= 0b00000001; //Enable PB0 (Pin 8) as output
    //DDRD &= 0b11111101; //Enable PD2 (Pin 2) as input
    
}

void loop(){

    /* 
    buttonState = PIND & 0b00000100;   //Read PD2 (Pin 2)
    
    if(buttonState){
      PORTB |= 0b00000001;             //Write PB0 (Pin 8) to high
      transmit();
    }else{
      PORTB &= 0b11111110;             //Write PB0 (Pin 8) to low
    }
    delay(10);  
    */   

    transmit();
    delay(1000);
    Serial.println("Sent");
    //_delay_ms(1000); 
}

void transmit(){
    //Serial.println("0");
    const uint8_t data[1] = {0x40}; //BEEP 
    //const uint8_t data[1] = {0x12}; //Toggle switch
    driver.send(&data[0], 1);
    //Serial.println("1");
    
    driver.waitPacketSent();
    //Serial.println("2");
    
    //_delay_ms(200);
    delay(200); 
    //Serial.println("3");
}
