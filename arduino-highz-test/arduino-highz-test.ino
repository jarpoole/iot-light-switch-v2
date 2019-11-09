//attach 3.3v to led to 2.2k resistor to I/O pin 10
//pinMode(INPUT) -> pin is highZ, pinMode(OUTPUT) + digitalWrite(LOW) -> pin is current sink
//
//note INPUT explicitly disables the 20k internal pullups while INPUT_PULLUP enables then

#define PIN 10

void setup() {
  //Initialize pin as highZ
  currentSinkPin(PIN, false);

}

void loop() {
  delay(1000);
  currentSinkPin(PIN, true);
  delay(1000);
  currentSinkPin(PIN, false);
}

void currentSinkPin(int pin, bool value){
  if(value){ //Pin should sink current to turn on light
    pinMode(pin, OUTPUT);
    digitalWrite(pin, LOW);
  }else{
    pinMode(pin, INPUT);
  }
}
