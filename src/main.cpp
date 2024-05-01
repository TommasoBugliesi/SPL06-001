#include <Arduino.h>
#include <Wire.h>
#include <SPL06-001.h>

#define SDA_PIN 05 // Example alternative pin for SDA
#define SCL_PIN 06 // Example alternative pin for SCL

SPL06_001 spl06_001(0x76, &Wire); // ADDR: 0 => 0x77, ADDR: 1 => 0x76

void setup() {
  Serial.begin(115200);

  Wire.begin(SDA_PIN, SCL_PIN);
  while (!spl06_001.begin()) {
    Serial.println("SPL06-001 not found !");
    delay(1000);
  }
}

void loop() {
  if (spl06_001.measure()) {
    Serial.print("Pressure: ");
    Serial.print(spl06_001.pressure() * 0.01, 5); // 1 Pa = 0.01 mbar so Pa * 0.01 = mBar
    Serial.print(" mbar\tTemperature: ");
    Serial.print(spl06_001.temperature(), 4);
    Serial.print(" *C");
    Serial.println();
  } else {
    Serial.println("SPL06-001 read error");
  }
  delay(1000);
}