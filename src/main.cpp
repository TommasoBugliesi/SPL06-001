#include <Arduino.h>
#include <Wire.h>
#include <SPL06-001.h>

#define SDA_PIN 05 // Example alternative pin for SDA
#define SCL_PIN 06 // Example alternative pin for SCL

SPL06_001 spl06_001(0x76, &Wire); // ADDR: 0 => 0x77, ADDR: 1 => 0x76

void setup() {
  Serial.begin(115200);

  Wire.begin(SDA_PIN, SCL_PIN);
  // while (!spl06_001.begin()) {
  //   Serial.println("SPL06-001 not found !");
  //   delay(1000);
  // }
}

void loop() {
  while (!spl06_001.begin()) {
    Serial.println("SPL06-001 not found !");
    delay(1000);
  }
  Serial.println("SPL06-001 connected!");
  delay(1000);
}