/*******************************************************************************
 * CONTROLLER ESP32 - ESP-NOW SENDER (VERSIONE CON PRESSIONE CONTINUA)
 * 
 * Controller wireless per braccio robotico
 * Usa 8 pulsanti per controllare Base, Elbow, Wrist, Claw
 * Invia comandi via ESP-NOW al braccio robotico
 * 
 * NOVITÀ: Pressione continua invia comandi ripetuti
 * 
 * Autore: Federico De Leonardis
 ******************************************************************************/

#include <esp_now.h>
#include <WiFi.h>
#include "include/Button.h"
#include "include/set_up.h"

// ============================================================================
// PARAMETRI CONFIGURABILI
// ============================================================================

const unsigned long SEND_INTERVAL = 100;           // ms tra invii (10Hz)
const unsigned long HEARTBEAT_INTERVAL = 2000;     // ms tra heartbeat (0.5Hz)
const unsigned long REPEAT_INTERVAL = 200;         // ms tra comandi ripetuti durante pressione continua

// ============================================================================
// VARIABILI GLOBALI
// ============================================================================

// Pulsanti con debouncing automatico
Button base_sx(YELLOW_BASE_SX, false);
Button base_dx(YELLOW_BASE_DX, false);
Button elbow_sx(WHITE_ELBOW_SX, false);
Button elbow_dx(WHITE_ELBOW_DX, false);
Button wrist_sx(BLUE_WRIST_SX, false);
Button wrist_dx(BLUE_WRIST_DX, false);
Button claw_sx(YELLOW_CLAW_SX, false);
Button claw_dx(YELLOW_CLAW_DX, false);

// Struttura mappatura pulsanti con timestamp per ripetizione
struct ButtonMapping {   
    Button* btn;
    const char* label;
    unsigned long lastRepeatTime;  // Timestamp ultimo invio
};

ButtonMapping buttons[] = {
    {&base_sx,  "Base SX", 0},
    {&base_dx,  "Base DX", 0},
    {&elbow_sx, "Elbow SX", 0},
    {&elbow_dx, "Elbow DX", 0},
    {&wrist_sx, "Wrist SX", 0},
    {&wrist_dx, "Wrist DX", 0},
    {&claw_sx,  "Claw Open", 0},
    {&claw_dx,  "Claw Close", 0}
};

const int NUM_BUTTONS = sizeof(buttons) / sizeof(buttons[0]);

// Stato connessione
bool isConnected = false;
unsigned long lastSendTime = 0;
unsigned long lastHeartbeatTime = 0;
unsigned long lastSuccessTime = 0;

// Statistiche
int messagesSent = 0;
int messagesFailed = 0;

// ============================================================================
// CALLBACK ESP-NOW
// ============================================================================

void onDataSent(const uint8_t* mac_addr, esp_now_send_status_t status) {
    if (status == ESP_NOW_SEND_SUCCESS) {
        isConnected = true;
        lastSuccessTime = millis();
        messagesSent++;
        
        // Log ridotto per evitare spam
        if (messagesSent % 20 == 0) {
            Serial.printf("✅ %d messaggi inviati\n", messagesSent);
        }
    }
    else {
        messagesFailed++;
        
        if (millis() - lastSuccessTime > 3000) {
            isConnected = false;
        }
        
        // Log errori solo occasionalmente
        if (messagesFailed % 5 == 0) {
            Serial.printf("❌ Errori: %d\n", messagesFailed);
        }
    }
}

// ============================================================================
// FUNZIONI UTILITY
// ============================================================================

