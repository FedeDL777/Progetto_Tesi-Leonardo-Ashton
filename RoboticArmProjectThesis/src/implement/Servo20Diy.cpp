
#include "include/Servo20Diy.h"

// Caratteristiche specifiche servo 270°
#define SERVO_270_MIN_PULSE  102   // 500μs → 0°
#define SERVO_270_MAX_PULSE  512   // 2500μs → 270°
#define SERVO_270_MIN_ANGLE  0
#define SERVO_270_MAX_ANGLE  270

ServoMotor20Diy::ServoMotor20Diy(
    int channel,
    int safeMin,
    int safeMax
) : ServoMotor(
    channel,
    SERVO_270_MIN_PULSE,
    SERVO_270_MAX_PULSE,
    SERVO_270_MIN_ANGLE,
    SERVO_270_MAX_ANGLE,
    (safeMin >= 0) ? safeMin : SERVO_270_MIN_ANGLE,
    (safeMax >= 0) ? safeMax : SERVO_270_MAX_ANGLE
) {
    Serial.println("✅ ServoMotor20Diy (270°) initialized");
}

