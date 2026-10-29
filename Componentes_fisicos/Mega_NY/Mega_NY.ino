#include <Wire.h>

#define ESCLAVO1_ADDR 0x08 // Puente (Servo)
#define ESCLAVO2_ADDR 0x09 // Elevador (Stepper)
#define PIN_LED_MEGA  13

bool esclavo1Online = false;
bool esclavo2Online = false;
unsigned long ciclosMega = 0; 

// ==========================================
// PINES DE LOS SEMÁFOROS
// ==========================================
const int pinRojo1     = 22;
const int pinAmarillo1 = 24;
const int pinVerde1    = 26;

const int pinRojo2     = 23;
const int pinAmarillo2 = 27;
const int pinVerde2    = 25; 

unsigned long tiempoSemaforo = 0;
int faseSemaforo = 1;
int faseAnterior = 0;

void setup() {
  Serial.begin(115200);       // Monitor USB de la PC (Ponlo en 115200 en el IDE)
  Serial1.begin(9600);        // Hacia el ESP32 (Pines 18 y 19) - 9600 es seguro
  Wire.begin();               // I2C Maestro (Pines 20 y 21)
  
  pinMode(PIN_LED_MEGA, OUTPUT);
  digitalWrite(PIN_LED_MEGA, LOW);

  pinMode(pinRojo1, OUTPUT); pinMode(pinAmarillo1, OUTPUT); pinMode(pinVerde1, OUTPUT);
  pinMode(pinRojo2, OUTPUT); pinMode(pinAmarillo2, OUTPUT); pinMode(pinVerde2, OUTPUT);

  apagarSemaforos();

  delay(1000);
  Serial.println("=== MEGA CONTROL MAESTRO y SEMAFOROS ===");
  Serial.println("Iniciando busqueda de esclavos I2C...");
  
  chequearEsclavos();
  tiempoSemaforo = millis();
}

void loop() {
  // 1. Escuchar al ESP32
  if (Serial1.available()) {
    String cmd = Serial1.readStringUntil('\n');
    cmd.trim();          
    cmd.toUpperCase();   
    
    if (cmd.length() > 0) {
      Serial.print("[ESP32] ");
      Serial.println(cmd);
      procesarComandoESP32(cmd);
    }
  }

  // 2. Chequear esclavos cada 5 segundos
  static unsigned long ultimoChequeo = 0;
  if (millis() - ultimoChequeo > 5000) {
    ultimoChequeo = millis();
    chequearEsclavos();
  }

  // 3. Control de Semáforos
  actualizarSemaforos();
}

// ============================================
// PROCESAR COMANDOS DEL ESP32
// ============================================
void procesarComandoESP32(String cmd) {
  
  if (cmd == "PING") {
    ciclosMega++; 
    Serial1.print("CICLO_MEGA:");
    Serial1.println(ciclosMega);
    return;
  }

  if (cmd == "SEMAFORO_STATUS") {
    enviarEstadoSemaforosESP32(faseSemaforo);
    return;
  }

  // --- PUENTE ---
  if (cmd == "SERVO_SUBIR" || cmd == "SERVO_BAJAR" || cmd == "SERVO_STATUS" || cmd.startsWith("SERVO ")) {
    if (!esclavo1Online) {
      Serial1.println("RESP ERROR:ESCLAVO1_OFFLINE");
      return;
    }
    enviarI2C(ESCLAVO1_ADDR, cmd);
    return;
  }

  // --- ELEVADOR ---
  if (cmd.startsWith("SUBIR_NIVEL") || cmd.startsWith("BAJAR_NIVEL") || cmd.startsWith("IR_A_NIVEL") || 
      cmd == "ELEVADOR_PARAR" || cmd == "ELEVADOR_STATUS" || cmd == "CALIBRAR") {
    if (!esclavo2Online) {
      Serial1.println("RESP ERROR:ESCLAVO2_OFFLINE");
      return;
    }
    enviarI2C(ESCLAVO2_ADDR, cmd);
    return;
  }

  // --- TELEFERICO ---
  if (cmd.startsWith("TELEFERICO_AVANZAR") || cmd.startsWith("TELEFERICO_REGRESAR") || 
      cmd == "TELEFERICO_PARAR" || cmd == "TELEFERICO_STATUS") {
    if (!esclavo1Online) {
      Serial1.println("RESP ERROR:ESCLAVO1_OFFLINE");
      return;
    }
    enviarI2C(ESCLAVO1_ADDR, cmd);
    return;
  }

  Serial.print(">> Comando desconocido: [");
  Serial.print(cmd);
  Serial.println("]");
  Serial1.println("RESP ERROR:CMD_DESCONOCIDO");
}