bool initESPNow() {
    Serial.println("\nInizializzazione ESP-NOW...");
    
    WiFi.mode(WIFI_STA);
    Serial.print("   Controller MAC: ");
    Serial.println(WiFi.macAddress());
    
    if (esp_now_init() != ESP_OK) {
        Serial.println("Errore inizializzazione ESP-NOW");
        return false;
    }
    
    Serial.println("ESP-NOW inizializzato");
    esp_now_register_send_cb(onDataSent);
    
    esp_now_peer_info_t peerInfo = {};
    memcpy(peerInfo.peer_addr, receiverAddress, 6);
    peerInfo.channel = 0;
    peerInfo.encrypt = false;
    
    if (esp_now_add_peer(&peerInfo) != ESP_OK) {
        Serial.println("Errore aggiunta peer");
        return false;
    }
    
    Serial.print("Peer aggiunto: ");
    for (int i = 0; i < 6; i++) {
        Serial.printf("%02X", receiverAddress[i]);
        if (i < 5) Serial.print(":");
    }
    Serial.println("\n");
    
    return true;
}

bool sendMessage(const char* message) {
    // Throttling globale
    if (millis() - lastSendTime < SEND_INTERVAL) {
        return false;
    }
    
    esp_err_t result = esp_now_send(
        receiverAddress,
        (uint8_t*)message,
        strlen(message) + 1
    );
    
    lastSendTime = millis();
    
    if (result == ESP_OK) {
        Serial.printf("📤 %s\n", message);
        return true;
    }
    else {
        Serial.printf("⚠️ Errore: %d\n", result);
        return false;
    }
}

void sendHeartbeat() {
    if (millis() - lastHeartbeatTime > HEARTBEAT_INTERVAL) {
        sendMessage("HEARTBEAT");
        lastHeartbeatTime = millis();
    }
}

void printStats() {
    static unsigned long lastStatsTime = 0;
    
    if (millis() - lastStatsTime > 15000) {  // Ogni 15 secondi
        Serial.println("\n╔════════════════════════════════════╗");
        Serial.println("║  STATISTICHE CONTROLLER            ║");
        Serial.println("╚════════════════════════════════════╝");
        Serial.printf("Stato:      %s\n", isConnected ? "✅ Connesso" : "❌ Disconnesso");
        Serial.printf("Inviati:    %d\n", messagesSent);
        Serial.printf("Falliti:    %d\n", messagesFailed);
        Serial.printf("Successo:   %.1f%%\n", 
            messagesSent > 0 ? (messagesSent * 100.0 / (messagesSent + messagesFailed)) : 0.0
        );
        Serial.println();
        
        lastStatsTime = millis();
    }
}

// ============================================================================
// SETUP
// ============================================================================

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("\n╔════════════════════════════════════╗");
    Serial.println("║  CONTROLLER ESP32 - ESP-NOW        ║");
    Serial.println("║  (Versione Pressione Continua)     ║");
    Serial.println("╚════════════════════════════════════╝\n");
    
    if (!initESPNow()) {
        Serial.println("❌ Inizializzazione fallita!");
        Serial.println("   Riavvio in 5 secondi...");
        delay(5000);
        ESP.restart();
    }
    
    Serial.println("╔════════════════════════════════════╗");
    Serial.println("║  Controller pronto!                ║");
    Serial.println("║  Tieni premuto per azione continua ║");
    Serial.println("╚════════════════════════════════════╝\n");
    
    Serial.println("Mappatura pulsanti:");
    Serial.println("  • Base SX/DX   → Giallo (GPIO14/27)");
    Serial.println("  • Elbow SX/DX  → Bianco (GPIO26/25)");
    Serial.println("  • Wrist SX/DX  → Blu (GPIO33/32)");
    Serial.println("  • Claw Open/Close → Giallo (GPIO12/13)\n");
    Serial.println("💡 Tieni premuto per movimento continuo!");
    Serial.println("   Intervallo ripetizione: 200ms\n");
}

// ============================================================================
// LOOP - GESTIONE PRESSIONE CONTINUA
// ============================================================================

