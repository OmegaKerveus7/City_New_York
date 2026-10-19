const int pinMotor = 9; // Pin PWM (en Arduino Uno: 3, 5, 6, 9, 10, 11)

void setup() {
  pinMode(pinMotor, OUTPUT);
}

void loop() {
  // Velocidad al 100% (potencia máxima)
  analogWrite(pinMotor, 255);
  delay(10000);

}