#include <Wire.h>
#include <Stepper.h>

#define I2C_ADDRESS     0x09
#define PIN_STEP_IN1    8
#define PIN_STEP_IN2    9
#define PIN_STEP_IN3    10
#define PIN_STEP_IN4    11
#define STEPS_POR_VUELTA 2048
#define VELOCIDAD_STEP  10

#define PIN_FC_PISO0    12
#define PIN_FC_PISO_MAX 13
#define PIN_LED         3
#define INTERVALO_LED   1000

#define PASOS_POR_NIVEL 5000
#define NIVEL_MIN       0
#define NIVEL_MAX       5

Stepper miStepper(STEPS_POR_VUELTA, PIN_STEP_IN1, PIN_STEP_IN3, PIN_STEP_IN2, PIN_STEP_IN4);

int nivelActual = 0;
long pasosRestantes = 0;
int  direccionActual = 1;
bool elevadorMoviendo = false;
bool estadoLed = LOW;
unsigned long ultimoCambioLed = 0;

void setup() {
  Wire.begin(I2C_ADDRESS);
  Wire.onRequest(requestEvent);
  Wire.onReceive(receiveEvent);
  miStepper.setSpeed(VELOCIDAD_STEP);
  pinMode(PIN_FC_PISO0, INPUT_PULLUP);
  pinMode(PIN_FC_PISO_MAX, INPUT_PULLUP);
  pinMode(PIN_LED, OUTPUT);
  digitalWrite(PIN_LED, LOW);
  Serial.begin(9600);
  Serial.println("=== ESCLAVO 2 (ELEVADOR) LISTO ===");
}

void loop() {
  if (elevadorMoviendo && pasosRestantes > 0) {
    miStepper.step(direccionActual);
    pasosRestantes--;
    if (digitalRead(PIN_FC_PISO0) == LOW && direccionActual < 0) { pasosRestantes = 0; elevadorMoviendo = false; nivelActual = 0; }
    if (digitalRead(PIN_FC_PISO_MAX) == LOW && direccionActual > 0) { pasosRestantes = 0; elevadorMoviendo = false; nivelActual = NIVEL_MAX; }
    if (pasosRestantes == 0) { elevadorMoviendo = false; }
  }

  if (millis() - ultimoCambioLed >= INTERVALO_LED) {
    ultimoCambioLed = millis();
    estadoLed = !estadoLed;
    digitalWrite(PIN_LED, estadoLed);
  }
  delay(1);
}

void requestEvent() {
  char buffer[32];
  snprintf(buffer, sizeof(buffer), "NIVEL:%02d|PAS:%05ld\n", nivelActual, pasosRestantes);
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
  if (cmd.startsWith("SUBIR_NIVEL")) {
    int n = 1; int idx = cmd.indexOf(' ');
    if (idx > 0) n = cmd.substring(idx + 1).toInt();
    if (n > 0) moverNiveles(+n);
  }
  else if (cmd.startsWith("BAJAR_NIVEL")) {
    int n = 1; int idx = cmd.indexOf(' ');
    if (idx > 0) n = cmd.substring(idx + 1).toInt();
    if (n > 0) moverNiveles(-n);
  }
  else if (cmd.startsWith("IR_A_NIVEL")) {
    int idx = cmd.indexOf(' ');
    if (idx > 0) {
      int destino = cmd.substring(idx + 1).toInt();
      if (destino >= NIVEL_MIN && destino <= NIVEL_MAX) moverNiveles(destino - nivelActual);
    }
  }
  else if (cmd == "ELEVADOR_PARAR") { elevadorMoviendo = false; pasosRestantes = 0; }
  else if (cmd == "CALIBRAR") { nivelActual = 0; direccionActual = -1; pasosRestantes = 99999; elevadorMoviendo = true; }
}

void moverNiveles(int n) {
  int destino = nivelActual + n;
  if (destino < NIVEL_MIN) destino = NIVEL_MIN;
  if (destino > NIVEL_MAX) destino = NIVEL_MAX;
  if (destino == nivelActual) return;
  int niveles = destino - nivelActual;
  pasosRestantes = (long)abs(niveles) * PASOS_POR_NIVEL;
  direccionActual = (niveles > 0) ? 1 : -1;
  elevadorMoviendo = true;
  nivelActual = destino;
}