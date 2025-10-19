#ifndef __COMMUNICATION_TASK_ESPNOW__
#define __COMMUNICATION_TASK_ESPNOW__

#include "kernel/Task.h"
#include "RoboticArmMachine.h"
#include <esp_now.h>
#include <WiFi.h>

// ============================================================================
// STRUTTURE DATI
// ============================================================================

/**
 * Struttura messaggio ricevuto via ESP-NOW
 * Memorizza temporaneamente i dati in arrivo
 */
struct ReceivedMessage {
    bool available;              // True se c'è un messaggio da processare
    uint8_t macAddress[6];       // MAC address mittente
    char data[250];              // Dati messaggio (max ESP-NOW = 250 byte)
    int dataLength;              // Lunghezza effettiva dati
    unsigned long timestamp;     // Timestamp ricezione
};

// ============================================================================
// COMMUNICATION TASK CLASS
// ============================================================================

class CommunicationTask_ESPNOW : public Task {
public:
    /**
     * Costruttore
     * @param machine Puntatore a RoboticArmMachine
     */
    CommunicationTask_ESPNOW(RoboticArmMachine* machine);

    /**
     * Inizializzazione ESP-NOW
     * Deve essere chiamato nel setup() PRIMA di addTask()
     * @return true se inizializzazione ok, false altrimenti
     */
    bool initESPNow();
    
    /**
     * Tick periodico del task (chiamato dallo scheduler)
     */
    void tick() override;
    
    /**
     * Callback statica ESP-NOW per ricezione
     * Deve essere statica per essere registrata come callback C
     */
    static void onDataReceived(
        const uint8_t* macAddr,
        const uint8_t* data,
        int dataLen
    );
    
    // ========================================
    // GETTERS - STATISTICHE
    // ========================================
    
    int getMessagesReceived() const { return messagesReceived; }
    int getMessagesDropped() const { return messagesDropped; }
    int getTimeoutCount() const { return timeoutCount; }
    unsigned long getLastMessageTime() const { return lastMessageTime; }
    bool isConnected() const;
    
    /**
     * Ottiene stringa MAC address del controller connesso
     */
    String getControllerMAC() const;
    
private:
    // ========================================
    // VARIABILI PRIVATE
    // ========================================
    
    RoboticArmMachine* machine;
    
    // Timeout detection
    unsigned long lastMessageTime;
    const unsigned long TIMEOUT_MS = 5000;  // 5 secondi
    bool wasConnected;
    
    // Statistiche
    int messagesReceived;
    int messagesDropped;
    int timeoutCount;
    
    // Buffer messaggi (statico per callback)
    static ReceivedMessage messageBuffer;
    static CommunicationTask_ESPNOW* instance;  // Per accedere da callback statica
    
    // ========================================
    // METODI PRIVATI
    // ========================================
    
    /**
     * Processa messaggi ricevuti dal buffer
     */
    void processIncomingMessages();
    
    /**
     * Controlla timeout connessione
     */
    void checkConnectionTimeout();
    
    /**
     * Parsing e interpretazione comando
     */
    void parseCommand(const char* command, int length);
    
    /**
     * Stampa informazioni messaggio ricevuto
     */
    void logReceivedMessage(const uint8_t* mac, const char* data, int len);
};
#endif