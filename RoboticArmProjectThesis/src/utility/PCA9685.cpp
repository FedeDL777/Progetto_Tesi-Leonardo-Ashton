#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>
#include "include/set_up.h"

/**
 * Converte angolo (0-180°) in pulse width per PCA9685
 * @param angle Angolo desiderato (0-180)
 * @return Valore PWM per PCA9685 (SERVOMIN-SERVOMAX)
 */
uint16_t angleToPulse(float angle, int MinAngle, int MaxAngle)
{
    // Limita angolo a range valido (MG996R è 180°, non 270°!)
    angle = constrain(angle, MinAngle, MaxAngle);

    // Mappa angolo su range PWM
    uint16_t pulse = map(angle * 10, MinAngle * 10, MaxAngle * 10, SERVOMIN, SERVOMAX);

    // Applica trim di calibrazione
    pulse += SERVO_TRIM;

    // Assicura che rimanga nei limiti
    pulse = constrain(pulse, SERVOMIN, SERVOMAX);

    return pulse;
}

