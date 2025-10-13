#include "include/ServoMG66R.h"

// Caratteristiche specifiche MG66R
#define MG66R_MIN_PULSE  102   // 500μs → 0°
#define MG66R_MAX_PULSE  512   // 2500μs → 180°
#define MG66R_MIN_ANGLE  0
#define MG66R_MAX_ANGLE  180

ServoMotorMG66R::ServoMotorMG66R(
    int channel,
    int safeMin,
    int safeMax
) : ServoMotor(
    channel,
    MG66R_MIN_PULSE,
    MG66R_MAX_PULSE,
    MG66R_MIN_ANGLE,
    MG66R_MAX_ANGLE,
    (safeMin >= 0) ? safeMin : MG66R_MIN_ANGLE,
    (safeMax >= 0) ? safeMax : MG66R_MAX_ANGLE
) {
    Serial.println("✅ ServoMotorMG66R initialized");
}

void ServoMotorMG66R::calibrate() {
    Serial.println("\n╔════════════════════════════════════╗");
    Serial.println("║  MG66R CALIBRATION                 ║");
    Serial.println("╚════════════════════════════════════╝\n");
    
    Serial.println("Calibration procedure:");
    Serial.println("1. Servo will move to 0°");
    Serial.println("2. Then to 180°");
    Serial.println("3. Finally to center 90°\n");
    
    // Potrai aggiungere logica di calibrazione qui
}