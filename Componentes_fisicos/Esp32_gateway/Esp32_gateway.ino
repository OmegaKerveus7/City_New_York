// =====================================================
// ESP32 Gateway - Puente Elevadizo
// =====================================================
// Configuracion WiFi
#define WIFI_SSID "Omega"
#define WIFI_PASSWORD "d5adc4a32689"

// Backend ngrok
#define BACKEND_URL "https://warless-predestinately-bethann.ngrok-free.dev"

// Intervalo de polling (milisegundos)
#define INTERVALO_POLL 5000

// Timeout Mega (milisegundos)
#define TIMEOUT_MEGA 15000

// Pines UART
#define RXD2 16
#define TXD2 17

// Pines botones locales
#define PIN_BOTON_ABRIR 32
#define PIN_BOTON_CERRAR 33
// =====================================================

#include <WiFi.h>
#include <WebServer.h>
#include <HTTPClient.h>

WebServer server(80);

bool megaOnline = false;
bool esclavo1Online = false;
unsigned long ultimoMensajeMega = 0;
unsigned long ultimoPoll = 0;
int anguloActualServo = 90;

bool ultimoEstadoBotonAbrir = HIGH;
bool ultimoEstadoBotonCerrar = HIGH;

void setup() {
  Serial.begin(115200);
  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);

  pinMode(PIN_BOTON_ABRIR, INPUT_PULLUP);
  pinMode(PIN_BOTON_CERRAR, INPUT_PULLUP);

  Serial.println("=== ESP32 Gateway Puente Elevadizo ===");
  Serial.print("WiFi SSID: ");
  Serial.println(WIFI_SSID);
  Serial.print("Backend: ");
  Serial.println(BACKEND_URL);

  conectarWiFi();
  iniciarServidorHTTP();

  Serial.println("Listo. Esperando comandos...");
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    conectarWiFi();
  }

  server.handleClient();
  procesarBotonesLocales();

  if (millis() - ultimoPoll > INTERVALO_POLL) {
    ultimoPoll = millis();
    consultarInstruccionesPendientes();
  }

  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();
    if (cmd.length() > 0) {
      procesarComandoLocal(cmd);
    }
  }

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

  if (megaOnline && (millis() - ultimoMensajeMega > TIMEOUT_MEGA)) {
    megaOnline = false;
    Serial.println("[ALERTA] Mega no responde");
  }
}

void conectarWiFi() {
  Serial.print("Conectando a WiFi: ");
  Serial.println(WIFI_SSID);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

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
    Serial.println("WiFi FALLO, reintentando...");
  }
}

void procesarMensajeMega(String msg) {
  if (msg.startsWith("ESCLAVO1=")) {
    String estado = msg.substring(9);
    esclavo1Online = (estado == "OK");
    Serial.print(">> Esclavo 1 ");
    Serial.println(esclavo1Online ? "disponible" : "NO disponible");
    return;
  }

  if (msg.startsWith("RESP ")) {
    String resp = msg.substring(5);
    Serial.print(">> Respuesta: ");
    Serial.println(resp);
    if (resp.startsWith("POS:")) {
      anguloActualServo = resp.substring(4).toInt();
    }
    return;
  }

  if (msg.startsWith("RESP ERROR:")) {
    Serial.print(">> ERROR: ");
    Serial.println(msg.substring(11));
    return;
  }
}

void procesarComandoLocal(String cmd) {
  if (cmd == "ESTADO") {
    Serial.println("--- Estado ---");
    Serial.println("WiFi: " + String(WiFi.status() == WL_CONNECTED ? "OK" : "CAIDO"));
    Serial.println("IP: " + WiFi.localIP().toString());
    Serial.println("Mega: " + String(megaOnline ? "OK" : "OFFLINE"));
    Serial.println("Esclavo: " + String(esclavo1Online ? "OK" : "OFFLINE"));
    Serial.println("Angulo: " + String(anguloActualServo) + "°");
    return;
  }

  if (cmd == "PING") {
    Serial2.println("PING");
    return;
  }

  Serial.print(">> Enviando al Mega: ");
  Serial.println(cmd);
  Serial2.println(cmd);
}

void procesarBotonesLocales() {
  bool estadoBotonAbrir = digitalRead(PIN_BOTON_ABRIR);
  bool estadoBotonCerrar = digitalRead(PIN_BOTON_CERRAR);

  if (estadoBotonAbrir == LOW && ultimoEstadoBotonAbrir == HIGH) {
    delay(50);
    if (digitalRead(PIN_BOTON_ABRIR) == LOW) {
      Serial.println("[BOTON] ABRIR pulsado");
      enviarInstruccionAlMega("SERVO_SUBIR");
      notificarEjecucionLocal("SERVO_SUBIR", "OK");
    }
  }
  ultimoEstadoBotonAbrir = estadoBotonAbrir;

  if (estadoBotonCerrar == LOW && ultimoEstadoBotonCerrar == HIGH) {
    delay(50);
    if (digitalRead(PIN_BOTON_CERRAR) == LOW) {
      Serial.println("[BOTON] CERRAR pulsado");
      enviarInstruccionAlMega("SERVO_BAJAR");
      notificarEjecucionLocal("SERVO_BAJAR", "OK");
    }
  }
  ultimoEstadoBotonCerrar = estadoBotonCerrar;
}

void enviarInstruccionAlMega(String instruccion) {
  if (!megaOnline || !esclavo1Online) {
    Serial.println("[ERROR] Mega o Esclavo no disponible");
    return;
  }
  Serial.print(">> Enviando al Mega: ");
  Serial.println(instruccion);
  Serial2.println(instruccion);
}

