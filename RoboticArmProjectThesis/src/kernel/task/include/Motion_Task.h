#ifndef __MOTION_CONTROL_TASK__
#define __MOTION_CONTROL_TASK__

#include "kernel/Task.h"
#include "RoboticArmMachine.h"

struct MotionCommand {
    bool active;
    int servoIndex;  // 0=base, 1=elbow, 2=wrist, 3=claw
    int targetAngle;
    unsigned long duration;  // ms
    unsigned long startTime;
};

class MotionControlTask : public Task {
public:
    MotionControlTask(RoboticArmMachine* machine);
    void tick() override;
    
    // Metodi pubblici per accodare movimenti
    void queueMotion(int servoIndex, int targetAngle, int duration = 1000);
    void stopAllMotion();
    
    bool isMotionComplete() const;
    
private:
    RoboticArmMachine* machine;
    
    MotionCommand currentCommand;
    
    // Queue di comandi (semplice, per ora)
    static const int MAX_QUEUE_SIZE = 10;
    MotionCommand commandQueue[MAX_QUEUE_SIZE];
    int queueHead;
    int queueTail;
    
    // Metodi privati
    void executeCurrentMotion();
    void dequeueNextCommand();
    int interpolateAngle(int start, int target, float progress);
};

#endif