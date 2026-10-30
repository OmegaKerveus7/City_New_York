// ============================================
// ESCLAVO 3 - CENTRAL PARK (Fuente + Luces del jardín)
// ============================================
// Relé de la bomba (fuente) -> Pin 2  (activo en BAJO)
// Sensor de nivel (flotador) -> Pin 8  (INPUT_PULLUP, LOW = hay agua)
// Relé de las luces del jardín -> Pin 9  (activo en BAJO)
// Sensor de nivel analógico  -> A0     (0-100 %)
// LED de estado              -> LED_BUILTIN
// I2C                        -> Dirección 0x0A
// ============================================
#include <Wire.h>

#define I2C_ADDRESS 0x0A
#define PIN_RELE    2      // Relé de la bomba de la fuente
#define PIN_SENSOR_AGUA 8  // Flotador del tanque
#define PIN_RELE_LUCES 9   // Relé de las luces del jardín
#define PIN_SENSOR_ANALOGO A0

// ¡IMPORTANTE! Ponlo en true para probar sin el sensor de agua físico
#define SIMULAR_SENSOR_AGUA true 

// Módulos de relé: activos en BAJO (cambia a HIGH si tu relé es diferente)
#define RELE_ON  LOW
#define RELE_OFF HIGH

#define INTERVALO_LED 1000
#define PULSO_MAX_SEG 600

bool motorEncendido = false;
bool lucesEncendidas = false;
bool sensorAguaDetecta = false;
bool enPulso = false;
unsigned long finPulso = 0;

bool estadoLed = LOW;
unsigned long ultimoCambioLed = 0;

void setup() {
  Wire.begin(I2C_ADDRESS);
  Wire.onRequest(requestEvent);
  Wire.onReceive(receiveEvent);
  
  pinMode(PIN_RELE, OUTPUT);
  digitalWrite(PIN_RELE, RELE_OFF); // Apagado inicial

  pinMode(PIN_RELE_LUCES, OUTPUT);
  digitalWrite(PIN_RELE_LUCES, RELE_OFF);

  pinMode(PIN_SENSOR_AGUA, INPUT_PULLUP);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  
  Serial.begin(9600);
  delay(1500); // Espera para que el Monitor Serie abra bien
  
  // --- MENÚ DE OPCIONES EN PANTALLA ---
  Serial.println("");
  Serial.println("=== ESCLAVO 3 (CENTRAL PARK) LISTO ===");
  Serial.print("I2C: 0x"); Serial.println(I2C_ADDRESS, HEX);
  Serial.print("Relé en Pin: "); Serial.println(PIN_RELE);
  Serial.print("Sensor de agua en Pin: "); Serial.println(PIN_SENSOR_AGUA);
  Serial.print("Relé de luces en Pin: "); Serial.println(PIN_RELE_LUCES);
  Serial.println("Comandos disponibles:");
  Serial.println("  CENTRAL_ON  -> Enciende la fuente/motor");
  Serial.println("  CENTRAL_OFF -> Apaga la fuente/motor");
  Serial.println("  FUENTE_ENCENDER   -> Enciende la fuente");
  Serial.println("  FUENTE_APAGAR     -> Apaga la fuente");
  Serial.println("  FUENTE_PULSO n    -> Enciende n segundos y se apaga");
  Serial.println("  FUENTE_STATUS     -> Reporta el estado");
  Serial.println("  JARDIN_LUCES_ON   -> Enciende las luces del jardín");
  Serial.println("  JARDIN_LUCES_OFF  -> Apaga las luces del jardín");
  Serial.println("=======================================");
}

void loop() {
  // --- Fin del pulso temporizado ---
  if (enPulso && millis() >= finPulso) {
    enPulso = false;
    apagarFuente("fin del pulso");
  }

  // --- Lectura del flotador ---
  sensorAguaDetecta = leerSensorAgua();

  // --- LED de estado (parpadea con la fuente encendida) ---
  if (millis() - ultimoCambioLed >= INTERVALO_LED) {
    ultimoCambioLed = millis();
    if (motorEncendido || enPulso) {
      estadoLed = !estadoLed;
    } else {
      estadoLed = LOW;
    }
    digitalWrite(LED_BUILTIN, estadoLed);
  }
}

