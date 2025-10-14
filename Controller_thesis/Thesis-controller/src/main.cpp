#define LED_BLUE 23
#define LED_GREEN 22

#include <Arduino.h>
void setup() {
  pinMode(LED_BLUE, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  Serial.begin(115200);
  Serial.println("Test LED Grove iniziato!");
}

void loop() {
  Serial.println("Accendo LED blu...");
  digitalWrite(LED_BLUE, HIGH);
  delay(1000);

  Serial.println("Accendo LED verde...");
  digitalWrite(LED_GREEN, HIGH);
  delay(1000);

  Serial.println("Spengo LED blu...");
  digitalWrite(LED_BLUE, LOW);
  delay(1000);

  Serial.println("Spengo LED verde...");
  digitalWrite(LED_GREEN, LOW);
  delay(1000);
}
