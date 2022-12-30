#include <Servo.h>

Servo servo;

void setup() {
  servo.attach(6);

}

void loop() {
  servo.write(40);
  delay(100);
  servo.write(75);
  
  delay(5000);
  
  servo.write(145);
  delay(100);
  servo.write(115);
  
  delay(5000);
}
