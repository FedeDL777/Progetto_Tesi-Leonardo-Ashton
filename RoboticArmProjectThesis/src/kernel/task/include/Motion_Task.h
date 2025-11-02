
#ifndef __MOTION_TASK_H__
#define __MOTION_TASK_H__

#include "../include/Task.h"
#include "RoboticArmMachine.h"

class MotionTask : public Task {
public:
    MotionTask(RoboticArmMachine* machine);
    
    void tick() override;
    
    int getCommandsProcessed() const { return commandsProcessed; }
    int getCommandsFailed() const { return commandsFailed; }

private:
    RoboticArmMachine* machine;
    
    int commandsProcessed;
    int commandsFailed;
    
    unsigned long lastCommandTime;
    const unsigned long COMMAND_INTERVAL = 100;  // Min 100ms tra comandi
};

#endif