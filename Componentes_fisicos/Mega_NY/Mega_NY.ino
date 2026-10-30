#include <Wire.h>

#define ESCLAVO1_ADDR 0x08 // Puente / Teleférico
#define ESCLAVO2_ADDR 0x09 // Elevador
#define ESCLAVO3_ADDR 0x0A // Central Park (fuente + luces)
#define PIN_LED_MEGA  13

bool esclavo1Online = false;
bool esclavo2Online = false;
bool esclavo3Online = false;
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

#define TIEMPO_VERDE_MS     150000 // 2.5 minutos
#define TIEMPO_AMARILLO_MS  30000  // 30 segundos

unsigned long tiempoSemaforo = 0;
int faseSemaforo = 1;
int faseAnterior = 0;

bool modoAuto = true;
String colorNS = "ROJO";
String colorEO = "ROJO";

void setup() {
  Serial.begin(115200);       
  Serial1.begin(9600);        // Hacia el ESP32 (Pines 18 y 19)
  Wire.begin();               // I2C Maestro (Pines 20 y 21)
  
  pinMode(PIN_LED_MEGA, OUTPUT);
  digitalWrite(PIN_LED_MEGA, LOW);

  pinMode(pinRojo1, OUTPUT); pinMode(pinAmarillo1, OUTPUT); pinMode(pinVerde1, OUTPUT);
  pinMode(pinRojo2, OUTPUT); pinMode(pinAmarillo2, OUTPUT); pinMode(pinVerde2, OUTPUT);

  apagarSemaforos();

  delay(1000);
  Serial.println("=== MEGA CONTROL MAESTRO y SEMAFOROS ===");
  chequearEsclavos();
  tiempoSemaforo = millis();
}

void loop() {
  if (Serial1.available()) {
    String cmd = Serial1.readStringUntil('\n');
    cmd.trim();          
    cmd.toUpperCase();   
    if (cmd.length() > 0) {
      Serial.print("[ESP32] "); Serial.println(cmd);
      procesarComandoESP32(cmd);
    }
  }

  static unsigned long ultimoChequeo = 0;
  if (millis() - ultimoChequeo > 5000) {
    ultimoChequeo = millis();
    chequearEsclavos();
  }

  actualizarSemaforos();
}

void procesarComandoESP32(String cmd) {
  if (cmd == "PING") {
    ciclosMega++; 
    Serial1.print("CICLO_MEGA:"); Serial1.println(ciclosMega);
    return;
  }

  if (cmd == "STATUS_ALL") {
    Serial1.println(construirEstadoGeneral());
    return;
  }

  if (cmd.startsWith("SEMAFORO_")) {
    procesarSemaforo(cmd);
    return;
  }

  // --- PUENTE ---
  if (cmd == "SERVO_SUBIR" || cmd == "SERVO_BAJAR" || cmd == "SERVO_STATUS" || cmd.startsWith("SERVO ")) {
    if (!esclavo1Online) { Serial1.println("RESP ERROR:ESCLAVO1_OFFLINE"); return; }
    enviarI2C(ESCLAVO1_ADDR, cmd);
    return;
  }

  // --- ELEVADOR ---
  if (cmd.startsWith("SUBIR_NIVEL") || cmd.startsWith("BAJAR_NIVEL") || cmd.startsWith("IR_A_NIVEL") || 
      cmd == "ELEVADOR_PARAR" || cmd == "ELEVADOR_STATUS" || cmd == "CALIBRAR") {
    if (!esclavo2Online) { Serial1.println("RESP ERROR:ESCLAVO2_OFFLINE"); return; }
    enviarI2C(ESCLAVO2_ADDR, cmd);
    return;
  }

  // --- TELEFERICO ---
  if (cmd.startsWith("TELEFERICO_AVANZAR") || cmd.startsWith("TELEFERICO_REGRESAR") || 
      cmd == "TELEFERICO_PARAR" || cmd == "TELEFERICO_STATUS") {
    if (!esclavo1Online) { Serial1.println("RESP ERROR:ESCLAVO1_OFFLINE"); return; }
    enviarI2C(ESCLAVO1_ADDR, cmd);
    return;
  }

  // --- CENTRAL PARK ---
  if (cmd == "CENTRAL_ON" || cmd == "CENTRAL_OFF" || cmd == "ENCENDER" || cmd == "APAGAR" ||
      cmd.startsWith("FUENTE") || cmd.startsWith("JARDIN")) {
    if (!esclavo3Online) { Serial1.println("RESP ERROR:ESCLAVO3_OFFLINE"); return; }
    enviarI2C(ESCLAVO3_ADDR, cmd);
    return;
  }

  Serial.print(">> Comando desconocido: ["); Serial.print(cmd); Serial.println("]");
  Serial1.println("RESP ERROR:CMD_DESCONOCIDO");
}

