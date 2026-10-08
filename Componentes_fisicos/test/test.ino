#include <Servo.h>

Servo miServo;  // Crear objeto servo

void setup() {
  miServo.attach(9);  // Pin de señal del servo
}

void loop() {
  // De 0° a 180°
  for (int angulo = 0; angulo <= 180; angulo++) {
    miServo.write(angulo);
    delay(15);
  }
  
  // De 180° a 0°
  for (int angulo = 180; angulo >= 0; angulo--) {
    miServo.write(angulo);
    delay(15);
  }
}