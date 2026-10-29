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

unsigned long ciclosEsp32 = 0;
unsigned long ciclosMega = 0;

bool ultimoEstadoBotonAbrir = HIGH;
bool ultimoEstadoBotonCerrar = HIGH;

// =====================================================
// SETUP
// =====================================================
void setup() {
  Serial.begin(115200);
  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2); // 9600 baudios hacia el Mega

  pinMode(PIN_BOTON_ABRIR, INPUT_PULLUP);
  pinMode(PIN_BOTON_CERRAR, INPUT_PULLUP);

  Serial.println("=== ESP32 Gateway Puente Elevadizo + Elevador ===");
  conectarWiFi();
  iniciarServidorHTTP();
}

// =====================================================
// LOOP PRINCIPAL
// =====================================================
void loop() {
  if (WiFi.status() != WL_CONNECTED) conectarWiFi();

  server.handleClient();
  procesarBotonesLocales();

  if (millis() - ultimoPingMega > INTERVALO_PING) {
    ultimoPingMega = millis();
    ciclosEsp32++;
    Serial2.println("PING");
    Serial.print("[KEEPALIVE] PING -> Mega. Ciclo ESP32: ");
    Serial.println(ciclosEsp32);
  }

  if (millis() - ultimoPoll > INTERVALO_POLL) {
    ultimoPoll = millis();
    consultarInstruccionesPendientes();
  }

  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();
    if (cmd.length() > 0) {
      Serial2.println(cmd);
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
    esclavo1Online = false;
    esclavo2Online = false;
    Serial.println("[ALERTA] Mega no responde");
  }
}

// =====================================================
// WIFI Y MENSAJES
// =====================================================
void conectarWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  unsigned long inicio = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - inicio < 15000) {
    delay(500);
    Serial.print(".");
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("\nWiFi OK. IP: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nWiFi FALLO");
  }
}

void procesarMensajeMega(String msg) {
  megaOnline = true;
  ultimoMensajeMega = millis();

  if (msg.startsWith("ESCLAVO1=")) { esclavo1Online = (msg.substring(9) == "OK"); return; }
  if (msg.startsWith("ESCLAVO2=")) { esclavo2Online = (msg.substring(9) == "OK"); return; }
  
  if (msg.startsWith("CICLO_MEGA:")) {
    ciclosMega = msg.substring(11).toInt();
    return;
  }

  if (msg.startsWith("RESP ")) {
    String resp = msg.substring(5);
    if (resp.startsWith("POS:")) { esclavo1Online = true; anguloActualServo = resp.substring(4).toInt(); return; }
    if (resp.startsWith("NIVEL:")) {
      esclavo2Online = true;
      int barra = resp.indexOf('|');
      if (barra > 0) {
        nivelActualElevador = resp.substring(6, barra).toInt();
        pasosRestantesElevador = resp.substring(barra + 5).toInt();
      }
      return;
    }
  }
}

void procesarBotonesLocales() {
  bool estadoAbrir = digitalRead(PIN_BOTON_ABRIR);
  if (estadoAbrir == LOW && ultimoEstadoBotonAbrir == HIGH) {
    delay(50);
    if (digitalRead(PIN_BOTON_ABRIR) == LOW && megaOnline) {
      Serial2.println("SERVO_SUBIR");
      esperarRespuestaMega(3500);
      notificarEjecucionLocal("ABRIR", "OK");
    }
  }
  ultimoEstadoBotonAbrir = estadoAbrir;

  bool estadoCerrar = digitalRead(PIN_BOTON_CERRAR);
  if (estadoCerrar == LOW && ultimoEstadoBotonCerrar == HIGH) {
    delay(50);
    if (digitalRead(PIN_BOTON_CERRAR) == LOW && megaOnline) {
      Serial2.println("SERVO_BAJAR");
      esperarRespuestaMega(3500);
      notificarEjecucionLocal("CERRAR", "OK");
    }
  }
  ultimoEstadoBotonCerrar = estadoCerrar;
}

