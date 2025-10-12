#ifndef __SERVO_MOTOR_MG66R__
#define __SERVO_MOTOR_MG66R__


#include <arduino.h>

#include <Servo.h>	

#define SERVOMIN 102 // 500μs  → 0°
#define SERVOMAX 512 // 2500μs → 180°
#define SERVOMID 307 // 1500μs → 90° (centro)


#define MIN_ANGLE 0
#define MAX_ANGLE 180


class ServoMotorMG66R{

public:
  ServoMotorMG66R(int channel);

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


/**
 * Muove servo a posizione specificata
 * @param channel Canale PCA9685 (0-15)
 * @param angle Angolo desiderato (0-180)
 */

#endif