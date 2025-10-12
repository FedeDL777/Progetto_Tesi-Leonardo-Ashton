#include "include/Servo20Diy.h"

ServoMotor20Diy::ServoMotor20Diy(int channel)
{
    this->channel = channel;
}

void ServoMotor20Diy::moveServo(Adafruit_PWMServoDriver pwm, float angle)
{
    int pulseWidth = map(angle, MIN_ANGLE, MAX_ANGLE, SERVOMIN, SERVOMAX);
    pwm.setPWM(channel, 0, pulseWidth);
    
    this->currentAngle = angle;

    Serial.printf("Servo CH%d → %.1f° (PWM: %d)\n", channel, angle, pulseWidth);
}