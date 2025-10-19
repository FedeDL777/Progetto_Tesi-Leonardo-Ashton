
#ifndef __SAFETY_MONITOR_TASK__
#define __SAFETY_MONITOR_TASK__

#include "kernel/Task.h"
#include "RoboticArmMachine.h"

class SafetyMonitorTask : public Task {
public:
    SafetyMonitorTask(RoboticArmMachine* machine);
    void tick() override;
    
    // Statistiche
    int getEmergencyStopsCount() const { return emergencyStops; }
    
private:
    RoboticArmMachine* machine;
    
    // Stato pulsanti (debouncing)
    bool lastWhiteButtonState;
    bool lastBlueButtonState;
    unsigned long lastWhiteDebounce;
    unsigned long lastBlueDebounce;
    const unsigned long DEBOUNCE_DELAY = 50;
    
    // Statistiche
    int emergencyStops;
    
    // Metodi privati
    void checkEmergencyButtons();
    void checkServoLimits();
    void checkObstacles();  // Se presente sensore ultrasonico
};

#endif