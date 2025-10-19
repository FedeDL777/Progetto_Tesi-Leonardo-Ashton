

#include "kernel/task/include/Comunication_Task_ESPNOW.h"

// Inizializzazione variabili statiche
ReceivedMessage CommunicationTask_ESPNOW::messageBuffer = {false, {0}, {0}, 0, 0};
CommunicationTask_ESPNOW* CommunicationTask_ESPNOW::instance = nullptr;

// ============================================================================
// COSTRUTTORE
// ============================================================================

CommunicationTask_ESPNOW::CommunicationTask_ESPNOW(RoboticArmMachine* machine) {
    this->machine = machine;
    this->lastMessageTime = 0;
    this->messagesReceived = 0;
    this->messagesDropped = 0;
    this->timeoutCount = 0;
    this->wasConnected = false;
    
    // Salva istanza per callback
    instance = this;
    
    Serial.println("CommunicationTask created");
}

// ============================================================================
// INIZIALIZZAZIONE ESP-NOW
// ============================================================================

bool CommunicationTask_ESPNOW::initESPNow() {
    Serial.println("\nInizializzazione ESP-NOW...");
    
    // Imposta WiFi in modalità Station
    WiFi.mode(WIFI_STA);
    
    // Ottieni e stampa MAC address
    Serial.print("   MAC Address: ");
    Serial.println(WiFi.macAddress());
    Serial.println("Usa questo indirizzo nel controller!\n");
    
    // Inizializza ESP-NOW
    if (esp_now_init() != ESP_OK) {
        Serial.println("Errore inizializzazione ESP-NOW");
        return false;
    }
    
    Serial.println("ESP-NOW inizializzato");
    
    // Registra callback ricezione
    esp_now_register_recv_cb(CommunicationTask_ESPNOW::onDataReceived);
    
    Serial.println("Callback registrata\n");
    
    // Inizializza timestamp
    lastMessageTime = millis();
    
    return true;
}

// ============================================================================
// CALLBACK ESP-NOW (STATICA)
// ============================================================================

void CommunicationTask_ESPNOW::onDataReceived(
    const uint8_t* macAddr,
    const uint8_t* data,
    int dataLen
) {
    // Callback chiamata da ISR - deve essere velocissima!
    // Non fare Serial.print() qui dentro, copia solo i dati
    
    if (instance == nullptr) return;
    
    // Se c'è già un messaggio non processato, scarta questo
    if (messageBuffer.available) {
        instance->messagesDropped++;
        return;
    }
    
    // Copia dati nel buffer
    messageBuffer.available = true;
    memcpy(messageBuffer.macAddress, macAddr, 6);
    memcpy(messageBuffer.data, data, min(dataLen, 250));
    messageBuffer.dataLength = dataLen;
    messageBuffer.timestamp = millis();
}

// ============================================================================
// TICK PERIODICO
// ============================================================================

void CommunicationTask_ESPNOW::tick() {
    // Processa eventuali messaggi ricevuti
    processIncomingMessages();
    
    // Controlla timeout connessione
    checkConnectionTimeout();
}

// ============================================================================
// PROCESS INCOMING MESSAGES
// ============================================================================

void CommunicationTask_ESPNOW::processIncomingMessages() {
    // Controlla se c'è un messaggio disponibile
    if (!messageBuffer.available) {
        return;  // Nessun messaggio
    }
    
    // Aggiorna timestamp ultima ricezione
    lastMessageTime = messageBuffer.timestamp;
    
    // Se eravamo disconnessi, ristabilisci connessione
    if (machine->getCurrentState() == STATE_NETWORK_LOST) {
        Serial.println("🔄 Riconnessione stabilita!");
        machine->connectionEstablished();
        wasConnected = true;
    }
    
    // Se prima era START, transizione a CONNECTED
    if (machine->getCurrentState() == STATE_START) {
        Serial.println("🔗 Prima connessione stabilita!");
        machine->connectionEstablished();
        wasConnected = true;
    }
    
    // Log messaggio ricevuto
    logReceivedMessage(
        messageBuffer.macAddress,
        messageBuffer.data,
        messageBuffer.dataLength
    );
    
    // Parsing comando
    parseCommand(messageBuffer.data, messageBuffer.dataLength);
    
    // Incrementa contatore
    messagesReceived++;
    
    // Libera buffer per prossimo messaggio
    messageBuffer.available = false;
}

// ============================================================================
// CHECK CONNECTION TIMEOUT
// ============================================================================

void CommunicationTask_ESPNOW::checkConnectionTimeout() {
    unsigned long timeSinceLastMessage = millis() - lastMessageTime;
    
    // Se timeout superato e non siamo già in stato NETWORK_LOST
    if (timeSinceLastMessage > TIMEOUT_MS) {
        if (machine->getCurrentState() != STATE_NETWORK_LOST &&
            machine->getCurrentState() != STATE_START) {
            
            Serial.printf(
                "⚠️  Timeout connessione! Nessun messaggio da %lu ms\n",
                timeSinceLastMessage
            );
            
            machine->connectionLost();
            wasConnected = false;
            timeoutCount++;
            
            // Reset timer per evitare spam
            lastMessageTime = millis();
        }
    }
}

