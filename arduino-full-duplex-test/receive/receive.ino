jared#include <RH_ASK.h>
#include <SPI.h> // Not actualy used but needed to compile

#define TRANSMITTER 5
#define RECEIVER 6

RH_ASK driver(500, RECEIVER, TRANSMITTER, -1, false);

void setup()
{
    Serial.begin(115200);  // Debugging only
    Serial.println("Started");
    if (!driver.init())
         Serial.println("driver init failed");
}

void loop()
{
    uint8_t buf[12];
    uint8_t buflen = sizeof(buf);
    if (driver.recv(buf, &buflen)) // Non-blocking
    {
      int i;
      // Message with a good checksum received, dump it.
      Serial.print("Message: ");
      //Serial.println((char*)buf);  
      PrintHex8(buf,buflen);
      Serial.print("\n");      
    }
}

void PrintHex8(uint8_t *data, uint8_t length) // prints 8-bit data in hex with leading zeroes
{
     char tmp[16];
       for (int i=0; i<length; i++) {
         sprintf(tmp, "0x%.2X",data[i]);
         Serial.print(tmp); Serial.print(" ");
       }
}
