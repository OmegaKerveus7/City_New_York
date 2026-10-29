// Declaración individual de cada pin (GPIO del ESP32)
const int pin2  = 2;
const int pin4  = 4;
const int pin5  = 5;
const int pin13 = 13;
const int pin14 = 14;
const int pin16 = 16;
const int pin17 = 17;
const int pin18 = 18;
const int pin19 = 19;

void setup() {
  // Configurar cada pin como salida (uno por uno)
  pinMode(pin2,  OUTPUT);
  pinMode(pin4,  OUTPUT);
  pinMode(pin5,  OUTPUT);
  pinMode(pin13, OUTPUT);
  pinMode(pin14, OUTPUT);
  pinMode(pin16, OUTPUT);
  pinMode(pin17, OUTPUT);
  pinMode(pin18, OUTPUT);
  pinMode(pin19, OUTPUT);

  // Apagar todos al inicio
  digitalWrite(pin2,  LOW);
  digitalWrite(pin4,  LOW);
  digitalWrite(pin5,  LOW);
  digitalWrite(pin13, LOW);
  digitalWrite(pin14, LOW);
  digitalWrite(pin16, LOW);
  digitalWrite(pin17, LOW);
  digitalWrite(pin18, LOW);
  digitalWrite(pin19, LOW);
}

void loop() {
  // Ejemplo: enciendo los pares y dejo apagados los impares
  digitalWrite(pin2,  HIGH);
  digitalWrite(pin4,  LOW);
  digitalWrite(pin5,  HIGH);
  digitalWrite(pin13, LOW);
  digitalWrite(pin14, HIGH);
  digitalWrite(pin16, LOW);
  digitalWrite(pin17, HIGH);
  digitalWrite(pin18, LOW);
  digitalWrite(pin19, HIGH);
}