// ============================================
// ENVIAR Y LEER POR I2C (SIN BLOQUEOS)
// ============================================
void enviarI2C(uint8_t direccion, String cmd) {
  Wire.beginTransmission(direccion);
  Wire.write(cmd.c_str());
  byte err = Wire.endTransmission();

  if (err != 0) {
    Serial1.println("RESP ERROR:I2C_WRITE");
    return;
  }

  unsigned long inicio = millis();
  while (millis() - inicio < 50) {
    if (Serial1.available()) {
      String extra = Serial1.readStringUntil('\n');
      extra.trim();
    }
    yield();
  }

  Wire.requestFrom(direccion, 32);
  String resp = "";
  while (Wire.available()) {
    char c = Wire.read();
    if (c == '\n') break; 
    resp += c;
  }
  resp.trim();

  if (resp.length() > 0) {
    Serial1.print("RESP ");
    Serial1.println(resp);
  } else {
    Serial1.println("RESP ERROR:SIN_RESPUESTA");
  }
}

bool pingI2C(uint8_t addr) {
  Wire.beginTransmission(addr);
  return (Wire.endTransmission() == 0);
}

// ============================================
// CHEQUEAR ESCLAVOS (MODIFICADO PARA VER EN PANTALLA)
// ============================================
void chequearEsclavos() {
  bool e1 = pingI2C(ESCLAVO1_ADDR);
  bool e2 = pingI2C(ESCLAVO2_ADDR);

  // Solo enviamos el aviso al ESP32 si hay un cambio real de estado
  if (e1 != esclavo1Online) {
    esclavo1Online = e1;
    Serial1.print("ESCLAVO1=");
    Serial1.println(esclavo1Online ? "OK" : "ERROR");
  }

  if (e2 != esclavo2Online) {
    esclavo2Online = e2;
    Serial1.print("ESCLAVO2=");
    Serial1.println(esclavo2Online ? "OK" : "ERROR");
  }

  // SIEMPRE imprimimos en el Monitor Serie del Mega para saber qué pasa
  Serial.print("[I2C] Esclavo 1 (0x08): ");
  Serial.print(e1 ? "OK" : "FALLO");
  Serial.print(" | Esclavo 2 (0x09): ");
  Serial.println(e2 ? "OK" : "FALLO");
}

// --- FUNCIONES DE SEMÁFOROS ---
void actualizarSemaforos() {
  unsigned long tiempoActual = millis();
  unsigned long tiempoTranscurrido = tiempoActual - tiempoSemaforo;

  switch (faseSemaforo) {
    case 1: if (tiempoTranscurrido >= 150000) { faseSemaforo = 2; tiempoSemaforo = tiempoActual; } break;
    case 2: if (tiempoTranscurrido >= 30000) { faseSemaforo = 3; tiempoSemaforo = tiempoActual; } break;
    case 3: if (tiempoTranscurrido >= 150000) { faseSemaforo = 4; tiempoSemaforo = tiempoActual; } break;
    case 4: if (tiempoTranscurrido >= 30000) { faseSemaforo = 1; tiempoSemaforo = tiempoActual; } break;
  }

  if (faseSemaforo != faseAnterior) {
    faseAnterior = faseSemaforo;
    aplicarEstadoSemaforos(faseSemaforo);
    enviarEstadoSemaforosESP32(faseSemaforo);
  }
}

void aplicarEstadoSemaforos(int fase) {
  if (fase == 1) {
    digitalWrite(pinRojo1, LOW); digitalWrite(pinAmarillo1, LOW); digitalWrite(pinVerde1, HIGH);
    digitalWrite(pinRojo2, HIGH); digitalWrite(pinAmarillo2, LOW); digitalWrite(pinVerde2, LOW);
  } else if (fase == 2) {
    digitalWrite(pinRojo1, LOW); digitalWrite(pinAmarillo1, HIGH); digitalWrite(pinVerde1, LOW);
    digitalWrite(pinRojo2, HIGH); digitalWrite(pinAmarillo2, LOW); digitalWrite(pinVerde2, LOW);
  } else if (fase == 3) {
    digitalWrite(pinRojo1, HIGH); digitalWrite(pinAmarillo1, LOW); digitalWrite(pinVerde1, LOW);
    digitalWrite(pinRojo2, LOW); digitalWrite(pinAmarillo2, LOW); digitalWrite(pinVerde2, HIGH);
  } else if (fase == 4) {
    digitalWrite(pinRojo1, HIGH); digitalWrite(pinAmarillo1, LOW); digitalWrite(pinVerde1, LOW);
    digitalWrite(pinRojo2, LOW); digitalWrite(pinAmarillo2, HIGH); digitalWrite(pinVerde2, LOW);
  }
}

void apagarSemaforos() {
  digitalWrite(pinRojo1, LOW); digitalWrite(pinAmarillo1, LOW); digitalWrite(pinVerde1, LOW);
  digitalWrite(pinRojo2, LOW); digitalWrite(pinAmarillo2, LOW); digitalWrite(pinVerde2, LOW);
}

void enviarEstadoSemaforosESP32(int fase) {
  String msg = "SEMAFORO:FASE_";
  msg += String(fase);
  Serial1.println(msg);
}