
//Flow Control
#define ERROR           0xFF   //255
#define ACK             0x01   //01
#define WAKE_UP         0x02   //02
#define NO_CHANGE       0x03   //03

//Switch Control
#define SWITCH_ON       0x10   //16
#define SWITCH_OFF      0x11   //17
#define SWITCH_TOGGLE   0x12   //18
 
//Configure ATMEGA328
#define SET_POLL_RATE   0x20   //32
#define ENABLE_SERIAL   0x21   //33
#define DISABLE_SERIAL  0x22   //34