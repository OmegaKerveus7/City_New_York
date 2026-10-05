#include <Wire.h>

#define I2C_ADDRESS 0x08

void setup() {
  Wire.begin(I2C_ADDRESS);        // se une al bus como esclavo
  Wire.onRequest(requestEvent);   // cuando el Mega pida datos
  Wire.onReceive(receiveEvent);   // cuando el Mega envíe datos
  Serial.begin(9600);
  Serial.println("Esclavo 1 listo (I2C addr 0x08)");
}

void loop() {
  delay(100);
}

// El Mega pide datos → respondemos algo
void requestEvent() {
  Wire.write("ESCLAVO1_OK");   // 11 bytes máximo por respuesta
}

// El Mega manda datos → los mostramos
void receiveEvent(int howMany) {
  String recibido = "";
  while (Wire.available()) {
    recibido += (char)Wire.read();
  }
  Serial.print("Comando recibido del Mega: ");
  Serial.println(recibido);
}