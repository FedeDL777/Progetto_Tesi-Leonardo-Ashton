#include "kernel/task/include/Motion_Task.h"




MotionControlTask::MotionControlTask(RoboticArmMachine* machine) {
    this->machine = machine;
    this->currentCommand.active = false;
    this->queueHead = 0;
    this->queueTail = 0;
    
    Serial.println("✅ MotionControlTask created");
}

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

void MotionControlTask::queueMotion(int servoIndex, int targetAngle, int duration) {
    // TODO: Implementare accodamento
}

void MotionControlTask::stopAllMotion() {
    currentCommand.active = false;
    queueHead = 0;
    queueTail = 0;
}

bool MotionControlTask::isMotionComplete() const {
    return !currentCommand.active && (queueHead == queueTail);
}

void MotionControlTask::dequeueNextCommand() {
    // TODO: Implementare dequeue
}

int MotionControlTask::interpolateAngle(int start, int target, float progress) {
    return start + (int)((target - start) * progress);
}