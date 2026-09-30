#include <WiFi.h>
#include <HTTPClient.h>

// ====== CONFIGURACIÓN ======
const char* ssid     = "CASA_TR_2.4G";
const char* password = "5DCD951DC7";

// URL de tu API
const char* apiUrl = "https://warless-predestinately-bethann.ngrok-free.dev/api/Test/conexion";

// ===========================

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("\n=== ESP32 API Test ===");
  Serial.print("Conectando a WiFi");
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi conectado");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    consultarAPI();
  } else {
    Serial.println("WiFi desconectado, reintentando...");
    WiFi.reconnect();
  }

  delay(10000); // consulta cada 10 segundos
}

void consultarAPI() {
  HTTPClient http;

  Serial.println("\n--- Consultando API ---");
  Serial.println(apiUrl);

  http.begin(apiUrl);

  // Headers IMPORTANTES para saltar la advertencia de ngrok free
  http.addHeader("ngrok-skip-browser-warning", "true");
  http.addHeader("User-Agent", "ESP32-Client");
  http.addHeader("Accept", "application/json");

  int httpCode = http.GET();

  Serial.print("Código HTTP: ");
  Serial.println(httpCode);

  if (httpCode > 0) {
    String payload = http.getString();
    Serial.println("Respuesta:");
    Serial.println(payload);
  } else {
    Serial.print("Error en la petición: ");
    Serial.println(http.errorToString(httpCode).c_str());
  }

  http.end();
}