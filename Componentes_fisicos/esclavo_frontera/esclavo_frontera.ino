#include <Wire.h>
#include <Servo.h>
#include <Stepper.h>

#define I2C_ADDRESS     0x08
#define PIN_SERVO       6
#define ANGULO_ABIERTO  115
#define ANGULO_CERRADO  180
#define ANGULO_MIN      0
#define ANGULO_MAX      180

#define PIN_STEP_IN1    8
#define PIN_STEP_IN2    9
#define PIN_STEP_IN3    10
#define PIN_STEP_IN4    11
#define STEPS_VUELTA    10500
#define VELOCIDAD_STEP  5

Servo miServo;
Stepper miStepper(STEPS_VUELTA, PIN_STEP_IN1, PIN_STEP_IN3, PIN_STEP_IN2, PIN_STEP_IN4);

int anguloActual = 90;
int anguloObjetivo = 90;
bool moviendoServo = false;
bool telefericoMoviendo = false;
int pasosTeleferico = 0;
int pasosRestantes = 0;

void setup() {
  Wire.begin(I2C_ADDRESS);
  Wire.onRequest(requestEvent);
  Wire.onReceive(receiveEvent);
  
  miServo.attach(PIN_SERVO);
  miServo.write(anguloActual);
  miStepper.setSpeed(VELOCIDAD_STEP);
  
  Serial.begin(9600);
  Serial.println("=== ESCLAVO 1 (PUENTE/TELEFERICO) LISTO ===");
}

void loop() {
  if (moviendoServo) {
    if (anguloActual < anguloObjetivo) { anguloActual++; miServo.write(anguloActual); delay(20); } 
    else if (anguloActual > anguloObjetivo) { anguloActual--; miServo.write(anguloActual); delay(20); } 
    else { moviendoServo = false; }
  }
  
  if (telefericoMoviendo && pasosRestantes > 0) {
    int direccion = (pasosTeleferico > 0) ? 1 : -1;
    miStepper.step(direccion);
    pasosRestantes--;
    if (pasosRestantes == 0) { telefericoMoviendo = false; }
  }
  delay(2);
}

void requestEvent() {
  char buffer[48];
  snprintf(buffer, sizeof(buffer), "POS:%03d|TELEFERICO:%s|PAS:%05d\n", anguloActual, telefericoMoviendo ? "MOVIENDO" : "DETENIDO", pasosRestantes);
  Wire.write((uint8_t*)buffer, strlen(buffer));
}

void receiveEvent(int howMany) {
  String cmd = "";
  while (Wire.available()) cmd += (char)Wire.read();
  cmd.trim(); cmd.toUpperCase();
  if (cmd.length() == 0) return;
  procesarComando(cmd);
}

void procesarComando(String cmd) {
  if (cmd == "SERVO_SUBIR") { iniciarMovimientoServo(ANGULO_ABIERTO); }
  else if (cmd == "SERVO_BAJAR") { iniciarMovimientoServo(ANGULO_CERRADO); }
  else if (cmd.startsWith("SERVO ") && !cmd.startsWith("SERVO_S")) {
    int ang = cmd.substring(6).toInt();
    if (ang >= ANGULO_MIN && ang <= ANGULO_MAX) iniciarMovimientoServo(ang);
  }
  else if (cmd == "SERVO_STOP") { moviendoServo = false; }
  else if (cmd == "TELEFERICO_AVANZAR") { iniciarTeleferico(STEPS_VUELTA); }
  else if (cmd.startsWith("TELEFERICO_AVANZAR ")) {
    int pasos = cmd.substring(19).toInt();
    if (pasos > 0) iniciarTeleferico(pasos);
  }
  else if (cmd == "TELEFERICO_REGRESAR") { iniciarTeleferico(-STEPS_VUELTA); }
  else if (cmd.startsWith("TELEFERICO_REGRESAR ")) {
    int pasos = cmd.substring(20).toInt();
    if (pasos > 0) iniciarTeleferico(-pasos);
  }
  else if (cmd == "TELEFERICO_PARAR") { telefericoMoviendo = false; pasosRestantes = 0; }
}

void iniciarMovimientoServo(int destino) {
  anguloObjetivo = constrain(destino, ANGULO_MIN, ANGULO_MAX);
  moviendoServo = true;
}

void iniciarTeleferico(int pasos) {
  pasosTeleferico = pasos;
  pasosRestantes = abs(pasos);
  telefericoMoviendo = true;
}