void esperarRespuestaMega(unsigned long timeout) {
  unsigned long inicio = millis();
  while (millis() - inicio < timeout) {
    if (Serial2.available()) {
      String linea = Serial2.readStringUntil('\n');
      linea.trim();
      if (linea.length() > 0) {
        ultimoMensajeMega = millis();
        megaOnline = true;
        procesarMensajeMega(linea);
      }
    }
    server.handleClient();
    delay(10);
  }
}

// =====================================================
// HTTP Y BACKEND
// =====================================================
void notificarEjecucionLocal(String accion, String resultado) {
  if (WiFi.status() != WL_CONNECTED) return;
  HTTPClient http;
  http.begin(String(BACKEND_URL) + "/api/instrucciones/local");
  http.addHeader("Content-Type", "application/json");
  http.setTimeout(5000);
  String json = "{\"nombre\":\"" + accion + "\",\"resultado\":\"" + resultado + "\",\"angulo\":" + String(anguloActualServo) + ",\"ipDispositivo\":\"" + WiFi.localIP().toString() + "\"}";
  http.POST(json);
  http.end();
}

void consultarInstruccionesPendientes() {
  if (WiFi.status() != WL_CONNECTED) return;
  HTTPClient http;
  http.begin(String(BACKEND_URL) + "/api/instrucciones/pendientes");
  http.setTimeout(5000);
  int httpCode = http.GET();
  if (httpCode == 200) {
    String payload = http.getString();
    if (payload.indexOf("Id") > 0) procesarPayloadInstrucciones(payload);
  }
  http.end();
}

void procesarPayloadInstrucciones(String payload) {
  int idIdx = payload.indexOf("\"Id\":");
  int nomIdx = payload.indexOf("\"Nombre\":\"");
  if (idIdx < 0 || nomIdx < 0) return;
  idIdx += 5; nomIdx += 10;
  int idFin = payload.indexOf(",", idIdx);
  if (idFin < 0) idFin = payload.indexOf("}", idIdx);
  int nomFin = payload.indexOf("\"", nomIdx);
  if (idFin < 0 || nomFin < 0) return;

  int id = payload.substring(idIdx, idFin).toInt();
  String nombre = payload.substring(nomIdx, nomFin);
  nombre.trim(); // <--- LIMPIA EL COMANDO ANTES DE ENVIARLO

  if (!megaOnline) {
    enviarRespuestaInstruccion(id, "ERROR", 0);
    return;
  }

  Serial2.println(nombre);
  unsigned long inicio = millis();
  esperarRespuestaMega(3500);
  unsigned long tiempo = millis() - inicio;

  bool success = megaOnline && (esclavo1Online || esclavo2Online);
  enviarRespuestaInstruccion(id, success ? "OK" : "ERROR", tiempo);
}

void enviarRespuestaInstruccion(int idInstruccion, String resultado, unsigned long tiempoMs) {
  if (WiFi.status() != WL_CONNECTED) return;
  HTTPClient http;
  http.begin(String(BACKEND_URL) + "/api/instrucciones/respuesta");
  http.addHeader("Content-Type", "application/json");
  http.setTimeout(5000);
  String json = "{\"idPendiente\":" + String(idInstruccion) + ",\"resultado\":\"" + resultado + "\",\"angulo\":" + String(anguloActualServo) + ",\"nivel\":" + String(nivelActualElevador) + ",\"ipDispositivo\":\"" + WiFi.localIP().toString() + "\",\"tiempoRespuestaMs\":" + String(tiempoMs) + "}";
  http.POST(json);
  http.end();
}

// =====================================================
// SERVIDOR HTTP LOCAL
// =====================================================
void iniciarServidorHTTP() {
  server.on("/abrir", HTTP_POST, [](){ if(megaOnline){ Serial2.println("SERVO_SUBIR"); esperarRespuestaMega(3500); server.send(200, "application/json", "{\"estado\":\"OK\"}"); } else server.send(503, "application/json", "{}"); });
  server.on("/cerrar", HTTP_POST, [](){ if(megaOnline){ Serial2.println("SERVO_BAJAR"); esperarRespuestaMega(3500); server.send(200, "application/json", "{\"estado\":\"OK\"}"); } else server.send(503, "application/json", "{}"); });
  server.begin();
}