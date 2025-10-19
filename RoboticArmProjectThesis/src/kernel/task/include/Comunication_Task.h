#ifndef __COMMUNICATION_TASK__
#define __COMMUNICATION_TASK__

#include "Task.h"
#include "RoboticArmMachine.h"
#include <BluetoothSerial.h>

class CommunicationTask : public Task
{
public:
    CommunicationTask(
        RoboticArmMachine *machine,
        BluetoothSerial *bt);
    void tick() override;

    // Statistiche
    int getMessagesReceived() const { return messagesReceived; }
    int getMessagesDropped() const { return messagesDropped; }

private:
    RoboticArmMachine *machine;
    BluetoothSerial *bt;

    unsigned long lastMessageTime;
    const unsigned long TIMEOUT_MS = 5000;

    String messageBuffer;
    const int MAX_BUFFER_SIZE = 128;

    // Statistiche
    int messagesReceived;
    int messagesDropped;

    // Metodi privati
    void processIncomingData();
    void checkTimeout();
    void parseCommand(String command);
};

#endif