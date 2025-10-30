/*******************************************************************************
 * ROBOT ARM - ESP-NOW RECEIVER CON RoboticArmMachine
 * 
 * Integra ESP-NOW con l'architettura esistente del braccio robotico
 * Usa RoboticArmMachine per gestire FSM e movimenti servo
 * 
 * Comandi supportati:
 * - "Base SX" → Ruota base a sinistra (-10°)
 * - "Base DX" → Ruota base a destra (+10°)
 * - "Elbow SX" → Ruota gomito a sinistra (-10°)
 * - "Elbow DX" → Ruota gomito a destra (+10°)
 * - "Wrist SX" → Ruota polso a sinistra (-10°)
 * - "Wrist DX" → Ruota polso a destra (+10°)
 ******************************************************************************/

#include <esp_now.h>
#include <WiFi.h>
#include "RoboticArmMachine.h"
#include "include/set_up.h"

// ============================================================================
// VARIABILI GLOBALI
// ============================================================================

RoboticArmMachine *machine;

// Parametri di movimento
const int ANGLE_STEP = 10;  // Gradi per ogni comando

// Timeout per rilevare disconnessione controller
unsigned long lastMessageTime = 0;
const unsigned long CONNECTION_TIMEOUT = 5000;  // 5 secondi

// ============================================================================
// FUNZIONI MOVIMENTO
// ============================================================================

/**
 * Muove base a sinistra (decrementa angolo)
 */
void moveBaseLeft() {
    int currentAngle = machine->getBaseAngle();
    int newAngle = currentAngle - ANGLE_STEP;
    
    Serial.printf("⬅️  Base SX: %d° → %d°\n", currentAngle, newAngle);
    machine->moveBaseServo(newAngle);
}

/**
 * Muove base a destra (incrementa angolo)
 */
void moveBaseRight() {
    int currentAngle = machine->getBaseAngle();
    int newAngle = currentAngle + ANGLE_STEP;
    
    Serial.printf("➡️  Base DX: %d° → %d°\n", currentAngle, newAngle);
    machine->moveBaseServo(newAngle);
}

/**
 * Muove gomito a sinistra (decrementa angolo)
 */
void moveElbowLeft() {
    int currentAngle = machine->getElbowAngle();
    int newAngle = currentAngle - ANGLE_STEP;
    
    Serial.printf("⬅️  Elbow SX: %d° → %d°\n", currentAngle, newAngle);
    machine->moveElbowServo(newAngle);
}

/**
 * Muove gomito a destra (incrementa angolo)
 */
void moveElbowRight() {
    int currentAngle = machine->getElbowAngle();
    int newAngle = currentAngle + ANGLE_STEP;
    
    Serial.printf("➡️  Elbow DX: %d° → %d°\n", currentAngle, newAngle);
    machine->moveElbowServo(newAngle);
}

/**
 * Muove polso a sinistra (decrementa angolo)
 */
void moveWristLeft() {
    int currentAngle = machine->getWristAngle();
    int newAngle = currentAngle - ANGLE_STEP;
    
    Serial.printf("⬅️  Wrist SX: %d° → %d°\n", currentAngle, newAngle);
    machine->moveWristServo(newAngle);
}

/**
 * Muove polso a destra (incrementa angolo)
 */
void moveWristRight() {
    int currentAngle = machine->getWristAngle();
    int newAngle = currentAngle + ANGLE_STEP;
    
    Serial.printf("➡️  Wrist DX: %d° → %d°\n", currentAngle, newAngle);
    machine->moveWristServo(newAngle);
}

void moveClawOpen() {
    int currentAngle = machine->getClawAngle();
    int newAngle = currentAngle + ANGLE_STEP;
    
    Serial.printf("➡️  Claw Open: %d° → %d°\n", currentAngle, newAngle);
    machine->moveClawServo(newAngle);
}

void moveClawClose() {
    int currentAngle = machine->getClawAngle();
    int newAngle = currentAngle - ANGLE_STEP;
    
    Serial.printf("⬅️  Claw Close: %d° → %d°\n", currentAngle, newAngle);
    machine->moveClawServo(newAngle);
}

