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

// Orden: IN1, IN3, IN2, IN4 (importante para ULN2003)
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
  delay(1500); // Espera para que el Monitor Serie abra bien
  
  // --- MENÚ DE OPCIONES EN PANTALLA ---
  Serial.println("");
  Serial.println("=== ESCLAVO 2 (ELEVADOR) LISTO ===");
  Serial.print("I2C: 0x"); Serial.println(I2C_ADDRESS, HEX);
  Serial.print("Pasos por nivel: "); Serial.println(PASOS_POR_NIVEL);
  Serial.print("LED en pin: "); Serial.println(PIN_LED);
  Serial.println("Comandos disponibles:");
  Serial.println("  SUBIR_NIVEL n     -> sube n pisos");
  Serial.println("  BAJAR_NIVEL n     -> baja n pisos");
  Serial.println("  IR_A_NIVEL n      -> va al piso n");
  Serial.println("  ELEVADOR_PARAR    -> detener");
  Serial.println("  ELEVADOR_STATUS   -> reportar estado");
  Serial.println("  CALIBRAR          -> bajar hasta piso 0");
  Serial.println("=======================================");
}

void loop() {
  // --- Movimiento paso a paso (no bloqueante) ---
  if (elevadorMoviendo && pasosRestantes > 0) {
    miStepper.step(direccionActual);
    pasosRestantes--;

    // Chequeo de fin de carrera piso 0
    if (digitalRead(PIN_FC_PISO0) == LOW && direccionActual < 0) {
      pasosRestantes = 0;
      elevadorMoviendo = false;
      nivelActual = 0;
      Serial.println(">> FIN CARRERA PISO 0 detectado");
    }
    // Chequeo de fin de carrera piso MAX
    if (digitalRead(PIN_FC_PISO_MAX) == LOW && direccionActual > 0) {
      pasosRestantes = 0;
      elevadorMoviendo = false;
      nivelActual = NIVEL_MAX;
      Serial.println(">> FIN CARRERA PISO MAX detectado");
    }

    if (pasosRestantes == 0) {
      elevadorMoviendo = false;
      Serial.print(">> Elevador detenido en nivel ");
      Serial.println(nivelActual);
    }
  }

  // --- LED parpadeando cada 1 segundo (no bloqueante) ---
  if (millis() - ultimoCambioLed >= INTERVALO_LED) {
    ultimoCambioLed = millis();
    estadoLed = !estadoLed;
    digitalWrite(PIN_LED, estadoLed);
  }

  // --- Comandos manuales por Monitor Serie (para pruebas) ---
  if (Serial.available()) {
    String cmdSerial = Serial.readStringUntil('\n');
    cmdSerial.trim();
    cmdSerial.toUpperCase();
    if (cmdSerial.length() > 0) {
      Serial.print(">> Manual: "); Serial.println(cmdSerial);
      procesarComando(cmdSerial);
    }
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
  Serial.print("[I2C] Recibido: "); Serial.println(cmd);
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
  else if (cmd == "ELEVADOR_PARAR") { 
    elevadorMoviendo = false; 
    pasosRestantes = 0; 
    Serial.println(">> Elevador PARADO");
  }
  else if (cmd == "ELEVADOR_STATUS") {
    Serial.print("Nivel: "); Serial.print(nivelActual);
    Serial.print(" | Pasos restantes: "); Serial.println(pasosRestantes);
  }
  else if (cmd == "CALIBRAR") { 
    Serial.println(">> Calibrando: bajando hasta piso 0...");
    nivelActual = 0; 
    direccionActual = -1; 
    pasosRestantes = 99999; 
    elevadorMoviendo = true; 
  }
}

void moverNiveles(int n) {
  int destino = nivelActual + n;
  if (destino < NIVEL_MIN) destino = NIVEL_MIN;
  if (destino > NIVEL_MAX) destino = NIVEL_MAX;
  if (destino == nivelActual) {
    Serial.println(">> Ya esta en ese nivel");
    return;
  }
  int niveles = destino - nivelActual;
  pasosRestantes = (long)abs(niveles) * PASOS_POR_NIVEL;
  direccionActual = (niveles > 0) ? 1 : -1;
  elevadorMoviendo = true;
  nivelActual = destino;
  
  Serial.print(">> Moviendo "); Serial.print(abs(niveles)); 
  Serial.print(" nivel(es) "); Serial.print(direccionActual > 0 ? "ARRIBA" : "ABAJO");
  Serial.print(" | Pasos: "); Serial.println(pasosRestantes);
}