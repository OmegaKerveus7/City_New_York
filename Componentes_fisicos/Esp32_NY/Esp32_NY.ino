// =====================================================
// ESP32 Gateway - Puente Elevadizo + Elevador de pisos
// =====================================================
#define WIFI_SSID "Omega"
#define WIFI_PASSWORD "d5adc4a32689"
#define BACKEND_URL "https://warless-predestinately-bethann.ngrok-free.dev"
#define INTERVALO_POLL 5000
#define INTERVALO_PING 3000
#define TIMEOUT_MEGA 10000
#define RXD2 16
#define TXD2 17
#define PIN_BOTON_ABRIR 32
#define PIN_BOTON_CERRAR 33

// Angulos del puente
#define ANGULO_ABIERTO  115
#define ANGULO_CERRADO  180

// =====================================================
#include <WiFi.h>
#include <WebServer.h>
#include <HTTPClient.h>

WebServer server(80);

bool megaOnline = false;
bool esclavo1Online = false;   // servo puente
bool esclavo2Online = false;   // elevador stepper
unsigned long ultimoMensajeMega = 0;
unsigned long ultimoPoll = 0;
unsigned long ultimoPingMega = 0;
int anguloActualServo = 90;
int nivelActualElevador = 0;
long pasosRestantesElevador = 0;

bool ultimoEstadoBotonAbrir = HIGH;
bool ultimoEstadoBotonCerrar = HIGH;

// =====================================================
// SETUP
// =====================================================
void setup() {
  Serial.begin(115200);
  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);

  pinMode(PIN_BOTON_ABRIR, INPUT_PULLUP);
  pinMode(PIN_BOTON_CERRAR, INPUT_PULLUP);

  Serial.println("=== ESP32 Gateway Puente Elevadizo + Elevador ===");
  Serial.print("WiFi SSID: ");
  Serial.println(WIFI_SSID);
  Serial.print("Backend: ");
  Serial.println(BACKEND_URL);
  Serial.println("");

  conectarWiFi();
  iniciarServidorHTTP();

  Serial.println("Listo. Esperando comandos...");
  Serial.println("");
}

// =====================================================
// LOOP PRINCIPAL
// =====================================================
void loop() {
  if (WiFi.status() != WL_CONNECTED) conectarWiFi();

  server.handleClient();
  procesarBotonesLocales();

  // KEEPALIVE: PING al Mega
  if (millis() - ultimoPingMega > INTERVALO_PING) {
    ultimoPingMega = millis();
    Serial2.println("PING");
    Serial.println("[KEEPALIVE] PING -> Mega");
  }

  // Poll al backend
  if (millis() - ultimoPoll > INTERVALO_POLL) {
    ultimoPoll = millis();
    consultarInstruccionesPendientes();
  }

  // Comandos desde Monitor Serie
  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();
    // NO forzar a mayúsculas para que el Mega reciba el formato original
    if (cmd.length() > 0) {
      Serial.print(">> Manual desde USB: [");
      Serial.print(cmd);
      Serial.println("]");
      Serial2.println(cmd);
    }
  }

  // Respuestas del Mega
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

  // Detectar Mega offline
  if (megaOnline && (millis() - ultimoMensajeMega > TIMEOUT_MEGA)) {
    megaOnline = false;
    esclavo1Online = false;
    esclavo2Online = false;
    Serial.println("[ALERTA] Mega no responde");
  }
}

// =====================================================
// WIFI
// =====================================================
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