// ============================================================================
// ESP-NOW CALLBACK
// ============================================================================

/**
 * Callback chiamata quando arriva un messaggio dal controller
 */
void onReceive(const uint8_t *mac, const uint8_t *incomingData, int len) {
    // Aggiorna timestamp ultima ricezione
    lastMessageTime = millis();
    
    // Se eravamo disconnessi, ristabilisci connessione
    if (machine->getCurrentState() == STATE_NETWORK_LOST) {
        Serial.println("🔄 Riconnessione stabilita!");
        machine->connectionEstablished();
    }
    
    // Stampa MAC address mittente
    Serial.print("📨 Ricevuto da ");
    for (int i = 0; i < 6; i++) {
        Serial.printf("%02X", mac[i]);
        if (i < 5) Serial.print(":");
    }
    Serial.print(" → ");
    
    // Converti messaggio in String
    String message = "";
    for (int i = 0; i < len; i++) {
        message += (char)incomingData[i];
    }
    message.trim();  // Rimuovi spazi extra
    
    Serial.printf("\"%s\"\n", message.c_str());
    
    // ========================================
    // PARSING COMANDI E TRANSIZIONE STATO
    // ========================================
    
    // Se non stiamo lavorando, inizia
    if (machine->getCurrentState() == STATE_CONNECTED || 
        machine->getCurrentState() == STATE_IDLE) {
        machine->startWorking();
    }
    
    // Processa comando
    if (message.indexOf("Base SX") >= 0) {
        moveBaseLeft();
    }
    else if (message.indexOf("Base DX") >= 0) {
        moveBaseRight();
    }
    else if (message.indexOf("Elbow SX") >= 0) {
        moveElbowLeft();
    }
    else if (message.indexOf("Elbow DX") >= 0) {
        moveElbowRight();
    }
    else if (message.indexOf("Wrist SX") >= 0) {
        moveWristLeft();
    }
    else if (message.indexOf("Wrist DX") >= 0) {
        moveWristRight();
    }
    else if (message.indexOf("Claw Open") >= 0) {
        moveClawOpen();
    }
    else if (message.indexOf("Claw Close") >= 0) {
        moveClawClose();
    }


    else if (message.indexOf("Safe") >= 0) {
        Serial.println("🔒 Posizione sicura richiesta");
        machine->moveAllToSafePosition();
    }
    else if (message.indexOf("Center") >= 0) {
        Serial.println("📍 Posizione centrale richiesta");
        machine->moveAllToCenter();
    }
    else {
        Serial.println("⚠️  Comando non riconosciuto");
    }
    
    Serial.println();
}

// ============================================================================
// SETUP
// ============================================================================

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("\n╔════════════════════════════════════════╗");
    Serial.println("║  ROBOT ARM - ESP-NOW + FSM            ║");
    Serial.println("╚════════════════════════════════════════╝\n");
    
    // ========================================
    // Inizializza RoboticArmMachine
    // ========================================
    
    Serial.println("🤖 Inizializzazione RoboticArmMachine...\n");
    
    machine = new RoboticArmMachine();
    machine->begin();
    
    Serial.println("✅ RoboticArmMachine pronta!\n");
    
    // ========================================
    // Setup ESP-NOW
    // ========================================
    
    Serial.println("📡 Inizializzazione ESP-NOW...");
    
    // Imposta WiFi in modalità Station
    WiFi.mode(WIFI_STA);
    
    // Ottieni e stampa MAC address
    Serial.print("   MAC Address: ");
    Serial.println(WiFi.macAddress());
    Serial.println("   (Usa questo indirizzo nel controller!)\n");
    
    // Inizializza ESP-NOW
    if (esp_now_init() != ESP_OK) {
        Serial.println("❌ Errore inizializzazione ESP-NOW");
        machine->servoError("ESP-NOW init failed");
        return;
    }
    
    Serial.println("✅ ESP-NOW inizializzato\n");
    
    // Registra callback ricezione
    esp_now_register_recv_cb(onReceive);
    
    // ========================================
    // Stampa stato iniziale
    // ========================================
    
    Serial.println(machine->getDebugInfo());
    
    Serial.println("╔════════════════════════════════════════╗");
    Serial.println("║  Sistema pronto!                       ║");
    Serial.println("║  In ascolto comandi wireless...        ║");
    Serial.println("╚════════════════════════════════════════╝\n");
    
    Serial.println("Comandi supportati:");
    Serial.println("  • Base SX/DX   → Ruota base");
    Serial.println("  • Elbow SX/DX  → Ruota gomito");
    Serial.println("  • Wrist SX/DX  → Ruota polso");
    Serial.println("  • Safe         → Posizione sicura");
    Serial.println("  • Center       → Posizione centrale\n");
    
    // Inizializza timestamp
    lastMessageTime = millis();
}

