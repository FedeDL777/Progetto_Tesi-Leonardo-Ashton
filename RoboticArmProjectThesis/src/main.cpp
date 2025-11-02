
#include <Arduino.h>
#include "RoboticArmMachine.h"
#include "kernel/Scheduler.h"
#include "kernel/task/include/Comunication_Task_ESPNOW.h"
#include "kernel/task/include/Motion_Task.h"
#include "kernel/task/include/SystemTask.h"

// ============================================================================
// OGGETTI GLOBALI
// ============================================================================

RoboticArmMachine* machine;
Scheduler scheduler;

CommunicationTask* commTask;
MotionTask* motionTask;
SystemTask* systemTask;

// ============================================================================
// SETUP
// ============================================================================

void setup() {
    Serial.begin(115200);
    delay(1000);

    // ========================================
    // Inizializza RoboticArmMachine
    // ========================================
    
    Serial.println("Inizializzazione RoboticArmMachine...\n");
    
    machine = new RoboticArmMachine();
    machine->begin();
    
    Serial.println("RoboticArmMachine pronta!\n");
    
    // ========================================
    // Inizializza Scheduler
    // ========================================
    
    Serial.println("Inizializzazione Scheduler...\n");
    
    scheduler.init(20);  // Base period 20ms (50Hz)

    Serial.println("Scheduler inizializzato (20ms base period)\n");

    // ========================================
    // Crea e Aggiungi Task
    // ========================================
    
    // Communication Task (ESP-NOW) - ogni 100ms
    commTask = new CommunicationTask(machine, 5000);
    
    while(!commTask->begin()) {
        Serial.println("Errore CommunicationTask");
        delay(1000);
    }
    commTask->init(100);  // 100ms period
    scheduler.addTask(commTask);
    Serial.println("CommunicationTask aggiunto (100ms)");

    // Motion Task - ogni 20ms (stessa frequenza base)
    motionTask = new MotionTask(machine);
    motionTask->init(20);  // 20ms period (50Hz servo)
    scheduler.addTask(motionTask);
    Serial.println("MotionTask aggiunto (20ms)");
    
    // System Task - ogni 50ms
    systemTask = new SystemTask(machine);
    systemTask->init(50);  // 50ms period
    scheduler.addTask(systemTask);
    Serial.println("✅ SystemTask aggiunto (50ms)\n");
    
}

// ============================================================================
// LOOP - USA SCHEDULER ESISTENTE
// ============================================================================

void loop() {
    // ========================================
    // Scheduler gestisce automaticamente timing
    // ========================================
    
    scheduler.schedule();
    
}