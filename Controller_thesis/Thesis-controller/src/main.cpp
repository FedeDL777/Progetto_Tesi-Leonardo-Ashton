#include <esp_now.h>
#include <WiFi.h>
#include "include/Button.h"
#include "include/set_up.h"

uint8_t receiverAddress[] = {0xF4, 0x65, 0x0B, 0x41, 0x43, 0xDC};
int counter = 0;
unsigned long lastSend = 0;
const unsigned long sendInterval = 200; // ms tra un invio e l’altro

// Callback di conferma invio
void onSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("Invio a ");
  for (int i = 0; i < 6; i++) {
    Serial.printf("%02X", mac_addr[i]);
    if (i < 5) Serial.print(":");
  }
  Serial.print(" → ");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "OK" : "Fallito");
}

Button base_sx(YELLOW_BASE_SX);
Button base_dx(YELLOW_BASE_DX);
Button elbow_sx(WHITE_ELBOW_SX);
Button elbow_dx(WHITE_ELBOW_DX);
Button wrist_sx(BLUE_WRIST_SX);
Button wrist_dx(BLUE_WRIST_DX);

struct BtnMap {
  Button* btn;
  const char* label;
};

BtnMap buttons[] = {
  { &base_sx, "Base SX" },
  { &base_dx, "Base DX" },
  { &elbow_sx, "Elbow SX" },
  { &elbow_dx, "Elbow DX" },
  { &wrist_sx, "Wrist SX" },
  { &wrist_dx, "Wrist DX" }
};

void setup() {
  Serial.begin(115200);
  Serial.println("=== Test pulsanti ESP32 espNOW ===");

  WiFi.mode(WIFI_STA);
  if (esp_now_init() != ESP_OK) {
    Serial.println("Errore inizializzazione ESP-NOW");
    return;
  }

  esp_now_register_send_cb(onSent);

  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, receiverAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Errore aggiunta peer");
    return;
  }

  Serial.println("ESP-NOW Sender pronto!");
}

void loop() {
  String stato = "";
  char msg[64];

  for (auto &b : buttons) {
    b.btn->update();
    if (b.btn->wasPressed()) {
      stato += String(b.label) + " ";
    }
  }

  if (stato != "" && millis() - lastSend > sendInterval) {
    snprintf(msg, sizeof(msg), "%s", stato.c_str());
    esp_err_t result = esp_now_send(receiverAddress, (uint8_t *)msg, strlen(msg) + 1);
    lastSend = millis();

    if (result == ESP_OK)
      Serial.printf("Inviato: %s\n", msg);
    else
      Serial.printf("Errore invio: %d\n", result);
  }
}