void notificarEjecucionLocal(String nombre, String resultado) {
  HTTPClient http;
  String url = String(BACKEND_URL) + "/api/instrucciones/local";

  http.begin(url);
  http.addHeader("Content-Type", "application/json");

  String json = "{\"nombre\":\"" + nombre + "\",\"resultado\":\"" + resultado + "\",\"angulo\":" + anguloActualServo + ",\"ipDispositivo\":\"" + WiFi.localIP().toString() + "\"}";

  int httpCode = http.POST(json);
  if (httpCode > 0) {
    Serial.print("[HTTP] Local: ");
    Serial.println(httpCode);
  } else {
    Serial.print("[HTTP] Error: ");
    Serial.println(http.errorToString(httpCode).c_str());
  }
  http.end();
}

void consultarInstruccionesPendientes() {
  if (WiFi.status() != WL_CONNECTED) return;

  HTTPClient http;
  String url = String(BACKEND_URL) + "/api/instrucciones/pendientes";

  http.begin(url);
  int httpCode = http.GET();

  if (httpCode == 200) {
    String payload = http.getString();
    Serial.print("[POLL] Respuesta: ");
    Serial.println(payload);

    // Parse JSON simple
    if (payload.indexOf("Id") > 0) {
      procesarPayload(payload);
    }
  } else if (httpCode == 404) {
    Serial.println("[POLL] Sin instrucciones pendientes");
  } else {
    Serial.print("[POLL] Error: ");
    Serial.println(httpCode);
  }
  http.end();
}

void procesarPayload(String payload) {
  // Extraer Id y Nombre del JSON
  int idIndex = payload.indexOf("\"Id\":");
  int nombreIndex = payload.indexOf("\"Nombre\":\"");
  int descIndex = payload.indexOf("\"Descripcion\":\"");

  if (idIndex < 0 || nombreIndex < 0) {
    Serial.println("[POLL] JSON sin datos validos");
    return;
  }

  // Extraer Id
  int idStart = idIndex + 5;
  int idEnd = payload.indexOf(",", idStart);
  if (idEnd < 0) idEnd = payload.indexOf("}", idStart);
  int idPendiente = payload.substring(idStart, idEnd).toInt();

  // Extraer Nombre
  int nombreStart = nombreIndex + 10;
  int nombreEnd = payload.indexOf("\"", nombreStart);
  String nombre = payload.substring(nombreStart, nombreEnd);

  Serial.print(">> Instruccion: ");
  Serial.println(nombre);

  // Enviar instruccion tal cual al Mega
  unsigned long inicio = millis();
  enviarInstruccionAlMega(nombre);
  delay(2500);
  unsigned long tiempo = millis() - inicio;

  bool success = megaOnline && esclavo1Online;

  // Notificar respuesta al backend
  notificarRespuesta(idPendiente, success ? "OK" : "ERROR", tiempo);
}

void notificarRespuesta(int idPendiente, String resultado, unsigned long tiempoMs) {
  HTTPClient http;
  String url = String(BACKEND_URL) + "/api/instrucciones/respuesta";

  http.begin(url);
  http.addHeader("Content-Type", "application/json");

  String json = "{\"idPendiente\":" + String(idPendiente) + ",\"resultado\":\"" + resultado + "\",\"angulo\":" + anguloActualServo + ",\"ipDispositivo\":\"" + WiFi.localIP().toString() + "\"}";

  int httpCode = http.POST(json);
  if (httpCode > 0) {
    Serial.print("[HTTP] Respuesta: ");
    Serial.println(httpCode);
  } else {
    Serial.print("[HTTP] Error: ");
    Serial.println(http.errorToString(httpCode).c_str());
  }
  http.end();
}

void iniciarServidorHTTP() {
  server.on("/abrir", HTTP_POST, handleAbrir);
  server.on("/cerrar", HTTP_POST, handleCerrar);
  server.on("/estado", HTTP_GET, handleEstado);

  server.begin();
  Serial.println(">> Servidor HTTP iniciado");
}

void handleAbrir() {
  if (!megaOnline || !esclavo1Online) {
    server.send(503, "application/json", "{\"error\":\"Mega o Esclavo no disponible\"}");
    return;
  }
  enviarInstruccionAlMega("SERVO_SUBIR");
  delay(2000);
  Serial2.println("SERVO_STATUS");
  delay(500);
  String estado = anguloActualServo <= 90 ? "ABIERTO" : "CERRADO";
  server.send(200, "application/json", "{\"estado\":\"" + estado + "\",\"angulo\":" + anguloActualServo + "}");
}

void handleCerrar() {
  if (!megaOnline || !esclavo1Online) {
    server.send(503, "application/json", "{\"error\":\"Mega o Esclavo no disponible\"}");
    return;
  }
  enviarInstruccionAlMega("SERVO_BAJAR");
  delay(2000);
  Serial2.println("SERVO_STATUS");
  delay(500);
  String estado = anguloActualServo <= 90 ? "ABIERTO" : "CERRADO";
  server.send(200, "application/json", "{\"estado\":\"" + estado + "\",\"angulo\":" + anguloActualServo + "}");
}

void handleEstado() {
  Serial2.println("SERVO_STATUS");
  delay(500);
  String estado = anguloActualServo <= 90 ? "ABIERTO" : "CERRADO";
  String respuesta = "{\"estado\":\"" + estado + "\",\"angulo\":" + anguloActualServo + ",\"megaOnline\":" + megaOnline + ",\"esclavoOnline\":" + esclavo1Online + "}";
  server.send(200, "application/json", respuesta);
}
