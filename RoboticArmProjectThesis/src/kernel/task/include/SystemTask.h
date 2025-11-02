#ifndef __SYSTEM_TASK_H__
#define __SYSTEM_TASK_H__

#include "Task.h"
#include "RoboticArmMachine.h"

class SystemTask : public Task {
public:
    SystemTask(RoboticArmMachine* machine);
    
    void tick() override;

private:
    RoboticArmMachine* machine;
    
    bool ledState;
    unsigned long lastLedBlink;
    
    unsigned long lastButtonCheck;
    bool lastWhiteState;
    bool lastBlueState;
};

#endif