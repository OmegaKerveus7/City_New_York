const int TRIG_PIN = 10;
const int ECHO_PIN = 9;

void setup() {
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  Serial.begin(9600);
  Serial.println("Sensor HC-SR04 iniciado");
}

long medirDistancia() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(5);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duracion = pulseIn(ECHO_PIN, HIGH, 30000);
  if (duracion == 0) return -1;

  return duracion / 58; // centímetros
}

void loop() {
  long suma = 0;
  int validas = 0;

  for (int i = 0; i < 5; i++) {
    long d = medirDistancia();
    if (d > 0 && d < 400) {
      suma += d;
      validas++;
    }
    delay(30);
  }

  if (validas > 0) {
    long promedio = suma / validas;
    Serial.print("Distancia: ");
    Serial.print(promedio);
    Serial.println(" cm");
  } else {
    Serial.println("Fuera de rango");
  }

  delay(500);
}