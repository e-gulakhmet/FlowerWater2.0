#include <Arduino.h>

void setup() {
  pinMode(0, OUTPUT);
  pinMode(1, OUTPUT);
}

void loop() {
  digitalWrite(0, LOW);
  digitalWrite(1, LOW);
}