// =====================================================
// PROCESAR MENSAJES DEL MEGA
// =====================================================
void procesarMensajeMega(String msg) {
  megaOnline = true;
  ultimoMensajeMega = millis();

  if (msg.startsWith("ESCLAVO1=")) {
    esclavo1Online = (msg.substring(9) == "OK");
    Serial.print(">> Esclavo 1 ");
    Serial.println(esclavo1Online ? "DISPONIBLE" : "NO DISPONIBLE");
    return;
  }

  if (msg.startsWith("PONG_MEGA")) {
    Serial.println(">> Mega OK");
    return;
  }

  if (msg.startsWith("RESP ")) {
    String resp = msg.substring(5);
    Serial.print(">> Respuesta: [");
    Serial.print(resp);
    Serial.println("]");

    // Servo del puente
    if (resp.startsWith("POS:")) {
      esclavo1Online = true;
      anguloActualServo = resp.substring(4).toInt();
      Serial.print(">> Angulo actualizado: ");
      Serial.print(anguloActualServo);
      Serial.println(" grados");
      return;
    }

    // Elevador: NIVEL:xx|PAS:xxxxx
    if (resp.startsWith("NIVEL:")) {
      esclavo2Online = true;
      int barra = resp.indexOf('|');
      if (barra > 0) {
        String nivelStr = resp.substring(6, barra);
        String pasStr = resp.substring(barra + 5);
        nivelActualElevador = nivelStr.toInt();
        pasosRestantesElevador = pasStr.toInt();
        Serial.print(">> Elevador nivel: ");
        Serial.print(nivelActualElevador);
        Serial.print(" | pasos restantes: ");
        Serial.println(pasosRestantesElevador);
      }
      return;
    }

    if (resp.startsWith("ERROR")) {
      Serial.print(">> ERROR del esclavo: ");
      Serial.println(resp);
      return;
    }

    return;
  }
}

// =====================================================
// BOTONES FISICOS
// =====================================================
void procesarBotonesLocales() {
  bool estadoBotonAbrir = digitalRead(PIN_BOTON_ABRIR);
  if (estadoBotonAbrir == LOW && ultimoEstadoBotonAbrir == HIGH) {
    delay(50);
    if (digitalRead(PIN_BOTON_ABRIR) == LOW) {
      Serial.println("[BOTON] ABRIR pulsado");
      if (megaOnline) {
        Serial2.println("SERVO_SUBIR");
        esperarRespuestaMega(3500);
        notificarEjecucionLocal("ABRIR", "OK");
      } else {
        Serial.println("[ERROR] Mega offline");
      }
    }
  }
  ultimoEstadoBotonAbrir = estadoBotonAbrir;

  bool estadoBotonCerrar = digitalRead(PIN_BOTON_CERRAR);
  if (estadoBotonCerrar == LOW && ultimoEstadoBotonCerrar == HIGH) {
    delay(50);
    if (digitalRead(PIN_BOTON_CERRAR) == LOW) {
      Serial.println("[BOTON] CERRAR pulsado");
      if (megaOnline) {
        Serial2.println("SERVO_BAJAR");
        esperarRespuestaMega(3500);
        notificarEjecucionLocal("CERRAR", "OK");
      } else {
        Serial.println("[ERROR] Mega offline");
      }
    }
  }
  ultimoEstadoBotonCerrar = estadoBotonCerrar;
}

// =====================================================
// ESPERAR RESPUESTA DEL MEGA
// =====================================================
void esperarRespuestaMega(unsigned long timeout) {
  unsigned long inicio = millis();
  while (millis() - inicio < timeout) {
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
    server.handleClient();
    delay(10);
  }
}

// =====================================================
// NOTIFICAR AL BACKEND
// =====================================================
void notificarEjecucionLocal(String accion, String resultado) {
  if (WiFi.status() != WL_CONNECTED) return;

  HTTPClient http;
  String url = String(BACKEND_URL) + "/api/instrucciones/local";

  http.begin(url);
  http.addHeader("Content-Type", "application/json");
  http.setTimeout(5000);

  String json = "{\"nombre\":\"" + accion +
                "\",\"resultado\":\"" + resultado +
                "\",\"angulo\":" + String(anguloActualServo) +
                ",\"ipDispositivo\":\"" + WiFi.localIP().toString() + "\"}";

  Serial.print("[HTTP] Local: ");
  Serial.println(json);

  int httpCode = http.POST(json);
  if (httpCode > 0) {
    Serial.print("[HTTP] Local respuesta: ");
    Serial.println(httpCode);
  } else {
    Serial.print("[HTTP] Error: ");
    Serial.println(http.errorToString(httpCode).c_str());
  }
  http.end();
}

