#define TIMEOUT         100

//Flow Control
#define NACK            0xFF
#define ACK             0x01
#define WAKE_UP         0x02   //Begin transmission
#define SLEEP		        0x03   //End transmission
#define NO_CHANGE       0x04

//Switch Control
#define SWITCH_ON       0x10
#define SWITCH_OFF      0x11
#define SWITCH_TOGGLE   0x12

//Configure ATMEGA328
#define SET_POLL_RATE   0x20
#define ENABLE_SERIAL   0x21
#define DISABLE_SERIAL  0x22

//Debug
#define BEEP            0x40
