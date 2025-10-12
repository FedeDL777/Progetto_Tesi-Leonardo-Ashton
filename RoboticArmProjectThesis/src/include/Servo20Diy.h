#ifndef __SERVO_20_DIY_
#define __SERVO_20_DIY_

#include <arduino.h>

#include <Servo.h>
#include <Adafruit_PWMServoDriver.h>

#define SERVOMIN 102 // 500μs  → 0°
#define SERVOMAX 512 // 2500μs → 180°
#define SERVOMID 307 // 1500μs → 90° (centro)


#define MIN_ANGLE 0
#define MAX_ANGLE 180

class ServoMotor20Diy
{

public:
    ServoMotor20Diy(int channel);

    void moveServo(Adafruit_PWMServoDriver pwm, float angle);
    void on();
    void off();
    void fullyOpen();
    void close();
    void openDegree(int angle);

private:
    int channel;
    int currentAngle = 90; // Posizione attuale
    Servo motor;
};

#endif