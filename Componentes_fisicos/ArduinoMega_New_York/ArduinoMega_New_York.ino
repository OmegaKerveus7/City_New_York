void setup() {
  Serial.begin(115200);   // Monitor USB
  Serial1.begin(9600);    // UART al ESP32
  delay(1500);
  Serial.println("=== Mega: prueba de conteo ===");
}

void loop() {
  // 1) Escuchar respuestas del ESP32
  while (Serial1.available()) {
    String linea = Serial1.readStringUntil('\n');
    linea.trim();
    if (linea.length() > 0) {
      Serial.print("[ESP32] ");
      Serial.println(linea);
    }
  }

  // 2) Enviar un número cada 1 segundo
  static unsigned long ultimoEnvio = 0;
  static int contador = 0;

  if (millis() - ultimoEnvio > 1000) {
    ultimoEnvio = millis();
    contador++;

    Serial.print(">> Enviando número: ");
    Serial.println(contador);

    Serial1.println(contador);   // envía "1\n", "2\n", ...
  }
}