void receiveEvent(int howMany) {
  String cmd = "";
  while (Wire.available()) cmd += (char)Wire.read();
  cmd.trim();
  cmd.toUpperCase();
  
  if (cmd.length() == 0) return;

  Serial.print("[I2C] Recibido: ");
  Serial.println(cmd);

  if (cmd == "CENTRAL_ON" || cmd == "ENCENDER" || cmd == "FUENTE_ENCENDER") {
    if (!sensorAguaDetecta) {
      Serial.println(">> ERROR: sin agua, la bomba queda bloqueada por seguridad");
      return;
    }
    enPulso = false;
    encenderFuente();
  } 
  else if (cmd == "CENTRAL_OFF" || cmd == "APAGAR" || cmd == "FUENTE_APAGAR") {
    enPulso = false;
    apagarFuente("comando de apagado");
  }
  else if (cmd.startsWith("FUENTE_PULSO")) {
    int seg = 0;
    int idx = cmd.indexOf(' ');
    if (idx > 0) seg = cmd.substring(idx + 1).toInt();
    if (seg <= 0 || seg > PULSO_MAX_SEG) {
      Serial.println(">> ERROR: segundos de pulso inválidos");
      return;
    }
    if (!sensorAguaDetecta) {
      Serial.println(">> ERROR: sin agua, la bomba queda bloqueada por seguridad");
      return;
    }
    encenderFuente();
    enPulso = true;
    finPulso = millis() + (unsigned long)seg * 1000UL;
    Serial.print(">> Pulso de ");
    Serial.print(seg);
    Serial.println(" segundos");
  }
  else if (cmd == "FUENTE_STATUS") {
    Serial.print("Fuente: ");
    Serial.print(motorEncendido ? "ON" : "OFF");
    Serial.print(" | Agua: ");
    Serial.print(porcentajeAgua());
    Serial.print("% | Flotador: ");
    Serial.print(sensorAguaDetecta ? "CON AGUA" : "SECO");
    Serial.print(" | Luces: ");
    Serial.println(lucesEncendidas ? "ON" : "OFF");
  }
  else if (cmd == "JARDIN_LUCES_ON") {
    lucesEncendidas = true;
    digitalWrite(PIN_RELE_LUCES, RELE_ON);
    Serial.println(">> Luces del jardín ENCENDIDAS");
  }
  else if (cmd == "JARDIN_LUCES_OFF") {
    lucesEncendidas = false;
    digitalWrite(PIN_RELE_LUCES, RELE_OFF);
    Serial.println(">> Luces del jardín APAGADAS");
  }
  else if (cmd == "PING") {
    Serial.println("PONG");
  }
}

void requestEvent() {
  char buffer[48];
  // Formato: FUENTE:ON|AGUA:42|LUCES:OFF|FLOT:1
  snprintf(buffer, sizeof(buffer), "FUENTE:%s|AGUA:%02d|LUCES:%s|FLOT:%d\n",
           motorEncendido ? "ON" : "OFF",
           porcentajeAgua(),
           lucesEncendidas ? "ON" : "OFF",
           sensorAguaDetecta ? 1 : 0);
  Wire.write((uint8_t*)buffer, strlen(buffer));
}

// ============================================
// Fuente y luces
// ============================================
void encenderFuente() {
  motorEncendido = true;
  digitalWrite(PIN_RELE, RELE_ON);
  Serial.println(">> Motor ENCENDIDO");
}

void apagarFuente(String motivo) {
  motorEncendido = false;
  digitalWrite(PIN_RELE, RELE_OFF);
  Serial.print(">> Motor APAGADO (");
  Serial.print(motivo);
  Serial.println(")");
}

// ============================================
// Sensores
// ============================================
bool leerSensorAgua() {
  if (SIMULAR_SENSOR_AGUA) return true;
  // INPUT_PULLUP: LOW = circuito cerrado = hay agua
  return digitalRead(PIN_SENSOR_AGUA) == LOW;
}

int porcentajeAgua() {
  int lectura = analogRead(PIN_SENSOR_ANALOGO);
  int porcentaje = (lectura * 100) / 1023;
  if (porcentaje < 0) porcentaje = 0;
  if (porcentaje > 100) porcentaje = 100;
  return porcentaje;
}