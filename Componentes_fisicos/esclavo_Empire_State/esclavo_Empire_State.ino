// ============================================
// ESCLAVO - ELEVADOR con Stepper (I2C) + LED
// ============================================
// Stepper  -> Pines 8, 9, 10, 11 (via ULN2003)
// FinCarrera Piso0 -> Pin 12 (opcional)
// FinCarrera PisoMax -> Pin 13 (opcional)
// LED      -> Pin 4 (parpadea cada 1 s)
// I2C      -> Direccion 0x09
// ============================================
#include <Wire.h>
#include <Stepper.h>

// ====== I2C ======
#define I2C_ADDRESS     0x09

// ====== STEPPER ======
#define PIN_STEP_IN1    8
#define PIN_STEP_IN2    9
#define PIN_STEP_IN3    10
#define PIN_STEP_IN4    11
#define STEPS_POR_VUELTA 2048   // 28BYJ-48 típico. Ajusta si usas otro.
#define VELOCIDAD_STEP  10      // RPM

// ====== FINES DE CARRERA (opcional) ======
#define PIN_FC_PISO0    12
#define PIN_FC_PISO_MAX 13

// ====== LED ======
#define PIN_LED         3
#define INTERVALO_LED   1000    // 1 segundo (ON 1s, OFF 1s)

// ====== CONFIG ELEVADOR ======
#define PASOS_POR_NIVEL 5000    // Pasos por cada piso
#define NIVEL_MIN       0
#define NIVEL_MAX       5       // 6 pisos: 0..5

// ====== OBJETOS ======
// Orden: IN1, IN3, IN2, IN4 (importante para ULN2003)
Stepper miStepper(STEPS_POR_VUELTA, PIN_STEP_IN1, PIN_STEP_IN3, PIN_STEP_IN2, PIN_STEP_IN4);

// ====== ESTADO ======
int nivelActual = 0;            // Piso actual (0..NIVEL_MAX)
long pasosRestantes = 0;        // Pasos que faltan por ejecutar
int  direccionActual = 1;       // 1 = subir, -1 = bajar
bool elevadorMoviendo = false;
bool calibrado = false;

// LED
bool estadoLed = LOW;
unsigned long ultimoCambioLed = 0;

// ============================================
// SETUP
// ============================================
void setup() {
  Wire.begin(I2C_ADDRESS);
  Wire.onRequest(requestEvent);
  Wire.onReceive(receiveEvent);

  miStepper.setSpeed(VELOCIDAD_STEP);

  pinMode(PIN_FC_PISO0, INPUT_PULLUP);
  pinMode(PIN_FC_PISO_MAX, INPUT_PULLUP);

  // LED
  pinMode(PIN_LED, OUTPUT);
  digitalWrite(PIN_LED, LOW);

  Serial.begin(9600);
  Serial.println("=== ESCLAVO ELEVADOR LISTO ===");
  Serial.print("I2C: 0x"); Serial.println(I2C_ADDRESS, HEX);
  Serial.print("Pasos por nivel: "); Serial.println(PASOS_POR_NIVEL);
  Serial.print("LED en pin: "); Serial.println(PIN_LED);
  Serial.println("Comandos:");
  Serial.println("  SUBIR_NIVEL n     -> sube n pisos");
  Serial.println("  BAJAR_NIVEL n     -> baja n pisos");
  Serial.println("  IR_A_NIVEL n      -> va al piso n");
  Serial.println("  ELEVADOR_PARAR    -> detener");
  Serial.println("  ELEVADOR_STATUS   -> reportar estado");
  Serial.println("  CALIBRAR          -> bajar hasta piso 0");
  Serial.println("  PING              -> PONG");
}

// ============================================
// LOOP
// ============================================
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
      calibrado = true;
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

  // --- Comandos manuales por Serial (para pruebas) ---
  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();
    cmd.toUpperCase();
    if (cmd.length() > 0) {
      Serial.print(">> Manual: "); Serial.println(cmd);
      procesarComando(cmd);
    }
  }

  delay(1);
}

