#include "kernel/task/include/Comunication_Task.h"

CommunicationTask::CommunicationTask(
    RoboticArmMachine* machine,
    BluetoothSerial* bt
) {
    this->machine = machine;
    this->bt = bt;
    this->lastMessageTime = millis();
    this->messagesReceived = 0;
    this->messagesDropped = 0;
    this->messageBuffer = "";
}

void CommunicationTask::tick() {
    processIncomingData();
    checkTimeout();
}

void CommunicationTask::processIncomingData() {
    while (bt->available()) {
        char c = bt->read();
        
        if (c == '\n' || c == '\r') {
            if (messageBuffer.length() > 0) {
                // Messaggio completo ricevuto
                parseCommand(messageBuffer);
                messageBuffer = "";
                messagesReceived++;
                lastMessageTime = millis();
            }
        }
        else {
            messageBuffer += c;
            
            // Protezione overflow buffer
            if (messageBuffer.length() > MAX_BUFFER_SIZE) {
                Serial.println("Buffer overflow - messaggio scartato");
                messageBuffer = "";
                messagesDropped++;
            }
        }
    }
}

void CommunicationTask::checkTimeout() {
    if (millis() - lastMessageTime > TIMEOUT_MS) {
        // Timeout: nessun messaggio da troppo tempo
        if (machine->getCurrentState() != STATE_NETWORK_LOST &&
            machine->getCurrentState() != STATE_START) {
            
            Serial.println("⚠️  Communication timeout!");
            machine->connectionLost();
        }
        
        // Reset timer per evitare spam
        lastMessageTime = millis();
    }
}

void CommunicationTask::parseCommand(String command) {
    command.trim();
    
    Serial.printf("📨 Command received: \"%s\"\n", command.c_str());
    
    // Invia alla RoboticArmMachine per elaborazione
    machine->receiveCommand(command);
    
    // Conferma ricezione
    bt->println("OK");
}