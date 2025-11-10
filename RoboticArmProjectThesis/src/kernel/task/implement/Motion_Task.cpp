
#include "..\include\Motion_Task.h"


// COSTRUTTORE


MotionTask::MotionTask(RoboticArmMachine* machine)
    : machine(machine),
      commandsProcessed(0),
      commandsFailed(0),
      lastCommandTime(0)
{
}


// TASK TICK


void MotionTask::tick() {

    // 1. Aggiorna SEMPRE movimenti servo

    
    machine->updateServoMovements();
    

    // 2. Processa comandi dalla coda

    
    unsigned long now = millis();
    
    // Throttling
    if (now - lastCommandTime < COMMAND_INTERVAL) {
        return;
    }
    
    // Se non ci sono comandi, esci
    if (!machine->hasCommands()) {
        return;
    }
    
    // Se servo ancora in movimento, aspetta
    if (machine->isAnyServoMoving()) {
        return;
    }
    

    // 3. Estrai e esegui comando

    
    String cmd = machine->popCommand();
    
    if (cmd.length() > 0) {
        Serial.printf("Esecuzione: \"%s\"\n", cmd.c_str());
        
        bool success = machine->executeCommand(cmd);
        
        if (success) {
            commandsProcessed++;
        } else {
            commandsFailed++;
            Serial.println("Comando fallito");
        }
        
        lastCommandTime = now;
    }
    
}