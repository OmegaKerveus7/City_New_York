// ============================================
// ESCLAVO - Servo (Puente) + Stepper (Teleferico)
// ============================================
// Servo   -> Pin 6
// Stepper -> Pines 8, 9, 10, 11 (via ULN2003)
// I2C     -> Direccion 0x08
// ============================================
#include <Wire.h>
#include <Servo.h>
#include <Stepper.h>

// ====== I2C ======
#define I2C_ADDRESS     0x08

// ====== SERVO ======
#define PIN_SERVO       6
#define ANGULO_ABIERTO  115
#define ANGULO_CERRADO  180
#define ANGULO_MIN      0
#define ANGULO_MAX      180

// ====== STEPPER ======
#define PIN_STEP_IN1    8
#define PIN_STEP_IN2    9
#define PIN_STEP_IN3    10
#define PIN_STEP_IN4    11
#define STEPS_VUELTA    10500
#define VELOCIDAD_STEP  5    // RPM

// ====== OBJETOS ======
Servo miServo;
// Orden: IN1, IN3, IN2, IN4
Stepper miStepper(STEPS_VUELTA, PIN_STEP_IN1, PIN_STEP_IN3, PIN_STEP_IN2, PIN_STEP_IN4);

// ====== ESTADO ======
int anguloActual = 90;
int anguloObjetivo = 90;
bool moviendoServo = false;

bool telefericoMoviendo = false;
int pasosTeleferico = 0;
int pasosRestantes = 0;

// ============================================
// SETUP
// ============================================
void setup() {
  // I2C
  Wire.begin(I2C_ADDRESS);
  Wire.onRequest(requestEvent);
  Wire.onReceive(receiveEvent);
  
  // Servo
  miServo.attach(PIN_SERVO);
  miServo.write(anguloActual);
  
  // Stepper
  miStepper.setSpeed(VELOCIDAD_STEP);
  
  // Serial
  Serial.begin(9600);
  Serial.println("=== ESCLAVO LISTO ===");
  Serial.println("I2C: 0x08");
  Serial.print("Servo pin: ");
  Serial.println(PIN_SERVO);
  Serial.print("Stepper IN1-IN4: ");
  Serial.print(PIN_STEP_IN1); Serial.print(", ");
  Serial.print(PIN_STEP_IN2); Serial.print(", ");
  Serial.print(PIN_STEP_IN3); Serial.print(", ");
  Serial.println(PIN_STEP_IN4);
  Serial.println("");
  Serial.println("=== COMANDOS ===");
  Serial.println("PUENTE:");
  Serial.println("  SERVO_SUBIR          -> 115 (abrir)");
  Serial.println("  SERVO_BAJAR          -> 180 (cerrar)");
  Serial.println("  SERVO <ang>          -> angulo especifico");
  Serial.println("  SERVO_STOP           -> detener servo");
  Serial.println("  SERVO_STATUS         -> reportar angulo");
  Serial.println("");
  Serial.println("TELEFERICO:");
  Serial.println("  TELEFERICO_AVANZAR   -> avanza 1 vuelta");
  Serial.println("  TELEFERICO_AVANZAR N -> avanza N pasos");
  Serial.println("  TELEFERICO_REGRESAR  -> retrocede 1 vuelta");
  Serial.println("  TELEFERICO_REGRESAR N-> retrocede N pasos");
  Serial.println("  TELEFERICO_PARAR     -> detener motor");
  Serial.println("  TELEFERICO_STATUS    -> reportar estado");
  Serial.println("");
  Serial.println("GENERAL:");
  Serial.println("  PING                 -> PONG");
}

// ============================================
// LOOP
// ============================================
void loop() {
  // --- Servo paso a paso (no bloqueante) ---
  if (moviendoServo) {
    if (anguloActual < anguloObjetivo) {
      anguloActual++;
      miServo.write(anguloActual);
      delay(20);
    } 
    else if (anguloActual > anguloObjetivo) {
      anguloActual--;
      miServo.write(anguloActual);
      delay(20);
    } 
    else {
      moviendoServo = false;
      Serial.print(">> Servo detenido en: ");
      Serial.println(anguloActual);
    }
  }
  
  // --- Stepper paso a paso (no bloqueante) ---
  if (telefericoMoviendo && pasosRestantes > 0) {
    int direccion = (pasosTeleferico > 0) ? 1 : -1;
    miStepper.step(direccion);
    pasosRestantes--;
    
    if (pasosRestantes == 0) {
      telefericoMoviendo = false;
      Serial.println(">> Teleferico detenido");
    }
  }
  
  // --- Comandos manuales por Monitor Serie (para pruebas) ---
  if (Serial.available()) {
    String cmdSerial = Serial.readStringUntil('\n');
    cmdSerial.trim();
    cmdSerial.toUpperCase();
    if (cmdSerial.length() > 0) {
      Serial.print(">> Manual: ");
      Serial.println(cmdSerial);
      procesarComando(cmdSerial);
    }
  }
  
  delay(2);
}

