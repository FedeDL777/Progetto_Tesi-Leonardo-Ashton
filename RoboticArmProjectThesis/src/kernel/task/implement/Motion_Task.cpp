#include "kernel/task/include/Motion_Task.h"

void MotionControlTask::tick() {
    // Se c'è un comando attivo
    if (currentCommand.active) {
        executeCurrentMotion();
    }
    // Altrimenti prendi il prossimo dalla coda
    else if (queueHead != queueTail) {
        dequeueNextCommand();
    }
}

void MotionControlTask::executeCurrentMotion() {
    unsigned long elapsed = millis() - currentCommand.startTime;
    
    if (elapsed >= currentCommand.duration) {
        // Movimento completato
        switch (currentCommand.servoIndex) {
            case 0: machine->moveBaseServo(currentCommand.targetAngle); break;
            case 1: machine->moveElbowServo(currentCommand.targetAngle); break;
            case 2: machine->moveWristServo(currentCommand.targetAngle); break;
            case 3: machine->moveClawServo(currentCommand.targetAngle); break;
        }
        
        currentCommand.active = false;
        Serial.println("Motion completed");
    }
    else {
        // Interpolazione (movimento smooth)
        float progress = (float)elapsed / currentCommand.duration;
        
        int currentAngle;
        switch (currentCommand.servoIndex) {
            case 0: currentAngle = machine->getBaseAngle(); break;
            case 1: currentAngle = machine->getElbowAngle(); break;
            case 2: currentAngle = machine->getWristAngle(); break;
            case 3: currentAngle = machine->getClawAngle(); break;
        }
        
        int interpolated = interpolateAngle(
            currentAngle,
            currentCommand.targetAngle,
            progress
        );
        
        // Aggiorna servo
        switch (currentCommand.servoIndex) {
            case 0: machine->moveBaseServo(interpolated); break;
            case 1: machine->moveElbowServo(interpolated); break;
            case 2: machine->moveWristServo(interpolated); break;
            case 3: machine->moveClawServo(interpolated); break;
        }
    }
}