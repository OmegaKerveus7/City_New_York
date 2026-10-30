#include <WiFi.h>
#include <WebServer.h>
#include <HTTPClient.h>

#define WIFI_SSID "Omega"
#define WIFI_PASSWORD "d5adc4a32689"
#define BACKEND_URL "https://warless-predestinately-bethann.ngrok-free.dev"
#define INTERVALO_POLL 5000
#define INTERVALO_PING 3000
#define INTERVALO_ESTADO 5000
#define TIMEOUT_MEGA 10000
#define RXD2 16
#define TXD2 17
#define PIN_BOTON_ABRIR 32
#define PIN_BOTON_CERRAR 33

WebServer server(80);

bool megaOnline = false;
bool esclavo1Online = false;
bool esclavo2Online = false;
bool esclavo3Online = false;
unsigned long ultimoMensajeMega = 0;
unsigned long ultimoPoll = 0;
unsigned long ultimoPingMega = 0;
unsigned long ultimoEstadoMega = 0;
int anguloActualServo = 90;
int nivelActualElevador = 0;
long pasosRestantesElevador = 0;

String estadoPuente = "DESCONOCIDO";
String estadoTeleferico = "DETENIDO";
long pasosTeleferico = 0;
String fuenteBomba = "OFF";
int fuenteAgua = 0;
String fuenteLuces = "OFF";
String semaforoNS = "ROJO";
String semaforoEO = "ROJO";
String semaforoModo = "MANUAL";
String ultimaLineaEstado = "";

bool procesandoInstruccion = false;
unsigned long ciclosEsp32 = 0;
unsigned long ciclosMega = 0;
bool ultimoEstadoBotonAbrir = HIGH;
bool ultimoEstadoBotonCerrar = HIGH;

void setup() {
  Serial.begin(115200);
  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2); // 9600 hacia el Mega

  pinMode(PIN_BOTON_ABRIR, INPUT_PULLUP);
  pinMode(PIN_BOTON_CERRAR, INPUT_PULLUP);

  Serial.println("=== ESP32 Gateway Puente + Elevador + Central Park ===");
  conectarWiFi();
  iniciarServidorHTTP();
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) conectarWiFi();

  server.handleClient();
  procesarBotonesLocales();

  if (millis() - ultimoPingMega > INTERVALO_PING) {
    ultimoPingMega = millis();
    ciclosEsp32++;
    Serial2.println("PING");
  }

  if (millis() - ultimoPoll > INTERVALO_POLL) {
    ultimoPoll = millis();
    consultarInstruccionesPendientes();
  }

  if (millis() - ultimoEstadoMega > INTERVALO_ESTADO && megaOnline && !procesandoInstruccion) {
    ultimoEstadoMega = millis();
    Serial2.println("STATUS_ALL");
  }

  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();
    if (cmd.length() > 0) Serial2.println(cmd);
  }

  if (Serial2.available()) {
    String linea = Serial2.readStringUntil('\n');
    linea.trim();
    if (linea.length() > 0) {
      ultimoMensajeMega = millis();
      megaOnline = true;
      Serial.print("[Mega] "); Serial.println(linea);
      procesarMensajeMega(linea);
    }
  }

  if (megaOnline && (millis() - ultimoMensajeMega > TIMEOUT_MEGA)) {
    megaOnline = false;
    esclavo1Online = false;
    esclavo2Online = false;
    esclavo3Online = false;
    Serial.println("[ALERTA] Mega no responde");
  }
}

void conectarWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  unsigned long inicio = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - inicio < 15000) {
    delay(500);
    Serial.print(".");
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("\nWiFi OK. IP: "); Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nWiFi FALLO");
  }
}

void procesarMensajeMega(String msg) {
  megaOnline = true;
  ultimoMensajeMega = millis();

  if (msg.startsWith("ESCLAVO1=")) { esclavo1Online = (msg.substring(9) == "OK"); return; }
  if (msg.startsWith("ESCLAVO2=")) { esclavo2Online = (msg.substring(9) == "OK"); return; }
  if (msg.startsWith("ESCLAVO3=")) { esclavo3Online = (msg.substring(9) == "OK"); return; }

  if (msg.startsWith("CICLO_MEGA:")) { ciclosMega = msg.substring(11).toInt(); return; }

  if (msg.startsWith("SEMAFORO:")) {
    actualizarEstadoSemaforo(msg.substring(9));
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
    if (resp.startsWith("FUENTE:")) { esclavo3Online = true; actualizarEstadoFuente(resp); return; }
    if (resp.startsWith("ERROR")) { Serial.print(">> Error del Mega: "); Serial.println(resp); return; }
    return;
  }

  if (msg.startsWith("ESTADO:")) {
    actualizarEstadoGeneral(msg.substring(7));
    enviarEstadoAlBackend(msg.substring(7));
    return;
  }
}