void procesarSemaforo(String cmd) {
  if (cmd == "SEMAFORO_AUTO") {
    modoAuto = true; tiempoSemaforo = millis();
    Serial1.println("RESP OK:SEMAFORO_AUTO"); return;
  }
  if (cmd == "SEMAFORO_PARAR") {
    modoAuto = false;
    Serial1.println("RESP OK:SEMAFORO_PARAR"); return;
  }
  if (cmd.startsWith("SEMAFORO_NS_")) {
    modoAuto = false; aplicarColorCarril1(colorDesdeComando(cmd.substring(12)));
    Serial1.println("RESP OK:" + cmd); return;
  }
  if (cmd.startsWith("SEMAFORO_EO_")) {
    modoAuto = false; aplicarColorCarril2(colorDesdeComando(cmd.substring(12)));
    Serial1.println("RESP OK:" + cmd); return;
  }
  Serial1.println("RESP ERROR:SEMAFORO_DESCONOCIDO");
}

String colorDesdeComando(String texto) {
  if (texto.startsWith("ROJO")) return "ROJO";
  if (texto.startsWith("AMARILLO")) return "AMARILLO";
  if (texto.startsWith("VERDE")) return "VERDE";
  return "APAGADO";
}

void aplicarColorCarril1(String color) {
  colorNS = color;
  digitalWrite(pinRojo1,     color == "ROJO"     ? HIGH : LOW);
  digitalWrite(pinAmarillo1, color == "AMARILLO" ? HIGH : LOW);
  digitalWrite(pinVerde1,    color == "VERDE"    ? HIGH : LOW);
}

void aplicarColorCarril2(String color) {
  colorEO = color;
  digitalWrite(pinRojo2,     color == "ROJO"     ? HIGH : LOW);
  digitalWrite(pinAmarillo2, color == "AMARILLO" ? HIGH : LOW);
  digitalWrite(pinVerde2,    color == "VERDE"    ? HIGH : LOW);
}

void enviarI2C(uint8_t direccion, String cmd) {
  Wire.beginTransmission(direccion);
  Wire.write(cmd.c_str());
  byte err = Wire.endTransmission();
  if (err != 0) { Serial1.println("RESP ERROR:I2C_WRITE"); return; }

  unsigned long inicio = millis();
  while (millis() - inicio < 50) {
    if (Serial1.available()) { String extra = Serial1.readStringUntil('\n'); extra.trim(); }
    yield();
  }

  Wire.requestFrom(direccion, 64);
  String resp = "";
  while (Wire.available()) {
    char c = Wire.read();
    if (c == '\n') break; 
    resp += c;
  }
  resp.trim();

  if (resp.length() > 0) {
    Serial1.print("RESP "); Serial1.println(resp);
  } else {
    Serial1.println("RESP ERROR:SIN_RESPUESTA");
  }
}

bool pingI2C(uint8_t addr) {
  Wire.beginTransmission(addr);
  return (Wire.endTransmission() == 0);
}

void chequearEsclavos() {
  bool e1 = pingI2C(ESCLAVO1_ADDR);
  bool e2 = pingI2C(ESCLAVO2_ADDR);
  bool e3 = pingI2C(ESCLAVO3_ADDR);

  if (e1 != esclavo1Online) { esclavo1Online = e1; Serial1.print("ESCLAVO1="); Serial1.println(esclavo1Online ? "OK" : "ERROR"); }
  if (e2 != esclavo2Online) { esclavo2Online = e2; Serial1.print("ESCLAVO2="); Serial1.println(esclavo2Online ? "OK" : "ERROR"); }
  if (e3 != esclavo3Online) { esclavo3Online = e3; Serial1.print("ESCLAVO3="); Serial1.println(esclavo3Online ? "OK" : "ERROR"); }

  Serial.print("[I2C] E1: "); Serial.print(e1 ? "OK" : "FAIL");
  Serial.print(" | E2: "); Serial.print(e2 ? "OK" : "FAIL");
  Serial.print(" | E3: "); Serial.println(e3 ? "OK" : "FAIL");
}

