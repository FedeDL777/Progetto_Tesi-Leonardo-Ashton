#include "Arduino.h"
#ifndef __SETUP__
#define __SETUP__

#define BASE_SERVO 0
#define SERVO_ELBOW 4
#define SERVO_WRIST 5
#define SERVO_CLAW 6

// Pin I2C ESP8266 (default)
#define SDA_PIN D2 // GPIO4
#define SCL_PIN D1 // GPIO5

// Indirizzo I2C PCA9685 (default 0x40)
#define PCA9685_ADDRESS 0x40

#define SERVOMIN 102 // 500μs  → 0°
#define SERVOMAX 512 // 2500μs → 180°
#define SERVOMID 307 // 1500μs → 90° (centro)
#define SERVO_TRIM 0


#endif