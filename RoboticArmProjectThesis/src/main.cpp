#include "RoboticArmMachine.h"
#include "kernel/Scheduler.h"

#include "kernel/task/include/Motion_Task.h"

//#include "kernel/task/include/FSM_Task.h"

#include "kernel\task\include\Comunication_Task_ESPNOW.h"
#include "kernel\task\include\Safety_Task.h"
#include <BluetoothSerial.h>

// Oggetti globali
RoboticArmMachine* machine;
BluetoothSerial SerialBT;
Scheduler scheduler;

// Task
//FSMTask* fsmTask;
CommunicationTask_ESPNOW* commTask;
MotionControlTask* motionTask;
SafetyMonitorTask* safetyTask;
//StatusFeedbackTask* statusTask;

void setup() {
    Serial.begin(115200);
    
    // Inizializza macchina
    machine = new RoboticArmMachine();
    machine->begin();
    
    // Inizializza Bluetooth
    SerialBT.begin("RobotArm");
    Serial.println("✅ Bluetooth ready: RobotArm\n");
    
    // Inizializza scheduler
    scheduler.init(50);  // 50ms base period
    
    commTask = new CommunicationTask_ESPNOW(machine);
    
    // ⚠️ IMPORTANTE: Inizializza ESP-NOW PRIMA di addTask!
    if (!commTask->initESPNow()) {
        Serial.println("Errore ESP-NOW!");
        while(1) delay(1000);
    }
    
    commTask->init(50);
    scheduler.addTask(commTask);
    

    // Crea e registra task
    //fsmTask = new FSMTask(machine);
    //fsmTask->init(50);  // 50ms (20Hz) - CRITICO
    //scheduler.addTask(fsmTask);

    
    motionTask = new MotionControlTask(machine);
    motionTask->init(100);  // 100ms (10Hz) - MEDIA PRIORITÀ
    scheduler.addTask(motionTask);
    
    safetyTask = new SafetyMonitorTask(machine);
    safetyTask->init(200);  // 200ms (5Hz) - BASSA PRIORITÀ
    scheduler.addTask(safetyTask);
    
    //statusTask = new StatusFeedbackTask(machine, &SerialBT);
    //statusTask->init(500);  // 500ms (2Hz) - MOLTO BASSA
    //scheduler.addTask(statusTask);
    
    Serial.println("All tasks initialized\n");
}

void loop() {
    scheduler.schedule();
}