void actualizarSemaforos() {
  if (!modoAuto) return;
  unsigned long tiempoActual = millis();
  unsigned long t = tiempoActual - tiempoSemaforo;

  switch (faseSemaforo) {
    case 1: if (t >= TIEMPO_VERDE_MS) { faseSemaforo = 2; tiempoSemaforo = tiempoActual; } break;
    case 2: if (t >= TIEMPO_AMARILLO_MS) { faseSemaforo = 3; tiempoSemaforo = tiempoActual; } break;
    case 3: if (t >= TIEMPO_VERDE_MS) { faseSemaforo = 4; tiempoSemaforo = tiempoActual; } break;
    case 4: if (t >= TIEMPO_AMARILLO_MS) { faseSemaforo = 1; tiempoSemaforo = tiempoActual; } break;
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
    colorNS = "VERDE"; colorEO = "ROJO";
  } else if (fase == 2) {
    digitalWrite(pinRojo1, LOW); digitalWrite(pinAmarillo1, HIGH); digitalWrite(pinVerde1, LOW);
    digitalWrite(pinRojo2, HIGH); digitalWrite(pinAmarillo2, LOW); digitalWrite(pinVerde2, LOW);
    colorNS = "AMARILLO"; colorEO = "ROJO";
  } else if (fase == 3) {
    digitalWrite(pinRojo1, HIGH); digitalWrite(pinAmarillo1, LOW); digitalWrite(pinVerde1, LOW);
    digitalWrite(pinRojo2, LOW); digitalWrite(pinAmarillo2, LOW); digitalWrite(pinVerde2, HIGH);
    colorNS = "ROJO"; colorEO = "VERDE";
  } else if (fase == 4) {
    digitalWrite(pinRojo1, HIGH); digitalWrite(pinAmarillo1, LOW); digitalWrite(pinVerde1, LOW);
    digitalWrite(pinRojo2, LOW); digitalWrite(pinAmarillo2, HIGH); digitalWrite(pinVerde2, LOW);
    colorNS = "ROJO"; colorEO = "AMARILLO";
  }
}

void apagarSemaforos() {
  digitalWrite(pinRojo1, LOW); digitalWrite(pinAmarillo1, LOW); digitalWrite(pinVerde1, LOW);
  digitalWrite(pinRojo2, LOW); digitalWrite(pinAmarillo2, LOW); digitalWrite(pinVerde2, LOW);
}

void enviarEstadoSemaforosESP32(int fase) {
  String msg = "SEMAFORO:NS=" + colorNS + "|EO=" + colorEO + "|MODO=" + String(modoAuto ? "AUTO" : "MANUAL") + "|FASE=" + String(fase);
  Serial1.println(msg);
}

String construirEstadoGeneral() {
  String e1 = leerEsclavo(ESCLAVO1_ADDR);
  String e2 = leerEsclavo(ESCLAVO2_ADDR);
  String e3 = leerEsclavo(ESCLAVO3_ADDR);

  int angulo = valorDe(e1, "POS:").toInt();
  int pasosTeleferico = valorDe(e1, "PAS:").toInt();
  String movTeleferico = valorDe(e1, "TELEFERICO:");
  int nivel = valorDe(e2, "NIVEL:").toInt();
  String bomba = valorDe(e3, "FUENTE:");
  int agua = valorDe(e3, "AGUA:").toInt();
  String luces = valorDe(e3, "LUCES:");

  String estado = "ESTADO:";
  estado += "PUENTE.ANGULO=" + String(angulo);
  estado += ";PUENTE.ESTADO=" + estadoPuente(angulo);
  estado += ";TELEFERICO.PASOS=" + String(pasosTeleferico);
  estado += ";TELEFERICO.MOVIMIENTO=" + (movTeleferico.length() ? movTeleferico : "DETENIDO");
  estado += ";ELEVADOR.NIVEL=" + String(nivel);
  estado += ";FUENTE.BOMBA=" + (bomba.length() ? bomba : "OFF");
  estado += ";FUENTE.AGUA=" + String(agua);
  estado += ";FUENTE.LUCES=" + (luces.length() ? luces : "OFF");
  estado += ";SEMAFORO.NS=" + colorNS;
  estado += ";SEMAFORO.EO=" + colorEO;
  estado += ";SEMAFORO.MODO=" + String(modoAuto ? "AUTO" : "MANUAL");
  return estado;
}

String estadoPuente(int angulo) {
  if (angulo >= 110 && angulo <= 130) return "ABIERTO";
  if (angulo >= 170) return "CERRADO";
  return "INTERMEDIO";
}

String valorDe(String texto, String prefijo) {
  int pos = texto.indexOf(prefijo);
  if (pos < 0) return "";
  int inicio = pos + prefijo.length();
  int fin = texto.indexOf('|', inicio);
  if (fin < 0) fin = texto.length();
  return texto.substring(inicio, fin);
}

String leerEsclavo(uint8_t direccion) {
  Wire.requestFrom(direccion, 64);
  String respuesta = "";
  while (Wire.available()) {
    char c = Wire.read();
    if (c == '\n') break;
    respuesta += c;
  }
  respuesta.trim();
  return respuesta;
}