// ============================================================================
// LOOP
// ============================================================================

void loop() {
    // ========================================
    // Aggiorna FSM
    // ========================================
    machine->update();
    
    // ========================================
    // Controlla timeout connessione
    // ========================================
    if (millis() - lastMessageTime > CONNECTION_TIMEOUT) {
        // Se siamo connessi e non riceviamo messaggi da troppo tempo
        if (machine->getCurrentState() != STATE_NETWORK_LOST &&
            machine->getCurrentState() != STATE_START) {
            
            Serial.println("⚠️  Timeout connessione! Nessun messaggio da 5s");
            machine->connectionLost();
            lastMessageTime = millis();  // Reset per evitare spam
        }
    }
    
    // ========================================
    // Gestione pulsanti locali (opzionale)
    // ========================================
    
    // Pulsante bianco → Safe position
    if (machine->isButtonWhitePressed()) {
        Serial.println("🔘 Pulsante bianco premuto → Safe position");
        machine->moveAllToSafePosition();
        delay(200);  // Debounce
    }
    
    // Pulsante blu → Stop/Emergency
    if (machine->isButtonBluePressed()) {
        Serial.println("🔘 Pulsante blu premuto → Stop");
        if (machine->getCurrentState() == STATE_WORKING) {
            machine->stopWorking();
        }
        delay(200);  // Debounce
    }
    
    // ========================================
    // Comandi seriali (debug)
    // ========================================
    
    if (Serial.available()) {
        String command = Serial.readStringUntil('\n');
        command.trim();
        
        if (command == "info") {
            Serial.println(machine->getDebugInfo());
        }
        else if (command == "safe") {
            machine->moveAllToSafePosition();
        }
        else if (command == "center") {
            machine->moveAllToCenter();
        }
        else if (command.startsWith("base:")) {
            int angle = command.substring(5).toInt();
            machine->moveBaseServo(angle);

        }
        else if (command.startsWith("elbow:")) {
            int angle = command.substring(6).toInt();
            machine->moveElbowServo(angle);
        }
        else if (command.startsWith("wrist:")) {
            int angle = command.substring(6).toInt();
            machine->moveWristServo(angle);
        }
        else if (command.startsWith("claw:")) {
            int angle = command.substring(5).toInt();
            machine->moveClawServo(angle);
        }
        else if (command == "help") {
            Serial.println("\n=== COMANDI SERIALI ===");
            Serial.println("info         → Mostra stato sistema");
            Serial.println("safe         → Posizione sicura");
            Serial.println("center       → Posizione centrale");
            Serial.println("base:XXX     → Muovi base a XXX gradi");
            Serial.println("elbow:XXX    → Muovi gomito a XXX gradi");
            Serial.println("wrist:XXX    → Muovi polso a XXX gradi");
            Serial.println("claw:XXX     → Muovi pinza a XXX gradi");
            Serial.println();
        }
        else {
            Serial.println("❌ Comando non riconosciuto (scrivi 'help')");
        }
    }
    
    delay(10);
}
