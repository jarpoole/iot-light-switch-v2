#include <RH_ASK.h>
#include <SPI.h> // Not actually used but needed to compile

#define TRANSMITTER 5
#define RECEIVER 6

//From documentation: RH_ASK (uint16_t speed=2000, uint8_t rxPin=11, uint8_t txPin=12, uint8_t pttPin=10, bool pttInverted=false)
//pttPin = -1 will disable push to talk functionality
RH_ASK driver(2000, RECEIVER, TRANSMITTER, -1, false);

void setup()
{
    Serial.begin(115200);    // Debugging only
    if (!driver.init()){
         Serial.println("init failed");
    }
    transmitByte(0x0);
}

void loop()
{
    byte count = receiveByte();
    Serial.println(count);
    count = count + 1;

    delay(1000);
    transmitByte(count);
}

byte receiveByte(){ //blocking
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
