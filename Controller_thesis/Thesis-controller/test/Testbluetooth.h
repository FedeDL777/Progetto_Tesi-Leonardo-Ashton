#include <esp_now.h>
#include <WiFi.h>

uint8_t receiverAddress[] = {0xF4, 0x65, 0x0B, 0x41, 0x43, 0xDC}; //MAC dell'altra ESP32
int counter = 0;

// Callback di conferma invio
void onSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("📤 Invio a ");
  for (int i = 0; i < 6; i++) {
    Serial.printf("%02X", mac_addr[i]);
    if (i < 5) Serial.print(":");
  }
  Serial.print(" → ");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "✅ OK" : "❌ Fallito");
}

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);

  if (esp_now_init() != ESP_OK) {
    Serial.println("❌ Errore inizializzazione ESP-NOW");
    return;
  }

  esp_now_register_send_cb(onSent);

  // Registra peer (ESP ricevente)
  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, receiverAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("❌ Errore aggiunta peer");
    return;
  }

  Serial.println("✅ ESP-NOW Sender pronto!");
}

void loop() {
  char msg[32];
  snprintf(msg, sizeof(msg), "Hello %d", counter++);
  esp_err_t result = esp_now_send(receiverAddress, (uint8_t *)msg, strlen(msg) + 1);

  if (result == ESP_OK) {
    Serial.printf("📨 Inviato: %s\n", msg);
  } else {
    Serial.printf("⚠️ Errore invio: %d\n", result);
  }

  delay(1000);
}