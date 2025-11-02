/*******************************************************************************
 * COMMUNICATION TASK - IMPLEMENTAZIONE
 ******************************************************************************/

#include "../include/Comunication_Task_ESPNOW.h"

// Instance statica
CommunicationTask* CommunicationTask::instance = nullptr;

// ============================================================================
// COSTRUTTORE
// ============================================================================

CommunicationTask::CommunicationTask(
    RoboticArmMachine* machine, 
    unsigned long timeout
) : machine(machine),
    connectionTimeout(timeout),
    lastMessageTime(0),
    connected(false),
    messagesReceived(0),
    messagesFailed(0)
{
    instance = this;
}

// ============================================================================
// INIZIALIZZAZIONE ESP-NOW
// ============================================================================

bool CommunicationTask::begin() {
    Serial.println("\n Inizializzazione ESP-NOW...");
    
    WiFi.mode(WIFI_STA);
    
    if (esp_now_init() != ESP_OK) {
        Serial.println("Errore inizializzazione ESP-NOW");
        return false;
    }
    
    Serial.println("ESP-NOW inizializzato");
    
    esp_now_register_recv_cb(CommunicationTask::onDataReceived);

    Serial.println("Callback registrata\n");

    lastMessageTime = millis();
    return true;
}

// ============================================================================
// CALLBACK ESP-NOW
// ============================================================================

void CommunicationTask::onDataReceived(
    const uint8_t* mac, 
    const uint8_t* data, 
    int len
) {
    if (instance != nullptr) {
        instance->handleMessage(mac, data, len);
    }
}

void CommunicationTask::handleMessage(
    const uint8_t* mac, 
    const uint8_t* data, 
    int len
) {
    lastMessageTime = millis();
    messagesReceived++;
    
    // Riconnessione se disconnessi
    if (!connected) {
        connected = true;
        Serial.println("Connessione ristabilita!");
        
        if (machine->getCurrentState() == STATE_NETWORK_LOST) {
            machine->connectionEstablished();
        }
    }
    
    // Converti dati in String
    String message = "";
    for (int i = 0; i < len && i < 128; i++) {
        message += (char)data[i];
    }
    message.trim();
    
    // Log ridotto
    if (messagesReceived % 10 == 0) {
        Serial.printf(" RX [%d]: \"%s\"\n", messagesReceived, message.c_str());
    }
    
    // Ignora heartbeat
    if (message == "HEARTBEAT" || message == "PING") {
        return;
    }
    
    // Push comando alla coda
    if (message.length() > 0) {
        bool pushed = machine->pushCommand(message);
        
        if (!pushed) {
            messagesFailed++;
            Serial.println("⚠️ Coda piena!");
        }
        
        // Avvia working se necessario
        if (machine->getCurrentState() == STATE_CONNECTED || 
            machine->getCurrentState() == STATE_IDLE) {
            machine->startWorking();
        }
    }
}

// ============================================================================
// TASK TICK
// ============================================================================

void CommunicationTask::tick() {
    unsigned long now = millis();
    
    // Controlla timeout
    if (connected) {
        if (now - lastMessageTime > connectionTimeout) {
            connected = false;
            
            Serial.println("\nTIMEOUT CONNESSIONE!");
            Serial.printf("   Nessun messaggio da %lu ms\n", 
                now - lastMessageTime);
            
            machine->connectionLost();
            lastMessageTime = now;
        }
    }
    
    // Log periodico (ogni 30 secondi)
    static unsigned long lastStatsLog = 0;
    if (now - lastStatsLog > 30000) {
        Serial.println("COMMUNICATION STATS");
        Serial.printf("Status:    %s\n", connected ? "Connected" : "Disconnected");
        Serial.printf("Received:  %d messages\n", messagesReceived);
        Serial.printf("Failed:    %d messages\n", messagesFailed);
        Serial.printf("Last msg:  %lu ms ago\n", now - lastMessageTime);
        Serial.println();
        
        lastStatsLog = now;
    }
}
