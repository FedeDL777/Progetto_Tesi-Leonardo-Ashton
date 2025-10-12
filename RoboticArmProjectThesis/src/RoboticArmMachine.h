#ifndef __RAM__
#define __RAM__

#include "include/ServoMG66R.h"
#include "include/Servo20Diy.h"
#include "include/set_up.h"
#include <Adafruit_PWMServoDriver.h>

class RoboticArmMachine
{
public:
    RoboticArmMachine();

        enum RobotState
    {
        STATE_START = 0,
        STATE_CONNECTED = 1,
        STATE_WORKING = 2,
        STATE_PROBLEM_SERVO = 3,
        STATE_NETWORK_LOST = 4,
        STATE_IDLE = 5
    };
    void begin(); 

    void update();
    
    // Getters
    RobotState getCurrentState();
    String getStateString();
    
    // Setters (transizioni)
    void tryConnectToNetwork();
    void connectionEstablished();
    void connectionLost();
    void startWorking();
    void stopWorking();
    void servoError(String errorMsg);
    void servoErrorResolved();
    void receiveCommand(String command);
    
    void setLedState(bool green, bool red);
    void moveBaseServo(int angle);
    void moveElbowServo(int angle);
    void moveWristServo(int angle);
    void moveClawServo(int angle);

private:
    ServoMotor20Diy *baseServo;
    ServoMotorMG66R *elbowServo;
    ServoMotorMG66R *wristServo;
    ServoMotorMG66R *clawServo;
    Adafruit_PWMServoDriver pwm;
    RobotState currentState;
    RobotState previousState;

    bool networkConnected;
    bool servoError;
    String lastErrorMsg;
    String pendingCommand;

};

#endif