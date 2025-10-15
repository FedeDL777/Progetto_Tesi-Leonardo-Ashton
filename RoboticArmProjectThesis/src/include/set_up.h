#include "Arduino.h"
#include <Adafruit_PWMServoDriver.h>

#ifndef __SETUP__
#define __SETUP__

// Canali servo sul PCA9685
#define BASE_SERVO 0
#define SERVO_ELBOW 4
#define SERVO_WRIST 8
#define SERVO_CLAW 12

//Pin I2C ESP32 (standard consigliati)
#define SDA_PIN 21 // pin SDA
#define SCL_PIN 22 // pin SCL

//LED e pulsanti — puoi adattarli in base al tuo cablaggio
#define LED_GREEN 16        // GPIO16 → LED verde (stato OK)
#define LED_RED 17          // GPIO17 → LED rosso (errore)
#define BUTTON_WHITE_PIN 14 // GPIO14 → pulsante bianco (start/safe)
#define BUTTON_BLUE_PIN 12  // GPIO12 → pulsante blu (stop/emergency)

//Indirizzo I2C del driver PCA9685
#define PCA9685_ADDRESS 0x40

//Parametri servo (calibrabili)
#define SERVOMIN 102 // 500μs → 0°
#define SERVOMAX 512 // 2500μs → 180°
#define SERVOMID 307 // 1500μs → 90° centro
#define SERVO_TRIM 0

#endif