// ============================================
// I2C - Responder al Mega
// ============================================
void requestEvent() {
  char buffer[32];
  // Formato: NIVEL:03|PAS:xxxxx\n
  snprintf(buffer, sizeof(buffer), "NIVEL:%02d|PAS:%05ld\n",
           nivelActual, pasosRestantes);
  Wire.write((uint8_t*)buffer, strlen(buffer));
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

  Serial.print("[I2C] Recibido: "); Serial.println(cmd);
  procesarComando(cmd);
}

// ============================================
// Procesar comandos
// ============================================
void procesarComando(String cmd) {

  // ===== PING =====
  if (cmd == "PING") {
    Serial.println("PONG");
    return;
  }

  // ===== SUBIR_NIVEL n =====
  if (cmd.startsWith("SUBIR_NIVEL")) {
    int n = 1;
    int idx = cmd.indexOf(' ');
    if (idx > 0) n = cmd.substring(idx + 1).toInt();
    if (n <= 0) { Serial.println("Niveles invalidos"); return; }
    moverNiveles(+n);
    return;
  }

  // ===== BAJAR_NIVEL n =====
  if (cmd.startsWith("BAJAR_NIVEL")) {
    int n = 1;
    int idx = cmd.indexOf(' ');
    if (idx > 0) n = cmd.substring(idx + 1).toInt();
    if (n <= 0) { Serial.println("Niveles invalidos"); return; }
    moverNiveles(-n);
    return;
  }

  // ===== IR_A_NIVEL n =====
  if (cmd.startsWith("IR_A_NIVEL")) {
    int idx = cmd.indexOf(' ');
    if (idx < 0) { Serial.println("Falta nivel"); return; }
    int destino = cmd.substring(idx + 1).toInt();
    if (destino < NIVEL_MIN || destino > NIVEL_MAX) {
      Serial.println("Nivel fuera de rango");
      return;
    }
    int delta = destino - nivelActual;
    if (delta == 0) { Serial.println("Ya esta en ese nivel"); return; }
    moverNiveles(delta);
    return;
  }

  // ===== ELEVADOR_PARAR =====
  if (cmd == "ELEVADOR_PARAR") {
    elevadorMoviendo = false;
    pasosRestantes = 0;
    Serial.println(">> Elevador PARADO");
    return;
  }

  // ===== ELEVADOR_STATUS =====
  if (cmd == "ELEVADOR_STATUS") {
    Serial.print("Nivel: "); Serial.print(nivelActual);
    Serial.print(" | Pasos restantes: "); Serial.println(pasosRestantes);
    return;
  }

  // ===== CALIBRAR =====
  if (cmd == "CALIBRAR") {
    Serial.println(">> Calibrando: bajando hasta piso 0...");
    nivelActual = 0;
    direccionActual = -1;
    pasosRestantes = 99999; // se detiene con fin de carrera
    elevadorMoviendo = true;
    return;
  }

  Serial.print("Desconocido: "); Serial.println(cmd);
}

// ============================================
// Mover N niveles (positivo=subir, negativo=bajar)
// ============================================
void moverNiveles(int n) {
  int destino = nivelActual + n;
  if (destino < NIVEL_MIN) {
    Serial.println(">> Limitado a piso minimo");
    destino = NIVEL_MIN;
  }
  if (destino > NIVEL_MAX) {
    Serial.println(">> Limitado a piso maximo");
    destino = NIVEL_MAX;
  }
  if (destino == nivelActual) {
    Serial.println(">> Ya esta en ese nivel");
    return;
  }

  int niveles = destino - nivelActual;
  pasosRestantes = (long)abs(niveles) * PASOS_POR_NIVEL;
  direccionActual = (niveles > 0) ? 1 : -1;
  elevadorMoviendo = true;

  Serial.print(">> Moviendo ");
  Serial.print(abs(niveles));
  Serial.print(" nivel(es) ");
  Serial.print(direccionActual > 0 ? "ARRIBA" : "ABAJO");
  Serial.print(" | Pasos: ");
  Serial.println(pasosRestantes);

  // Actualizamos nivelActual al vuelo
  nivelActual = destino;
}