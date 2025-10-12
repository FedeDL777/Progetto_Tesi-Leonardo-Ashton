#ifndef __RAM__
#define __RAM__

#include "include/ServoMG66R.h"
#include "include/set_up.h"
#include <Adafruit_PWMServoDriver.h>

class RoboticArmMachine
{
public:
    RoboticArmMachine();

    void begin();
    void moveBaseServo(int angle);
    void moveElbowServo(int angle);
    void moveWristServo(int angle);
    void moveClawServo(int angle);

private:
    // baseServo;
    ServoMotorMG66R *elbowServo;
    ServoMotorMG66R *wristServo;
    ServoMotorMG66R *clawServo;
    Adafruit_PWMServoDriver pwm;
};

#endif