void loop() {
    // ========================================
    // Aggiorna stato pulsanti
    // ========================================
    
    for (int i = 0; i < NUM_BUTTONS; i++) {
        buttons[i].btn->update();
    }
    
    // ========================================
    // Raccogli comandi da inviare
    // ========================================
    
    String commandsToSend = "";
    bool anyButtonPressed = false;
    
    for (int i = 0; i < NUM_BUTTONS; i++) {
        // Verifica se pulsante è attualmente premuto
        if (buttons[i].btn->isPressed()) {
            anyButtonPressed = true;
            
            // Verifica se è il momento di ripetere il comando
            unsigned long now = millis();
            
            // Prima pressione O tempo di ripetizione trascorso
            if (buttons[i].lastRepeatTime == 0 || 
                (now - buttons[i].lastRepeatTime) >= REPEAT_INTERVAL) {
                
                // Aggiungi comando alla lista
                commandsToSend += String(buttons[i].label) + " ";
                
                // Aggiorna timestamp
                buttons[i].lastRepeatTime = now;
                
                // Log prima pressione vs ripetizione
                if (buttons[i].btn->wasPressed()) {
                    Serial.printf("🔵 Premuto: %s (inizio)\n", buttons[i].label);
                } else {
                    Serial.printf("🔄 Ripeto: %s\n", buttons[i].label);
                }
            }
        }
        else {
            // Pulsante rilasciato, resetta timestamp
            if (buttons[i].lastRepeatTime > 0) {
                Serial.printf("⚪ Rilasciato: %s\n", buttons[i].label);
                buttons[i].lastRepeatTime = 0;
            }
        }
    }
    
    // ========================================
    // Invia comandi se presenti
    // ========================================
    
    if (commandsToSend.length() > 0) {
        commandsToSend.trim();
        
        char message[128];
        snprintf(message, sizeof(message), "%s", commandsToSend.c_str());
        
        sendMessage(message);
    }
    
    // ========================================
    // Heartbeat periodico (solo se nessun pulsante premuto)
    // ========================================
    
    if (!anyButtonPressed) {
        sendHeartbeat();
    }
    
    // ========================================
    // Statistiche periodiche
    // ========================================
    
    printStats();
    
    // ========================================
    // Delay minimo
    // ========================================
    
    delay(10);
}

// ============================================================================
// FUNZIONI DEBUG (OPZIONALI)
// ============================================================================

void handleSerialCommands() {
    if (Serial.available()) {
        String cmd = Serial.readStringUntil('\n');
        cmd.trim();
        
        if (cmd == "stats") {
            Serial.println("\n=== STATISTICHE DETTAGLIATE ===");
            Serial.printf("Stato:           %s\n", isConnected ? "Connesso" : "Disconnesso");
            Serial.printf("Messaggi OK:     %d\n", messagesSent);
            Serial.printf("Messaggi KO:     %d\n", messagesFailed);
            Serial.printf("Ultimo successo: %lu ms fa\n", millis() - lastSuccessTime);
            Serial.printf("Uptime:          %lu secondi\n", millis() / 1000);
            Serial.println();
        }
        else if (cmd == "restart") {
            Serial.println("Riavvio...");
            delay(1000);
            ESP.restart();
        }
        else if (cmd == "test") {
            Serial.println("Invio messaggio di test...");
            sendMessage("CENTER");
        }
        else if (cmd.startsWith("repeat:")) {
            // Cambia intervallo ripetizione dinamicamente
            // Esempio: "repeat:300" → imposta 300ms
            int newInterval = cmd.substring(7).toInt();
            if (newInterval >= 50 && newInterval <= 1000) {
                // NOTA: Dovresti rendere REPEAT_INTERVAL modificabile
                Serial.printf("⚙️ Intervallo ripetizione: %dms\n", newInterval);
            }
        }
        else if (cmd == "help") {
            Serial.println("\n=== COMANDI DISPONIBILI ===");
            Serial.println("stats    → Statistiche dettagliate");
            Serial.println("restart  → Riavvia controller");
            Serial.println("test     → Invia comando CENTER");
            Serial.println("help     → Mostra questo messaggio");
            Serial.println();
        }
    }
}