// =====================================================
// POLL AL BACKEND
// =====================================================
void consultarInstruccionesPendientes() {
  if (WiFi.status() != WL_CONNECTED) return;

  HTTPClient http;
  String url = String(BACKEND_URL) + "/api/instrucciones/pendientes";

  http.begin(url);
  http.setTimeout(5000);
  int httpCode = http.GET();

  if (httpCode == 200) {
    String payload = http.getString();
    Serial.print("[POLL] ");
    Serial.println(payload);

    if (payload.indexOf("Id") > 0) {
      procesarPayloadInstrucciones(payload);
    }
  } else if (httpCode == 404) {
    Serial.println("[POLL] Sin instrucciones pendientes");
  } else {
    Serial.print("[POLL] Error HTTP: ");
    Serial.println(httpCode);
  }
  http.end();
}

void procesarPayloadInstrucciones(String payload) {
  int idIdx = payload.indexOf("\"Id\":");
  int nomIdx = payload.indexOf("\"Nombre\":\"");

  if (idIdx < 0 || nomIdx < 0) {
    Serial.println("[JSON] Formato no reconocido");
    return;
  }

  idIdx += 5;
  nomIdx += 10;

  int idFin = payload.indexOf(",", idIdx);
  if (idFin < 0) idFin = payload.indexOf("}", idIdx);
  int nomFin = payload.indexOf("\"", nomIdx);

  if (idFin < 0 || nomFin < 0) {
    Serial.println("[JSON] Error al parsear");
    return;
  }

  int id = payload.substring(idIdx, idFin).toInt();
  String nombre = payload.substring(nomIdx, nomFin);

  Serial.print(">> Instruccion recibida: id=");
  Serial.print(id);
  Serial.print(" nombre=[");
  Serial.print(nombre);
  Serial.println("]");

  if (!megaOnline) {
    Serial.println(">> Mega offline, reportando ERROR");
    enviarRespuestaInstruccion(id, "ERROR", 0);
    return;
  }

  Serial.print(">> Enviando al Mega: [");
  Serial.print(nombre);
  Serial.println("]");
  Serial2.println(nombre);

  unsigned long inicio = millis();
  esperarRespuestaMega(3500);
  unsigned long tiempo = millis() - inicio;

  bool success = megaOnline && (esclavo1Online || esclavo2Online);
  Serial.print(">> Resultado: ");
  Serial.println(success ? "OK" : "ERROR");

  enviarRespuestaInstruccion(id, success ? "OK" : "ERROR", tiempo);
}

void enviarRespuestaInstruccion(int idInstruccion, String resultado, unsigned long tiempoMs) {
  if (WiFi.status() != WL_CONNECTED) return;

  HTTPClient http;
  String url = String(BACKEND_URL) + "/api/instrucciones/respuesta";

  http.begin(url);
  http.addHeader("Content-Type", "application/json");
  http.setTimeout(5000);

  String json = "{\"idPendiente\":" + String(idInstruccion) +
                ",\"resultado\":\"" + resultado +
                "\",\"angulo\":" + String(anguloActualServo) +
                ",\"nivel\":" + String(nivelActualElevador) +
                ",\"ipDispositivo\":\"" + WiFi.localIP().toString() +
                "\",\"tiempoRespuestaMs\":" + String(tiempoMs) + "}";

  Serial.print("[HTTP] Respuesta: ");
  Serial.println(json);

  int httpCode = http.POST(json);
  if (httpCode > 0) {
    Serial.print("[HTTP] Respuesta enviada: ");
    Serial.println(httpCode);
  } else {
    Serial.print("[HTTP] Error: ");
    Serial.println(http.errorToString(httpCode).c_str());
  }
  http.end();
}

