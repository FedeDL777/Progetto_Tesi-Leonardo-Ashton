
#ifndef __COMMUNICATION_TASK_H__
#define __COMMUNICATION_TASK_H__

#include "Task.h"
#include "RoboticArmMachine.h"
#include <esp_now.h>
#include <WiFi.h>

class CommunicationTask : public Task {
public:
    CommunicationTask(RoboticArmMachine* machine, unsigned long timeout = 5000);
    
    /**
     * Inizializza ESP-NOW
     * Da chiamare PRIMA di aggiungere al scheduler
     */
    bool begin();
    
    /**
     * Task tick - override da Task base
     */
    void tick() override;
    
    /**
     * Callback statica per ESP-NOW
     */
    static void onDataReceived(const uint8_t* mac, const uint8_t* data, int len);
    
    /**
     * Handler messaggi
     */
    void handleMessage(const uint8_t* mac, const uint8_t* data, int len);
    
    /**
     * Statistiche
     */
    int getMessagesReceived() const { return messagesReceived; }
    int getMessagesFailed() const { return messagesFailed; }
    bool isConnected() const { return connected; }

private:
    RoboticArmMachine* machine;
    unsigned long connectionTimeout;
    unsigned long lastMessageTime;
    bool connected;
    
    int messagesReceived;
    int messagesFailed;
    
    static CommunicationTask* instance;
};

#endif