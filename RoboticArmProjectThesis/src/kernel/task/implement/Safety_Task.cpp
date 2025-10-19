#include "kernel/task/include/Safety_Task.h"

SafetyMonitorTask::SafetyMonitorTask(RoboticArmMachine* machine) {
    this->machine = machine;
    this->lastWhiteButtonState = false;
    this->lastBlueButtonState = false;
    this->lastWhiteDebounce = 0;
    this->lastBlueDebounce = 0;
    this->emergencyStops = 0;
    
    Serial.println("SafetyMonitorTask created");
}



void SafetyMonitorTask::tick() {
    checkEmergencyButtons();
    checkServoLimits();
    // checkObstacles();  //Con sensore ultrasonico
}

void SafetyMonitorTask::checkEmergencyButtons() {
    // Pulsante bianco (safe position)
    bool whitePressed = machine->isButtonWhitePressed();
    if (whitePressed && !lastWhiteButtonState) {
        if (millis() - lastWhiteDebounce > DEBOUNCE_DELAY) {
            Serial.println("Emergency: Safe position requested");
            machine->moveAllToSafePosition();
            emergencyStops++;
            lastWhiteDebounce = millis();
        }
    }
    lastWhiteButtonState = whitePressed;
    
    // Pulsante blu (stop immediato)
    bool bluePressed = machine->isButtonBluePressed();
    if (bluePressed && !lastBlueButtonState) {
        if (millis() - lastBlueDebounce > DEBOUNCE_DELAY) {
            Serial.println("Emergency STOP!");
            machine->stopWorking();
            machine->connectionLost();  // Forza stato di errore
            emergencyStops++;
            lastBlueDebounce = millis();
        }
    }
    lastBlueButtonState = bluePressed;
}

void SafetyMonitorTask::checkServoLimits() {
    // Verifica che tutti i servo siano nei limiti
    bool allSafe = true;
    
    if (!machine->areAllAngleSafe()) {
        allSafe = false;
    }
    
    if (!allSafe) {
        machine->servoError("Servo out of safe limits");
    }
}