void actualizarEstadoFuente(String resp) {
  fuenteBomba = valorEnCampo(resp, "FUENTE:");
  String agua = valorEnCampo(resp, "AGUA:");
  if (agua.length() > 0) fuenteAgua = agua.toInt();
  fuenteLuces = valorEnCampo(resp, "LUCES:");
}

void actualizarEstadoSemaforo(String resp) {
  semaforoNS = valorEnCampo(resp, "NS=");
  semaforoEO = valorEnCampo(resp, "EO=");
  semaforoModo = valorEnCampo(resp, "MODO=");
}

void actualizarEstadoGeneral(String estado) {
  ultimaLineaEstado = estado;
  String angulo = valorEnCampo(estado, "PUENTE.ANGULO=");
  if (angulo.length() > 0) anguloActualServo = angulo.toInt();
  estadoPuente = valorEnCampo(estado, "PUENTE.ESTADO=");
  String pasos = valorEnCampo(estado, "TELEFERICO.PASOS=");
  if (pasos.length() > 0) pasosTeleferico = pasos.toInt();
  estadoTeleferico = valorEnCampo(estado, "TELEFERICO.MOVIMIENTO=");
  String nivel = valorEnCampo(estado, "ELEVADOR.NIVEL=");
  if (nivel.length() > 0) nivelActualElevador = nivel.toInt();
  fuenteBomba = valorEnCampo(estado, "FUENTE.BOMBA=");
  String agua = valorEnCampo(estado, "FUENTE.AGUA=");
  if (agua.length() > 0) fuenteAgua = agua.toInt();
  fuenteLuces = valorEnCampo(estado, "FUENTE.LUCES=");
  semaforoNS = valorEnCampo(estado, "SEMAFORO.NS=");
  semaforoEO = valorEnCampo(estado, "SEMAFORO.EO=");
  semaforoModo = valorEnCampo(estado, "SEMAFORO.MODO=");
}

String valorEnCampo(String texto, String clave) {
  int pos = texto.indexOf(clave);
  if (pos < 0) return "";
  int inicio = pos + clave.length();
  int fin = texto.indexOf(';', inicio);
  if (fin < 0) fin = texto.length();
  return texto.substring(inicio, fin);
}

void enviarEstadoAlBackend(String estado) {
  if (WiFi.status() != WL_CONNECTED || estado.length() == 0) return;
  HTTPClient http;
  http.begin(String(BACKEND_URL) + "/api/estado");
  http.addHeader("Content-Type", "application/json");
  http.setTimeout(5000);
  String json = "{\"ipDispositivo\":\"" + WiFi.localIP().toString() + "\",\"estado\":" + construirJsonEstado(estado) + "}";
  int httpCode = http.POST(json);
  http.end();
}

