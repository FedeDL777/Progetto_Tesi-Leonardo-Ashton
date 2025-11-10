#include "../include/SystemTask.h"


SystemTask::SystemTask(RoboticArmMachine* machine)
    : machine(machine),
      ledState(false),
      lastLedBlink(0),
      lastButtonCheck(0),
      lastWhiteState(false),
      lastBlueState(false)
{
}


// TASK TICK


void SystemTask::tick() {
          machine->update();
    
    // ✅ Edge detection gestito dalla classe Button!
    if (machine->wasButtonWhitePressed()) {
        Serial.println("Pulsante BIANCO");
        machine->moveAllToSafePosition();
        machine->clearCommands();
    }
    
    if (machine->wasButtonBluePressed()) {
        Serial.println("Pulsante BLU");
        if (machine->getCurrentState() == STATE_WORKING) {
            machine->stopWorking();
            machine->clearCommands();
        }
    }
    
    
}