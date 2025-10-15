#include <esp_now.h>
#include <WiFi.h>

void onReceive(const uint8_t *mac, const uint8_t *incomingData, int len) {
  Serial.print("Messaggio ricevuto da ");
  for (int i = 0; i < 6; i++) {
    Serial.printf("%02X", mac[i]);
    if (i < 5) Serial.print(":");
  }
  Serial.print(" → ");
  Serial.printf("%.*s\n", len, incomingData);
}

void setup() {
  Serial.begin(115200);
  Serial.println("=== Ricevitore ESP-NOW (Braccio Robotico) ===");

  WiFi.mode(WIFI_STA);

  if (esp_now_init() != ESP_OK) {
    Serial.println("Errore inizializzazione ESP-NOW");
    return;
  }

  esp_now_register_recv_cb(onReceive);
  Serial.println("In ascolto dei messaggi...");
}

void loop() {
  delay(100);
}