String construirJsonEstado(String estado) {
  String json = "{";
  int inicio = 0;
  bool primero = true;
  while (inicio < estado.length()) {
    int separador = estado.indexOf(';', inicio);
    String par = (separador < 0) ? estado.substring(inicio) : estado.substring(inicio, separador);
    int igual = par.indexOf('=');
    if (igual > 0) {
      String clave = par.substring(0, igual);
      String valor = par.substring(igual + 1);
      if (!primero) json += ",";
      json += "\"" + clave + "\":\"" + valor + "\"";
      primero = false;
    }
    if (separador < 0) break;
    inicio = separador + 1;
  }
  json += "}";
  return json;
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

bool esperarRespuestaMega(unsigned long timeout) {
  unsigned long inicio = millis();
  bool huboRespuesta = false;
  while (millis() - inicio < timeout) {
    if (Serial2.available()) {
      String linea = Serial2.readStringUntil('\n');
      linea.trim();
      if (linea.length() > 0) {
        ultimoMensajeMega = millis();
        megaOnline = true;
        if (linea.startsWith("RESP ")) huboRespuesta = true;
        procesarMensajeMega(linea);
      }
    }
    server.handleClient();
    delay(10);
  }
  return huboRespuesta;
}

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
  nombre.trim();

  if (!megaOnline) { enviarRespuestaInstruccion(id, "ERROR", 0); return; }

  procesandoInstruccion = true;
  Serial2.println(nombre);
  unsigned long inicio = millis();
  bool huboRespuesta = esperarRespuestaMega(3500);
  unsigned long tiempo = millis() - inicio;
  procesandoInstruccion = false;

  bool success = megaOnline && huboRespuesta;
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

void iniciarServidorHTTP() {
  server.on("/abrir", HTTP_POST, [](){ if(megaOnline){ Serial2.println("SERVO_SUBIR"); esperarRespuestaMega(3500); server.send(200, "application/json", "{\"estado\":\"OK\"}"); } else server.send(503, "application/json", "{}"); });
  server.on("/cerrar", HTTP_POST, [](){ if(megaOnline){ Serial2.println("SERVO_BAJAR"); esperarRespuestaMega(3500); server.send(200, "application/json", "{\"estado\":\"OK\"}"); } else server.send(503, "application/json", "{}"); });
  server.on("/fuente/encender", HTTP_POST, [](){ if(megaOnline){ Serial2.println("FUENTE_ENCENDER"); esperarRespuestaMega(2000); server.send(200, "application/json", "{\"estado\":\"OK\"}"); } else server.send(503, "application/json", "{}"); });
  server.on("/fuente/apagar", HTTP_POST, [](){ if(megaOnline){ Serial2.println("FUENTE_APAGAR"); esperarRespuestaMega(2000); server.send(200, "application/json", "{\"estado\":\"OK\"}"); } else server.send(503, "application/json", "{}"); });
  server.on("/fuente/status", HTTP_GET, [](){ server.send(200, "application/json", estadoFuenteJson()); });
  server.on("/luces/on", HTTP_POST, [](){ if(megaOnline){ Serial2.println("JARDIN_LUCES_ON"); esperarRespuestaMega(2000); server.send(200, "application/json", "{\"estado\":\"OK\"}"); } else server.send(503, "application/json", "{}"); });
  server.on("/luces/off", HTTP_POST, [](){ if(megaOnline){ Serial2.println("JARDIN_LUCES_OFF"); esperarRespuestaMega(2000); server.send(200, "application/json", "{\"estado\":\"OK\"}"); } else server.send(503, "application/json", "{}"); });
  server.on("/semaforo/auto", HTTP_POST, [](){ if(megaOnline){ Serial2.println("SEMAFORO_AUTO"); esperarRespuestaMega(2000); server.send(200, "application/json", "{\"estado\":\"OK\"}"); } else server.send(503, "application/json", "{}"); });
  server.on("/semaforo/parar", HTTP_POST, [](){ if(megaOnline){ Serial2.println("SEMAFORO_PARAR"); esperarRespuestaMega(2000); server.send(200, "application/json", "{\"estado\":\"OK\"}"); } else server.send(503, "application/json", "{}"); });
  server.on("/estado", HTTP_GET, [](){
    String json = "{\"estado\":\"" + estadoPuente + "\",\"angulo\":" + String(anguloActualServo) +
                  ",\"teleferico\":\"" + estadoTeleferico + "\"" + ",\"pasos\":" + String(pasosTeleferico) +
                  ",\"nivel\":" + String(nivelActualElevador) + ",\"fuente\":" + estadoFuenteJson() +
                  ",\"semaforoNS\":\"" + semaforoNS + "\"" + ",\"semaforoEO\":\"" + semaforoEO + "\"" +
                  ",\"semaforoModo\":\"" + semaforoModo + "\"" + ",\"megaOnline\":" + String(megaOnline ? "true" : "false") +
                  ",\"esclavoOnline\":" + String(esclavo1Online ? "true" : "false") + "}";
    server.send(200, "application/json", json);
  });
  server.on("/verificar", HTTP_GET, [](){ server.send(200, "application/json", "{\"estado\":\"OK\",\"megaOnline\":" + String(megaOnline ? "true" : "false") + "}"); });
  server.begin();
}

String estadoFuenteJson() {
  return "{\"bomba\":\"" + fuenteBomba + "\",\"agua\":" + String(fuenteAgua) + ",\"luces\":\"" + fuenteLuces + "\"}";
}