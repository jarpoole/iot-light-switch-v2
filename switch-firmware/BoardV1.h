#define BUTTON 3         //PD3, INT1
#define BUZZER 13        //PB5
#define TRANSMITTER 5    //PD5
#define RECEIVER 6       //PD6
#define VAR_RESISTOR A0  //PC0

//Serial Constants
#define SERIAL_ENABLE 4  //PD4
#define SERIAL_ENABLE_TIME 5000
#define SERIAL_BAUD 115200

//BMS Constants
#define BMS_ADDRESS 0x6A
#define BMS_INTERRUPT 2  //PD2, INT0
#define BMS_OTG A3       //PC3
#define BMS_CE A2        //PC2
#define BMS_QON A1       //PC1

//Servo Constants
#define SERVO 9          //PB1
#define SERVO_ENABLE 8   //PB0
#define ONPOSITION 75  //Confirmed
#define OFFPOSITION 120//115
#define OVERSHOOT 10   //30
#define OVERSHOOT_TIME 100
#define MOVE_TIME 200