// =====================================================
// SERVIDOR HTTP LOCAL
// =====================================================
void iniciarServidorHTTP() {
  // Puente
  server.on("/abrir", HTTP_POST, handleAbrir);
  server.on("/cerrar", HTTP_POST, handleCerrar);
  server.on("/estado", HTTP_GET, handleEstado);
  server.on("/actualizar", HTTP_POST, handleActualizar);
  server.on("/verificar", HTTP_GET, handleVerificar);

  // Elevador
  server.on("/elevador/subir", HTTP_POST, handleElevadorSubir);
  server.on("/elevador/bajar", HTTP_POST, handleElevadorBajar);
  server.on("/elevador/ir", HTTP_POST, handleElevadorIr);
  server.on("/elevador/parar", HTTP_POST, handleElevadorParar);
  server.on("/elevador/status", HTTP_GET, handleElevadorStatus);
  server.on("/elevador/calibrar", HTTP_POST, handleElevadorCalibrar);

  server.begin();
  Serial.println(">> Servidor HTTP iniciado en puerto 80");
}

// ---------- PUENTE ----------
void handleAbrir() {
  if (!megaOnline) {
    server.send(503, "application/json", "{\"error\":\"Mega offline\"}");
    return;
  }

  Serial.println("[HTTP] /abrir");
  Serial2.println("SERVO_SUBIR");
  esperarRespuestaMega(3500);

  String estado;
  if (anguloActualServo >= 110 && anguloActualServo <= 130) estado = "ABIERTO";
  else if (anguloActualServo >= 170) estado = "CERRADO";
  else estado = "INTERMEDIO";

  String resp = "{\"estado\":\"" + estado + "\",\"angulo\":" + String(anguloActualServo) + "}";
  server.send(200, "application/json", resp);
}

void handleCerrar() {
  if (!megaOnline) {
    server.send(503, "application/json", "{\"error\":\"Mega offline\"}");
    return;
  }

  Serial.println("[HTTP] /cerrar");
  Serial2.println("SERVO_BAJAR");
  esperarRespuestaMega(3500);

  String estado;
  if (anguloActualServo >= 110 && anguloActualServo <= 130) estado = "ABIERTO";
  else if (anguloActualServo >= 170) estado = "CERRADO";
  else estado = "INTERMEDIO";

  String resp = "{\"estado\":\"" + estado + "\",\"angulo\":" + String(anguloActualServo) + "}";
  server.send(200, "application/json", resp);
}

void handleEstado() {
  Serial.println("[HTTP] /estado");

  Serial2.println("SERVO_STATUS");
  esperarRespuestaMega(1500);

  String estado;
  if (anguloActualServo >= 110 && anguloActualServo <= 130) estado = "ABIERTO";
  else if (anguloActualServo >= 170) estado = "CERRADO";
  else estado = "INTERMEDIO";

  String resp = "{\"estado\":\"" + estado +
                "\",\"angulo\":" + String(anguloActualServo) +
                ",\"megaOnline\":" + String(megaOnline ? "true" : "false") +
                ",\"esclavoOnline\":" + String(esclavo1Online ? "true" : "false") + "}";
  server.send(200, "application/json", resp);
}

void handleActualizar() {
  if (!server.hasArg("angulo")) {
    server.send(400, "application/json", "{\"error\":\"Falta parametro angulo\"}");
    return;
  }

  int angulo = server.arg("angulo").toInt();
  if (angulo < 0 || angulo > 180) {
    server.send(400, "application/json", "{\"error\":\"Angulo fuera de rango (0-180)\"}");
    return;
  }

  if (!megaOnline) {
    server.send(503, "application/json", "{\"error\":\"Mega offline\"}");
    return;
  }

  Serial.print("[HTTP] /actualizar angulo=");
  Serial.println(angulo);

  Serial2.print("SERVO ");
  Serial2.println(angulo);
  esperarRespuestaMega(3500);

  String estado;
  if (anguloActualServo >= 110 && anguloActualServo <= 130) estado = "ABIERTO";
  else if (anguloActualServo >= 170) estado = "CERRADO";
  else estado = "INTERMEDIO";

  String resp = "{\"estado\":\"" + estado + "\",\"angulo\":" + String(anguloActualServo) + "}";
  server.send(200, "application/json", resp);
}

void handleVerificar() {
  server.send(200, "application/json",
    "{\"conectado\":true,\"ip\":\"" + WiFi.localIP().toString() +
    "\",\"megaOnline\":" + String(megaOnline ? "true" : "false") +
    ",\"esclavo1Online\":" + String(esclavo1Online ? "true" : "false") +
    ",\"esclavo2Online\":" + String(esclavo2Online ? "true" : "false") + "}");
}

