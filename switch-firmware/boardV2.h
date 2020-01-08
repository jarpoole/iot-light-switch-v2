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
#define BaudRate 9600
#define CALC_UBRR (F_CPU / 8 / BaudRate ) - 1

//BMS Constants
#define BMS_ADDRESS 0x6A
#define BMS_INTERRUPT 2  //PD2, INT0
#define BMS_OTG A3       //PC3
#define BMS_CE A2        //PC2
#define BMS_QON A1       //PC1

//Servo Constants
#define SERVO 9          //PB1
#define SERVO_ENABLE 8   //PB0
#define ONPOSITION 75    //Confirmed
#define OFFPOSITION 120  //115
#define OVERSHOOT 10     //30
#define OVERSHOOT_TIME 100
#define MOVE_TIME 200