// ============================================
// I2C - Responder al Mega (cuando pide POS)
// ============================================
void requestEvent() {
  char buffer[16];
  snprintf(buffer, sizeof(buffer), "POS:%03d\n", anguloActual);
  Wire.write(buffer);
}

// ============================================
// I2C - Recibir comando del Mega
// ============================================
void receiveEvent(int howMany) {
  String cmd = "";
  while (Wire.available()) cmd += (char)Wire.read();
  cmd.trim();
  cmd.toUpperCase();
  
  if (cmd.length() == 0) return;
  
  Serial.print("[I2C] Recibido: ");
  Serial.println(cmd);
  
  procesarComando(cmd);
}

// ============================================
// Procesar comandos (unificado)
// ============================================
void procesarComando(String cmd) {
  
  // ========== SERVO (PUENTE) ==========
  if (cmd == "SERVO_SUBIR") {
    iniciarMovimientoServo(ANGULO_ABIERTO);
  }
  else if (cmd == "SERVO_BAJAR") {
    iniciarMovimientoServo(ANGULO_CERRADO);
  }
  else if (cmd.startsWith("SERVO ") && !cmd.startsWith("SERVO_S")) {
    int ang = cmd.substring(6).toInt();
    if (ang >= ANGULO_MIN && ang <= ANGULO_MAX) {
      iniciarMovimientoServo(ang);
    } else {
      Serial.println("Angulo fuera de rango");
    }
  }
  else if (cmd == "SERVO_STOP") {
    moviendoServo = false;
    Serial.print(">> Servo STOP. Angulo: ");
    Serial.println(anguloActual);
  }
  else if (cmd == "SERVO_STATUS") {
    Serial.print("Servo status: ");
    Serial.println(anguloActual);
  }
  
  // ========== STEPPER (TELEFERICO) ==========
  else if (cmd == "TELEFERICO_AVANZAR") {
    iniciarTeleferico(STEPS_VUELTA);
  }
  else if (cmd.startsWith("TELEFERICO_AVANZAR ")) {
    int pasos = cmd.substring(19).toInt();
    if (pasos > 0) iniciarTeleferico(pasos);
    else Serial.println("Pasos invalidos");
  }
  else if (cmd == "TELEFERICO_REGRESAR") {
    iniciarTeleferico(-STEPS_VUELTA);
  }
  else if (cmd.startsWith("TELEFERICO_REGRESAR ")) {
    int pasos = cmd.substring(20).toInt();
    if (pasos > 0) iniciarTeleferico(-pasos);
    else Serial.println("Pasos invalidos");
  }
  else if (cmd == "TELEFERICO_PARAR") {
    telefericoMoviendo = false;
    pasosRestantes = 0;
    Serial.println(">> Teleferico PARADO");
  }
  else if (cmd == "TELEFERICO_STATUS") {
    Serial.print("Teleferico: ");
    Serial.print(telefericoMoviendo ? "MOVIENDO" : "DETENIDO");
    Serial.print(" | pasos restantes: ");
    Serial.println(pasosRestantes);
  }
  
  // ========== GENERAL ==========
  else if (cmd == "PING") {
    Serial.println("PONG");
  }
  else {
    Serial.print("Desconocido: ");
    Serial.println(cmd);
  }
}

// ============================================
// Iniciar movimiento del servo
// ============================================
void iniciarMovimientoServo(int destino) {
  destino = constrain(destino, ANGULO_MIN, ANGULO_MAX);
  anguloObjetivo = destino;
  moviendoServo = true;
  Serial.print(">> Servo: ");
  Serial.print(anguloActual);
  Serial.print(" -> ");
  Serial.println(anguloObjetivo);
}

// ============================================
// Iniciar movimiento del teleferico (stepper)
// ============================================
void iniciarTeleferico(int pasos) {
  pasosTeleferico = pasos;
  pasosRestantes = abs(pasos);
  
  if (pasos > 0) {
    Serial.print(">> Teleferico AVANZAR ");
    Serial.print(pasos);
    Serial.println(" pasos");
  } else {
    Serial.print(">> Teleferico REGRESAR ");
    Serial.print(abs(pasos));
    Serial.println(" pasos");
  }
  
  telefericoMoviendo = true;
}