/*******************************************************************************
 * ROBOTIC ARM MACHINE - HEADER
 * 
 * Gestisce: Servo, FSM, LED, Button, I2C
 * Include la logica di stato completa
 * 
 * Autore: Federico De Leonardis
 ******************************************************************************/

#ifndef __RAM__
#define __RAM__

#include "include/ServoMG66R.h"
#include "include/Servo20Diy.h"
#include "include/set_up.h"
#include "include/Led.h"
#include "include/Button.h"
#include <Adafruit_PWMServoDriver.h>


enum RobotStateEnum {
    STATE_START = 0,
    STATE_CONNECTED = 1,
    STATE_WORKING = 2,
    STATE_PROBLEM_SERVO = 3,
    STATE_NETWORK_LOST = 4,
    STATE_IDLE = 5
};

class RoboticArmMachine {

public:

    RoboticArmMachine();
    void begin();
    

    /**
     * Aggiorna lo stato della macchina
     * Chiamare periodicamente (ogni 50ms)
     */
    void update();
    


    int getCurrentState() const;
    String getStateString() const;
    void transitionTo(int newState);
    
    // TRANSIZIONI PUBBLICHE
    
    void tryConnectToNetwork();
    void connectionEstablished();
    void connectionLost();
    void startWorking();
    void stopWorking();
    void servoError(String errorMsg);
    void servoErrorResolved();
    void receiveCommand(String command);

    // MOVIMENTO SERVO
    void moveBaseServo(int angle);
    void moveElbowServo(int angle);
    void moveWristServo(int angle);
    void moveClawServo(int angle);
    

    void moveAllToSafePosition();
    
    void moveAllToCenter();

    
    int getBaseAngle() const;
    int getElbowAngle() const;
    int getWristAngle() const;
    int getClawAngle() const;
    
    /**
     * Verifica stato LED
     */
    bool isLedGreenOn() const;
    bool isLedRedOn() const;
    
    /**
     * Verifica stato pulsanti
     */
    bool isButtonWhitePressed();
    bool isButtonBluePressed();
    
    /**
     * Ottiene info debug
     */
    String getDebugInfo() const;
    
    // ========================================
    // SETTERS - SICUREZZA
    // ========================================
    
    /**
     * Imposta limiti di sicurezza per servo
     */
    void setBaseServoLimits(int min, int max);
    void setElbowServoLimits(int min, int max);
    void setWristServoLimits(int min, int max);
    void setClawServoLimits(int min, int max);
    
    /**
     * Abilita/disabilita controlli di sicurezza
     */
    void setSafetyEnabled(bool enabled);

private:
    // ========================================
    // OGGETTI
    // ========================================
    
    // Servo motori
    ServoMotor20Diy *baseServo;
    ServoMotor20Diy *elbowServo;
    ServoMotorMG66R *wristServo;
    ServoMotorMG66R *clawServo;
    
    // Driver e comunicazione
    Adafruit_PWMServoDriver pwm;
    
    // LED e Input
    Led *ledGreen;
    Led *ledRed;
    Button *buttonWhite;
    Button *buttonBlue;
    
    // ========================================
    // STATE VARIABLES
    // ========================================
    
    int currentState;
    int previousState;
    
    bool networkConnected;
    bool servoErrorFlag;
    String lastErrorMsg;
    String pendingCommand;
    
    unsigned long stateEntryTime;
    unsigned long lastNetworkCheck;
    unsigned long lastServoCheck;
    
    // Timeout configuration
    const unsigned long CONNECTION_TIMEOUT = 5000;
    const unsigned long NETWORK_CHECK_INTERVAL = 1000;
    const unsigned long SERVO_CHECK_INTERVAL = 500;
    const unsigned long ERROR_RECOVERY_TIMEOUT = 3000;
    
    // ========================================
    // HANDLERS DI STATO
    // ========================================
    
    void handleStart();
    void enterStart();
    void exitStart();
    
    void handleConnected();
    void enterConnected();
    void exitConnected();
    
    void handleWorking();
    void enterWorking();
    void exitWorking();
    
    void handleProblemServo();
    void enterProblemServo();
    void exitProblemServo();
    
    void handleNetworkLost();
    void enterNetworkLost();
    void exitNetworkLost();
    
    void handleIdle();
    void enterIdle();
    void exitIdle();
    
    // ========================================
    // UTILITY PRIVATE
    // ========================================
    
    void setLedState(bool green, bool red);
    void logStateChange(int oldState, int newState);
    void checkNetwork();
    void checkServoHealth();
    void processCommand(String command);
    void bringToSafePosition();
    
};

#endif