#include <Wire.h>

// ---------- Configuración ----------
#define ESCLAVO1_ADDR 0x08
#define INTERVALO_ESCANEO 5000   // escanear bus I2C cada 5 s
#define INTERVALO_ENVIO_ESP32 5000

// ---------- Variables ----------
bool esclavo1Conectado = false;

// ---------- Utilidades I2C ----------
bool pingI2C(uint8_t addr) {
  Wire.beginTransmission(addr);
  return (Wire.endTransmission() == 0);
}

// ---------- Setup ----------
void setup() {
  Serial.begin(115200);        // Monitor USB
  Serial1.begin(9600);         // UART al ESP32
  Wire.begin();                // I2C como maestro (SDA=20, SCL=21 en Mega)

  delay(1500);
  Serial.println("=== Mega Gateway iniciado ===");
  Serial.println("Escaneando bus I2C...");
  escanearI2C();
}

// ---------- Escaneo I2C ----------
void escanearI2C() {
  Serial.println("Dispositivos I2C encontrados:");
  bool encontrado = false;

  for (uint8_t addr = 1; addr < 127; addr++) {
    if (pingI2C(addr)) {
      Serial.print("  -> 0x");
      if (addr < 16) Serial.print("0");
      Serial.print(addr, HEX);
      if (addr == ESCLAVO1_ADDR) Serial.print("  (Esclavo 1)");
      Serial.println();
      encontrado = true;
    }
  }
  if (!encontrado) Serial.println("  (ninguno)");
}

// ---------- Chequeo del esclavo 1 ----------
void chequearEsclavo1() {
  bool antes = esclavo1Conectado;
  esclavo1Conectado = pingI2C(ESCLAVO1_ADDR);

  if (esclavo1Conectado && !antes) {
    Serial.println("[I2C] Esclavo 1 CONECTADO (0x08)");
  } else if (!esclavo1Conectado && antes) {
    Serial.println("[I2C] Esclavo 1 DESCONECTADO (0x08)");
  } else if (!esclavo1Conectado) {
    Serial.println("[I2C] Esclavo 1 no responde (0x08)");
  }
}

// ---------- Envío al ESP32 ----------
void enviarEstadoESP32() {
  Serial1.print("ESCLAVO1=");
  Serial1.println(esclavo1Conectado ? "OK" : "ERROR");
}

// ---------- Loop ----------
void loop() {
  // 1) Escuchar al ESP32
  while (Serial1.available()) {
    String linea = Serial1.readStringUntil('\n');
    linea.trim();
    if (linea.length() > 0) {
      Serial.print("[ESP32] ");
      Serial.println(linea);
    }
  }

  // 2) Escanear bus I2C cada 5 s
  static unsigned long ultimoEscaneo = 0;
  if (millis() - ultimoEscaneo > INTERVALO_ESCANEO) {
    ultimoEscaneo = millis();
    escanearI2C();
    chequearEsclavo1();
  }

  // 3) Reportar estado al ESP32
  static unsigned long ultimoEnvio = 0;
  if (millis() - ultimoEnvio > INTERVALO_ENVIO_ESP32) {
    ultimoEnvio = millis();
    enviarEstadoESP32();
  }
}