#include "include/ServoMG66R.h"
#include "utility/PCA9685.cpp"
#include "include/set_up.h"


ServoMotorMG66R::ServoMotorMG66R(int channel)
{
    this->channel = channel;
}


void ServoMotorMG66R::moveServo(Adafruit_PWMServoDriver pwm, float angle)
{
    uint16_t pulse = angleToPulse(angle, MIN_ANGLE, MAX_ANGLE);
    pwm.setPWM(channel, 0, pulse);

    this->currentAngle = angle;

    Serial.printf("Servo CH%d → %.1f° (PWM: %d)\n", channel, angle, pulse);
}