// ---------- ELEVADOR ----------
void handleElevadorSubir() {
  if (!megaOnline) {
    server.send(503, "application/json", "{\"error\":\"Mega offline\"}");
    return;
  }

  int niveles = 1;
  if (server.hasArg("niveles")) niveles = server.arg("niveles").toInt();
  if (niveles <= 0) niveles = 1;

  String cmd = "SUBIR_NIVEL " + String(niveles);
  Serial.print("[HTTP] /elevador/subir -> ");
  Serial.println(cmd);

  Serial2.println(cmd);
  esperarRespuestaMega(3500);

  String resp = "{\"nivel\":" + String(nivelActualElevador) +
                ",\"pasosRestantes\":" + String(pasosRestantesElevador) + "}";
  server.send(200, "application/json", resp);
}

void handleElevadorBajar() {
  if (!megaOnline) {
    server.send(503, "application/json", "{\"error\":\"Mega offline\"}");
    return;
  }

  int niveles = 1;
  if (server.hasArg("niveles")) niveles = server.arg("niveles").toInt();
  if (niveles <= 0) niveles = 1;

  String cmd = "BAJAR_NIVEL " + String(niveles);
  Serial.print("[HTTP] /elevador/bajar -> ");
  Serial.println(cmd);

  Serial2.println(cmd);
  esperarRespuestaMega(3500);

  String resp = "{\"nivel\":" + String(nivelActualElevador) +
                ",\"pasosRestantes\":" + String(pasosRestantesElevador) + "}";
  server.send(200, "application/json", resp);
}

void handleElevadorIr() {
  if (!megaOnline) {
    server.send(503, "application/json", "{\"error\":\"Mega offline\"}");
    return;
  }

  if (!server.hasArg("nivel")) {
    server.send(400, "application/json", "{\"error\":\"Falta parametro nivel\"}");
    return;
  }

  int nivel = server.arg("nivel").toInt();
  if (nivel < 0 || nivel > 5) {
    server.send(400, "application/json", "{\"error\":\"Nivel fuera de rango (0-5)\"}");
    return;
  }

  String cmd = "IR_A_NIVEL " + String(nivel);
  Serial.print("[HTTP] /elevador/ir -> ");
  Serial.println(cmd);

  Serial2.println(cmd);
  esperarRespuestaMega(3500);

  String resp = "{\"nivel\":" + String(nivelActualElevador) +
                ",\"pasosRestantes\":" + String(pasosRestantesElevador) + "}";
  server.send(200, "application/json", resp);
}

void handleElevadorParar() {
  if (!megaOnline) {
    server.send(503, "application/json", "{\"error\":\"Mega offline\"}");
    return;
  }

  Serial.println("[HTTP] /elevador/parar");
  Serial2.println("ELEVADOR_PARAR");
  esperarRespuestaMega(1500);

  server.send(200, "application/json", "{\"ok\":true}");
}

void handleElevadorStatus() {
  if (!megaOnline) {
    server.send(503, "application/json", "{\"error\":\"Mega offline\"}");
    return;
  }

  Serial.println("[HTTP] /elevador/status");
  Serial2.println("ELEVADOR_STATUS");
  esperarRespuestaMega(1500);

  String resp = "{\"nivel\":" + String(nivelActualElevador) +
                ",\"pasosRestantes\":" + String(pasosRestantesElevador) +
                ",\"esclavo2Online\":" + String(esclavo2Online ? "true" : "false") + "}";
  server.send(200, "application/json", resp);
}

void handleElevadorCalibrar() {
  if (!megaOnline) {
    server.send(503, "application/json", "{\"error\":\"Mega offline\"}");
    return;
  }

  Serial.println("[HTTP] /elevador/calibrar");
  Serial2.println("CALIBRAR");
  esperarRespuestaMega(3500);

  server.send(200, "application/json", "{\"ok\":true,\"nivel\":0}");
}