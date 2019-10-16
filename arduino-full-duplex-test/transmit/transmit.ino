#include <RH_ASK.h>
#include <SPI.h> // Not actually used but needed to compile

#define TRANSMITTER 5
#define RECEIVER 6

//RH_ASK driver(2000, RECEIVER, TRANSMITTER, -1, false);
RH_ASK driver(500, RECEIVER, TRANSMITTER, -1, false);

void setup()
{
    Serial.begin(115200);    // Debugging only
    if (!driver.init())
         Serial.println("init failed");
}

void loop()
{
    /*
    const char *msg = "Hello World!";
    driver.send((uint8_t *)msg, strlen(msg));
    driver.waitPacketSent();
    delay(1000);
    Serial.println("Sent");
    */

    
    const uint8_t data[4] = {0xFC, 0x45, 0xE5, 0x11};
    driver.send(&data[0], 4);
    driver.waitPacketSent();
    delay(1000);
    Serial.println("Sent");
   
}
