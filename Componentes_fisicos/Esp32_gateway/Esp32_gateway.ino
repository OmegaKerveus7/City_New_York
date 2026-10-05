#include <WiFi.h>

// ---------- Configuración UART ----------
#define RXD2 16
#define TXD2 17

// ---------- WiFi ----------
const char* ssid     = "Omega";
const char* password = "d5adc4a32689";

// ---------- Estado ----------
bool megaOnline = false;
unsigned long ultimoMensajeMega = 0;
const unsigned long TIMEOUT_MEGA = 15000;  // si no hay msg en 15 s, Mega offline

void setup() {
  Serial.begin(115200);
  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);

  Serial.println("=== ESP32 Gateway iniciado ===");

  conectarWiFi();

  Serial.println("Esperando mensajes del Mega...");
}

void loop() {
  // 1) Reconectar WiFi si se cae
  if (WiFi.status() != WL_CONNECTED) {
    conectarWiFi();
  }

  // 2) Leer del Mega
  if (Serial2.available()) {
    String linea = Serial2.readStringUntil('\n');
    linea.trim();

    if (linea.length() > 0) {
      ultimoMensajeMega = millis();
      megaOnline = true;

      Serial.print("[Mega] ");
      Serial.println(linea);

      procesarMensajeMega(linea);
    }
  }

  // 3) Detectar si el Mega se cayó
  if (megaOnline && (millis() - ultimoMensajeMega > TIMEOUT_MEGA)) {
    megaOnline = false;
    Serial.println("[ALERTA] Mega no responde");
  }
}

// ---------- WiFi ----------
void conectarWiFi() {
  Serial.print("Conectando a WiFi: ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  unsigned long inicio = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - inicio < 15000) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("WiFi OK. IP: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("WiFi FALLÓ, reintentando...");
  }
}

// ---------- Procesar mensajes del Mega ----------
void procesarMensajeMega(String msg) {
  // Formato esperado: "ESCLAVO1=OK" o "ESCLAVO1=ERROR"
  if (msg.startsWith("ESCLAVO1=")) {
    String estado = msg.substring(9);
    if (estado == "OK") {
      Serial.println(">> Esclavo 1 disponible");
    } else {
      Serial.println(">> Esclavo 1 NO disponible");
    }
    // Acá después podés publicar a MQTT, HTTP, etc.
  }
}