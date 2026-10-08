// ============================================
// MEGA - Puente entre ESP32 (UART) y Esclavo (I2C)
// ============================================
#include <Wire.h>

#define ESCLAVO1_ADDR   0x08
#define TIMEOUT_I2C     500
#define TIEMPO_SERVO    2500   // ms para que el servo termine

void setup() {
  Serial.begin(115200);
  Serial1.begin(9600);
  Wire.begin();
  
  delay(1000);
  Serial.println("=== MEGA LISTO ===");
  Serial.println("Serial1 (ESP32): 9600 baud");
  Serial.println("I2C maestro inicializado");
  Serial.println("");
  
  escanearI2C();
  Serial.println("");
  Serial.println("Listo para recibir comandos del ESP32...");
}

void loop() {
  if (Serial1.available()) {
    String cmd = Serial1.readStringUntil('\n');
    cmd.trim();
    cmd.toUpperCase();
    
    if (cmd.length() == 0) return;
    
    Serial.print("[ESP32 -> Mega] ");
    Serial.println(cmd);
    
    // PING directo
    if (cmd == "PING") {
      Serial1.println("PONG_MEGA");
      Serial.println(">> Respondido: PONG_MEGA");
      return;
    }
    
    // Todo lo demas va al esclavo
    enviarAlEsclavo(cmd);
  }
}

void enviarAlEsclavo(String cmd) {
  // 1. Enviar comando
  Wire.beginTransmission(ESCLAVO1_ADDR);
  Wire.write(cmd.c_str());
  byte err = Wire.endTransmission();
  
  if (err != 0) {
    Serial.print(">> ERROR I2C codigo: ");
    Serial.println(err);
    Serial1.println("RESP ERROR:I2C");
    return;
  }
  
  // 2. Esperar a que el servo termine de moverse
  delay(TIEMPO_SERVO);
  
  // 3. Leer respuesta
  String resp = leerRespuestaEsclavo();
  
  if (resp.length() == 0) {
    Serial.println(">> Sin respuesta del esclavo");
    Serial1.println("RESP ERROR:SIN_RESPUESTA");
    return;
  }
  
  Serial.print(">> Esclavo respondio: ");
  Serial.println(resp);
  
  // 4. Reenviar al ESP32
  Serial1.print("RESP ");
  Serial1.println(resp);
}

String leerRespuestaEsclavo() {
  Wire.requestFrom(ESCLAVO1_ADDR, 16);
  
  String resp = "";
  unsigned long inicio = millis();
  
  while (Wire.available() && (millis() - inicio < TIMEOUT_I2C)) {
    char c = Wire.read();
    if (c == '\n' || c == '\r') break;
    if (c >= 32 && c <= 126) resp += c;
  }
  
  resp.trim();
  return resp;
}

void escanearI2C() {
  Serial.println("Escaneando bus I2C...");
  byte encontrados = 0;
  
  for (byte addr = 1; addr < 127; addr++) {
    Wire.beginTransmission(addr);
    if (Wire.endTransmission() == 0) {
      Serial.print("  >> Encontrado: 0x");
      if (addr < 16) Serial.print("0");
      Serial.println(addr, HEX);
      encontrados++;
    }
  }
  
  if (encontrados == 0) {
    Serial.println("  >> NINGUN dispositivo I2C encontrado!");
    Serial.println("  >> Revisa SDA(20)/SCL(21)/GND y pull-ups");
  } else {
    Serial.print("  >> Total dispositivos: ");
    Serial.println(encontrados);
  }
}