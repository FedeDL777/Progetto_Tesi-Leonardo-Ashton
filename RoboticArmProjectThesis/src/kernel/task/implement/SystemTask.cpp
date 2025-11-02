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

// ============================================================================
// TASK TICK
// ============================================================================

void SystemTask::tick() {
    // ========================================
    // 1. Aggiorna FSM
    // ========================================
    
    machine->update();
    

    unsigned long now = millis();
    // ========================================
    // 3. Polling Pulsanti
    // ========================================
    
    if (now - lastButtonCheck > 100) {
        bool whitePressed = machine->isButtonWhitePressed();
        bool bluePressed = machine->isButtonBluePressed();
        
        // Edge detection pulsante bianco
        if (whitePressed && !lastWhiteState) {
            machine->moveAllToSafePosition();
            machine->clearCommands();
        }
        
        // Edge detection pulsante blu
        if (bluePressed && !lastBlueState) {
            if (machine->getCurrentState() == STATE_WORKING) {
                machine->stopWorking();
                machine->clearCommands();
            }
        }
        
        lastWhiteState = whitePressed;
        lastBlueState = bluePressed;
        lastButtonCheck = now;
    }
    
    // ========================================
    // 4. Log periodico
    // ========================================
    
}