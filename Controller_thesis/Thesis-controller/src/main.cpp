/*******************************************************************************
 * CONTROLLER ESP32 - ESP-NOW SENDER
 * 
 * Controller wireless per braccio robotico
 * Usa 6 pulsanti per controllare Base, Elbow, Wrist
 * Invia comandi via ESP-NOW al braccio robotico
 * 
 * Autore: Federico De Leonardis
 ******************************************************************************/

#include <esp_now.h>
#include <WiFi.h>
#include "include/Button.h"
#include "include/set_up.h"


// Parametri invio
const unsigned long SEND_INTERVAL = 100;  // ms tra invii (10Hz)
const unsigned long HEARTBEAT_INTERVAL = 2000;  // ms tra heartbeat (0.5Hz)

// ============================================================================
// VARIABILI GLOBALI
// ============================================================================

// Pulsanti con debouncing automatico
Button base_sx(YELLOW_BASE_SX, true);   // INPUT_PULLUP
Button base_dx(YELLOW_BASE_DX, true);
Button elbow_sx(WHITE_ELBOW_SX, true);
Button elbow_dx(WHITE_ELBOW_DX, true);
Button wrist_sx(BLUE_WRIST_SX, true);
Button wrist_dx(BLUE_WRIST_DX, true);

// Struttura mappatura pulsanti
struct ButtonMapping {   
    Button* btn;
    const char* label;
};