// ============================================================================
// PARSE COMMAND
// ============================================================================

void CommunicationTask_ESPNOW::parseCommand(const char* command, int length) {
    // Converti in String per manipolazione
    String cmd = String(command);
    cmd.trim();
    
    Serial.printf("📨 Processing command: \"%s\"\n", cmd.c_str());
    
    // Se non stiamo lavorando, inizia
    if (machine->getCurrentState() == STATE_CONNECTED || 
        machine->getCurrentState() == STATE_IDLE) {
        machine->startWorking();
    }
    
    // ========================================
    // PARSING COMANDI BASE/ELBOW/WRIST
    // ========================================
    
    if (cmd.indexOf("Base SX") >= 0) {
        int currentAngle = machine->getBaseAngle();
        int newAngle = currentAngle - 10;
        Serial.printf("⬅️  Base SX: %d° → %d°\n", currentAngle, newAngle);
        machine->moveBaseServo(newAngle);
    }
    else if (cmd.indexOf("Base DX") >= 0) {
        int currentAngle = machine->getBaseAngle();
        int newAngle = currentAngle + 10;
        Serial.printf("➡️  Base DX: %d° → %d°\n", currentAngle, newAngle);
        machine->moveBaseServo(newAngle);
    }
    else if (cmd.indexOf("Elbow SX") >= 0) {
        int currentAngle = machine->getElbowAngle();
        int newAngle = currentAngle - 10;
        Serial.printf("⬅️  Elbow SX: %d° → %d°\n", currentAngle, newAngle);
        machine->moveElbowServo(newAngle);
    }
    else if (cmd.indexOf("Elbow DX") >= 0) {
        int currentAngle = machine->getElbowAngle();
        int newAngle = currentAngle + 10;
        Serial.printf("➡️  Elbow DX: %d° → %d°\n", currentAngle, newAngle);
        machine->moveElbowServo(newAngle);
    }
    else if (cmd.indexOf("Wrist SX") >= 0) {
        int currentAngle = machine->getWristAngle();
        int newAngle = currentAngle - 10;
        Serial.printf("⬅️  Wrist SX: %d° → %d°\n", currentAngle, newAngle);
        machine->moveWristServo(newAngle);
    }
    else if (cmd.indexOf("Wrist DX") >= 0) {
        int currentAngle = machine->getWristAngle();
        int newAngle = currentAngle + 10;
        Serial.printf("➡️  Wrist DX: %d° → %d°\n", currentAngle, newAngle);
        machine->moveWristServo(newAngle);
    }
    
    // ========================================
    // COMANDI SPECIALI
    // ========================================
    
    else if (cmd.indexOf("Safe") >= 0 || cmd.indexOf("SAFE") >= 0) {
        Serial.println("🔒 Posizione sicura richiesta");
        machine->moveAllToSafePosition();
    }
    else if (cmd.indexOf("Center") >= 0 || cmd.indexOf("CENTER") >= 0) {
        Serial.println("📍 Posizione centrale richiesta");
        machine->moveAllToCenter();
    }
    
    // ========================================
    // COMANDO NON RICONOSCIUTO
    // ========================================
    
    else {
        Serial.println("⚠️  Comando non riconosciuto");
    }
    
    Serial.println();
}

// ============================================================================
// LOG RECEIVED MESSAGE
// ============================================================================

void CommunicationTask_ESPNOW::logReceivedMessage(
    const uint8_t* mac,
    const char* data,
    int len
) {
    Serial.print("📨 Ricevuto da ");
    
    // Stampa MAC address mittente
    for (int i = 0; i < 6; i++) {
        Serial.printf("%02X", mac[i]);
        if (i < 5) Serial.print(":");
    }
    
    Serial.print(" → \"");
    Serial.print(data);
    Serial.println("\"");
}

// ============================================================================
// GETTERS
// ============================================================================

bool CommunicationTask_ESPNOW::isConnected() const {
    unsigned long timeSinceLastMessage = millis() - lastMessageTime;
    return (timeSinceLastMessage < TIMEOUT_MS) && wasConnected;
}

String CommunicationTask_ESPNOW::getControllerMAC() const {
    if (!messageBuffer.available && messagesReceived == 0) {
        return "Not connected";
    }
    
    String mac = "";
    for (int i = 0; i < 6; i++) {
        if (messageBuffer.macAddress[i] < 16) mac += "0";
        mac += String(messageBuffer.macAddress[i], HEX);
        if (i < 5) mac += ":";
    }
    mac.toUpperCase();
    return mac;
}
