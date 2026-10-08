// ============================================
// ESCLAVO - Control de Servo Puente Elevadizo
// ============================================
// - SUBIR puente  -> servo a 115 grados (abierto)
// - BAJAR puente  -> servo a 180 grados (cerrado)
// ============================================
#include <Wire.h>
#include <Servo.h>

#define I2C_ADDRESS     0x08
#define PIN_SERVO       9

// ====== ANGULOS DEL PUENTE ======
#define ANGULO_ABIERTO  115   // subir puente
#define ANGULO_CERRADO  180   // bajar puente
// ================================

#define ANGULO_MIN      0
#define ANGULO_MAX      180

Servo miServo;
int anguloActual = 90;
int anguloObjetivo = 90;
bool moviendo = false;

void setup() {
  Wire.begin(I2C_ADDRESS);
  Wire.onRequest(requestEvent);
  Wire.onReceive(receiveEvent);
  
  miServo.attach(PIN_SERVO);
  miServo.write(anguloActual);
  
  Serial.begin(9600);
  Serial.println("=== ESCLAVO LISTO ===");
  Serial.println("Direccion I2C: 0x08");
  Serial.print("Angulo inicial: ");
  Serial.println(anguloActual);
  Serial.println("");
  Serial.println("Comandos:");
  Serial.println("  SERVO_SUBIR  -> 115 (abrir puente)");
  Serial.println("  SERVO_BAJAR  -> 180 (cerrar puente)");
  Serial.println("  SERVO <ang>  -> angulo especifico");
  Serial.println("  SERVO_STATUS -> reportar angulo");
  Serial.println("  SERVO_STOP   -> detener");
  Serial.println("  SERVO_SWEEP  -> barrido 0-180-0");
}

void loop() {
  // Movimiento paso a paso (no bloqueante)
  if (moviendo) {
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
      moviendo = false;
      Serial.print(">> Detenido en: ");
      Serial.println(anguloActual);
    }
  }
  delay(5);
}

void requestEvent() {
  char buffer[16];
  snprintf(buffer, sizeof(buffer), "POS:%03d\n", anguloActual);
  Wire.write(buffer);
}

void receiveEvent(int howMany) {
  String cmd = "";
  while (Wire.available()) cmd += (char)Wire.read();
  cmd.trim();
  cmd.toUpperCase();
  
  if (cmd.length() == 0) return;
  
  Serial.print("[I2C] Recibido: ");
  Serial.println(cmd);
  
  if (cmd == "SERVO_SUBIR") {
    iniciarMovimiento(ANGULO_ABIERTO);   // 115
  }
  else if (cmd == "SERVO_BAJAR") {
    iniciarMovimiento(ANGULO_CERRADO);   // 180
  }
  else if (cmd.startsWith("SERVO ")) {
    int ang = cmd.substring(6).toInt();
    if (ang >= ANGULO_MIN && ang <= ANGULO_MAX) {
      iniciarMovimiento(ang);
    } else {
      Serial.println("Angulo fuera de rango");
    }
  }
  else if (cmd == "SERVO_STOP") {
    moviendo = false;
    Serial.print(">> STOP. Angulo: ");
    Serial.println(anguloActual);
  }
  else if (cmd == "SERVO_SWEEP") {
    barrido();
  }
  else if (cmd == "SERVO_STATUS") {
    Serial.print("Status: ");
    Serial.println(anguloActual);
  }
  else if (cmd == "PING") {
    Serial.println("PONG");
  }
  else {
    Serial.print("Desconocido: ");
    Serial.println(cmd);
  }
}

void iniciarMovimiento(int destino) {
  destino = constrain(destino, ANGULO_MIN, ANGULO_MAX);
  anguloObjetivo = destino;
  moviendo = true;
  Serial.print(">> Mover de ");
  Serial.print(anguloActual);
  Serial.print(" a ");
  Serial.println(anguloObjetivo);
}

void barrido() {
  moviendo = false;
  for (int a = anguloActual; a <= 185; a++) {
    miServo.write(a); anguloActual = a; delay(15);
  }
  delay(300);
  for (int a = 180; a >= 0; a--) {
    miServo.write(a); anguloActual = a; delay(15);
  }
  delay(300);
  for (int a = 0; a <= 90; a++) {
    miServo.write(a); anguloActual = a; delay(15);
  }
  Serial.println(">> Barrido completado, en 90");
}