ButtonMapping buttons[] = {
    {&base_sx,  "Base SX"},
    {&base_dx,  "Base DX"},
    {&elbow_sx, "Elbow SX"},
    {&elbow_dx, "Elbow DX"},
    {&wrist_sx, "Wrist SX"},
    {&wrist_dx, "Wrist DX"}
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

/**
 * Callback invocata quando un messaggio viene inviato
 * Permette di monitorare stato connessione
 */
void onDataSent(const uint8_t* mac_addr, esp_now_send_status_t status) {
    // Aggiorna stato connessione
    if (status == ESP_NOW_SEND_SUCCESS) {
        isConnected = true;
        lastSuccessTime = millis();
        messagesSent++;
        
        // Log solo occasionalmente per non intasare seriale
        if (messagesSent % 10 == 0) {
            Serial.printf("✅ %d messaggi inviati con successo\n", messagesSent);
        }
    }
    else {
        messagesFailed++;
        
        // Dopo 3 fallimenti consecutivi, disconnetti
        if (millis() - lastSuccessTime > 3000) {
            isConnected = false;
        }
        
        Serial.print("Invio fallito a ");
        for (int i = 0; i < 6; i++) {
            Serial.printf("%02X", mac_addr[i]);
            if (i < 5) Serial.print(":");
        }
        Serial.println();
    }
}

// ============================================================================
// FUNZIONI UTILITY
// ============================================================================

/**
 * Inizializza ESP-NOW
 */
bool initESPNow() {
    Serial.println("\nInizializzazione ESP-NOW...");
    
    // Imposta WiFi in modalità Station
    WiFi.mode(WIFI_STA);
    
    // Stampa MAC address controller
    Serial.print("   Controller MAC: ");
    Serial.println(WiFi.macAddress());
    
    // Inizializza ESP-NOW
    if (esp_now_init() != ESP_OK) {
        Serial.println("Errore inizializzazione ESP-NOW");
        return false;
    }
    
    Serial.println("ESP-NOW inizializzato");
    
    // Registra callback invio
    esp_now_register_send_cb(onDataSent);
    
    // Aggiungi peer (braccio robotico)
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

/**
 * Invia messaggio al braccio robotico
 */
bool sendMessage(const char* message) {
    // Throttling: non inviare troppo velocemente
    if (millis() - lastSendTime < SEND_INTERVAL) {
        return false;
    }
    
    // Invia messaggio
    esp_err_t result = esp_now_send(
        receiverAddress,
        (uint8_t*)message,
        strlen(message) + 1
    );
    
    lastSendTime = millis();
    
    if (result == ESP_OK) {
        Serial.printf("📤 Inviato: \"%s\"\n", message);
        return true;
    }
    else {
        Serial.printf("⚠️ Errore invio: %d\n", result);
        return false;
    }
}

/**
 * Invia heartbeat periodico per mantenere connessione
 */
void sendHeartbeat() {
    if (millis() - lastHeartbeatTime > HEARTBEAT_INTERVAL) {
        sendMessage("HEARTBEAT");
        lastHeartbeatTime = millis();
    }
}

/**
 * Stampa statistiche periodicamente
 */
void printStats() {
    static unsigned long lastStatsTime = 0;
    
    if (millis() - lastStatsTime > 10000) {  // Ogni 10 secondi
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

/**
 * Gestisce LED stato (se presente)
 * Blink veloce = disconnesso
 * Blink lento = connesso
 */
void updateStatusLED() {
    // Se hai un LED di stato, aggiungi qui la logica
    // Esempio:
    // static bool ledState = false;
    // static unsigned long lastBlink = 0;
    // unsigned long blinkInterval = isConnected ? 1000 : 250;
    // 
    // if (millis() - lastBlink > blinkInterval) {
    //     ledState = !ledState;
    //     digitalWrite(LED_PIN, ledState);
    //     lastBlink = millis();
    // }
}

// ============================================================================
// SETUP
// ============================================================================

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("\n╔════════════════════════════════════╗");
    Serial.println("║  CONTROLLER ESP32 - ESP-NOW        ║");
    Serial.println("╚════════════════════════════════════╝\n");
    
    // Inizializza ESP-NOW
    if (!initESPNow()) {
        Serial.println("❌ Inizializzazione fallita!");
        Serial.println("   Riavvio in 5 secondi...");
        delay(5000);
        ESP.restart();
    }
    
    // Inizializza LED stato (opzionale)
    // pinMode(LED_PIN, OUTPUT);
    
    Serial.println("╔════════════════════════════════════╗");
    Serial.println("║  Controller pronto!                ║");
    Serial.println("║  Premi i pulsanti per controllare  ║");
    Serial.println("╚════════════════════════════════════╝\n");
    
    Serial.println("Mappatura pulsanti:");
    Serial.println("  • Base SX/DX   → Giallo (GPIO14/27)");
    Serial.println("  • Elbow SX/DX  → Bianco (GPIO26/25)");
    Serial.println("  • Wrist SX/DX  → Blu (GPIO33/32)\n");
}

// ============================================================================
// LOOP
// ============================================================================

void loop() {
    // ========================================
    // Aggiorna stato pulsanti
    // ========================================
    
    String commandsToSend = "";
    
    for (int i = 0; i < NUM_BUTTONS; i++) {
        buttons[i].btn->update();
        
        // Se pulsante premuto (evento)
        if (buttons[i].btn->wasPressed()) {
            Serial.printf("Pulsante premuto: %s\n", buttons[i].label);
            commandsToSend += String(buttons[i].label) + " ";
        }
    }
    
    // ========================================
    // Invia comandi se presenti
    // ========================================
    
    if (commandsToSend.length() > 0) {
        commandsToSend.trim();  // Rimuovi spazio finale
        
        char message[64];
        snprintf(message, sizeof(message), "%s", commandsToSend.c_str());
        
        sendMessage(message);
    }
    
    // ========================================
    // Heartbeat periodico
    // ========================================
    
    // Solo se non ci sono comandi da inviare
    if (commandsToSend.length() == 0) {
        sendHeartbeat();
    }
    
    // ========================================
    // Aggiorna LED stato
    // ========================================
    
    updateStatusLED();
    
    // ========================================
    // Statistiche periodiche
    // ========================================
    
    printStats();
    
    // ========================================
    // Delay minimo
    // ========================================
    
    delay(10);  // Piccolo delay per stabilità
}

// ============================================================================
// FUNZIONI AVANZATE (OPZIONALI)
// ============================================================================

/**
 * Riavvia controller in caso di problemi
 */
void checkHealthAndRestart() {
    static unsigned long lastHealthCheck = 0;
    
    if (millis() - lastHealthCheck > 30000) {  // Ogni 30 secondi
        // Se troppi fallimenti, riavvia
        if (messagesFailed > 100 && 
            (messagesSent * 100 / (messagesSent + messagesFailed)) < 50) {
            
            Serial.println("\n⚠️ Troppi errori! Riavvio...");
            delay(2000);
            ESP.restart();
        }
        
        lastHealthCheck = millis();
    }
}

/**
 * Gestione comandi seriali per debug
 */
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
        else if (cmd == "help") {
            Serial.println("\n=== COMANDI DISPONIBILI ===");
            Serial.println("stats    → Mostra statistiche dettagliate");
            Serial.println("restart  → Riavvia controller");
            Serial.println("test     → Invia comando di test (CENTER)");
            Serial.println("help     → Mostra questo messaggio");
            Serial.println();
        }
    }
}