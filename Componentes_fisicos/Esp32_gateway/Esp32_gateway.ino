#define RXD2 16
#define TXD2 17

int contadorRecibidos = 0;

void setup() {
  Serial.begin(115200);
  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);
  Serial.println("=== ESP32: prueba de conteo ===");
}

void loop() {
  if (Serial2.available()) {
    String linea = Serial2.readStringUntil('\n');
    linea.trim();

    if (linea.length() > 0) {
      int numero = linea.toInt();   // convierte "5" → 5
      contadorRecibidos++;

      Serial.print("[Mega] número recibido: ");
      Serial.print(numero);
      Serial.print("  |  total recibidos: ");
      Serial.println(contadorRecibidos);

      // Responder al Mega con el total
      Serial2.print("RECIBIDO ");
      Serial2.print(numero);
      Serial2.print(" TOTAL ");
      Serial2.println(contadorRecibidos);
    }
  }
}