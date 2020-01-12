#ifdef F_CPU
#undef F_CPU
#define F_CPU 8000000UL
#endif

#define BUTTON 3         //PD3, INT1
#define LED 7            //PD7
#define BUZZER 10        //PB2
#define TRANSMITTER 5    //PD5
#define RECEIVER 6       //PD6
#define VAR_RESISTOR A7  //PE3, ADC7

//Serial Constants
#define BaudRate 38400
#define CALC_UBRR (F_CPU / (16 * BaudRate)) - 1  //Disable 2x mode
//#define CALC_UBRR (F_CPU / (8 * BaudRate)) - 1  //Requires 2x mode
//#define CALC_UBRR 12  //38.4k

//BMS Constants
#define BMS_INTERRUPT 2  //PD2, INT0
#define BMS_OTG A3       //PC3
#define BMS_CE A2        //PC2
#define BMS_QON A1       //PC1
//I2C Slave Address: 6AH (1101010B + R/W) (bq25890)
//For read:  0b11010101  -> 0xD5
//For write: 0b11010100  -> 0xD4
#define BMS_READ_ADDR 0xD5
#define BMS_WRITE_ADDR 0xD4
#define BMS_OTG_TIME 100 //Turn-on time

//Servo Constants
#define SERVO 9          //PB1
#define SERVO_ENABLE 8   //PB0
#define ONPOSITION 70    //Confirmed 75
#define OFFPOSITION 125  //115
#define MOVE_TIME 500
