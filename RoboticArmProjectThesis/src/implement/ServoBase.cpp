

#include "include/ServoBase.h"

// ============================================================================
// COSTRUTTORE
// ============================================================================

ServoMotor::ServoMotor(
    int channel,
    uint16_t minPulse,
    uint16_t maxPulse,
    int minAngle,
    int maxAngle,
    int safeMin,
    int safeMax
) {
    this->channel = channel;
    this->minPulse = minPulse;
    this->maxPulse = maxPulse;
    this->minAngle = minAngle;
    this->maxAngle = maxAngle;
    
    // Se non specificati, limiti di sicurezza = limiti fisici
    this->safeMinAngle = (safeMin >= 0) ? safeMin : minAngle;
    this->safeMaxAngle = (safeMax >= 0) ? safeMax : maxAngle;
    
    this->currentAngle = (minAngle + maxAngle) / 2;  // Inizia al centro
    this->trim = 0;
    this->safetyEnabled = true;
    this->moving = false;
    
    Serial.printf(
        "✅ ServoMotor initialized\n"
        "   Channel: %d\n"
        "   Range: %d° - %d°\n"
        "   Safety: %d° - %d°\n"
        "   PWM: %d - %d\n\n",
        channel, minAngle, maxAngle,
        safeMinAngle, safeMaxAngle,
        minPulse, maxPulse
    );
}

// ============================================================================
// MOVIMENTO SERVO
// ============================================================================

void ServoMotor::moveServo(Adafruit_PWMServoDriver pwm, float angle) {
    // Applica limiti di sicurezza
    angle = applySafetyLimits(angle);
    
    // Converti a PWM
    uint16_t pulse = angleToPulse(angle);
    
    // Invia al servo
    pwm.setPWM(channel, 0, pulse);
    
    // Aggiorna stato
    currentAngle = (int)angle;
    moving = false;
    
    // Debug
    printDebug("moveServo", angle, pulse);
}

void ServoMotor::moveServoSmooth(
    Adafruit_PWMServoDriver pwm,
    float angle,
    uint16_t duration,
    uint16_t steps
) {
    angle = applySafetyLimits(angle);
    
    float angleDiff = angle - currentAngle;
    float delay_per_step = (float)duration / steps;
    // Debug
    Serial.printf(
        "🔄 Smooth move: %d° → %.0f° in %dms (%d steps)\n",
        currentAngle, angle, duration, steps
    );
    
    moving = true;
    moveStartAngle = currentAngle;
    moveTargetAngle = angle;
    moveStartTime = millis();
    moveDuration = duration;
    
    for (int i = 0; i <= steps; i++) {
        float currentPos = currentAngle + (angleDiff * i / steps);
        uint16_t pulse = angleToPulse(currentPos);
        
        pwm.setPWM(channel, 0, pulse);
        delay(delay_per_step);
    }
    
    currentAngle = (int)angle;
    moving = false;
}

void ServoMotor::moveRelative(Adafruit_PWMServoDriver pwm, int delta) {
    float newAngle = currentAngle + delta;
    moveServo(pwm, newAngle);
}

// ============================================================================
// POSIZIONI PREDEFINITE
// ============================================================================

void ServoMotor::moveToMin(Adafruit_PWMServoDriver pwm) {
    Serial.printf("Movimento a MIN (%d°)\n", safeMinAngle);
    moveServo(pwm, safeMinAngle);
}

void ServoMotor::moveToMax(Adafruit_PWMServoDriver pwm) {
    Serial.printf("Movimento a MAX (%d°)\n", safeMaxAngle);
    moveServo(pwm, safeMaxAngle);
}

void ServoMotor::moveToCenter(Adafruit_PWMServoDriver pwm) {
    int center = (safeMinAngle + safeMaxAngle) / 2;
    Serial.printf("Movimento a CENTER (%d°)\n", center);
    moveServo(pwm, center);
}

void ServoMotor::moveToSafePosition(Adafruit_PWMServoDriver pwm) {
    int safePos = (safeMinAngle + safeMaxAngle) / 2;
    Serial.printf("Movimento a SAFE POSITION (%d°)\n", safePos);
    moveServo(pwm, safePos);
}

// ============================================================================
// GETTERS
// ============================================================================

int ServoMotor::getCurrentAngle() const {
    return currentAngle;
}

int ServoMotor::getChannel() const {
    return channel;
}

bool ServoMotor::isMoving() const {
    return moving;
}

bool ServoMotor::isAngleSafe(int angle) const {
    if (!safetyEnabled) return true;
    return (angle >= safeMinAngle) && (angle <= safeMaxAngle);
}

String ServoMotor::getDebugInfo() const {
    String info = "\n╔════════════════════════════════════╗\n";
    info += "║  SERVO MOTOR DEBUG INFO            ║\n";
    info += "╚════════════════════════════════════╝\n\n";
    
    info += "Channel:     " + String(channel) + "\n";
    info += "Current:     " + String(currentAngle) + "°\n";
    info += "Range:       " + String(minAngle) + "° - " + String(maxAngle) + "°\n";
    info += "Safety:      " + String(safeMinAngle) + "° - " + String(safeMaxAngle) + "°\n";
    info += "Enabled:     " + String(safetyEnabled ? "YES" : "NO") + "\n";
    info += "Moving:      " + String(moving ? "YES" : "NO") + "\n";
    info += "PWM:         " + String(minPulse) + " - " + String(maxPulse) + "\n";
    info += "Trim:        " + String(trim) + "\n\n";
    
    return info;
}

// ============================================================================
// SETTERS
// ============================================================================

void ServoMotor::setSafetyLimits(int min, int max) {
    if (min < minAngle || max > maxAngle) {
        Serial.printf("⚠️  SAFETY LIMITS OUT OF RANGE!\n");
        Serial.printf("   Physical range: %d° - %d°\n", minAngle, maxAngle);
        Serial.printf("   Requested: %d° - %d°\n", min, max);
        return;
    }
    
    if (min > max) {
        Serial.printf("⚠️  Min > Max! Swapping...\n");
        int temp = min;
        min = max;
        max = temp;
    }
    
    safeMinAngle = min;
    safeMaxAngle = max;
    
    Serial.printf(
        "✅ Safety limits updated: %d° - %d°\n",
        safeMinAngle, safeMaxAngle
    );
}

void ServoMotor::setSafetyEnabled(bool enabled) {
    safetyEnabled = enabled;
    Serial.printf("Safety: %s\n", enabled ? "ENABLED " : "DISABLED");
}

void ServoMotor::setTrim(int trimValue) {
    trim = trimValue;
    Serial.printf("Trim offset set to: %d\n", trim);
}

// ============================================================================
// UTILITY PROTETTE
// ============================================================================

uint16_t ServoMotor::angleToPulse(float angle) {
    // Limita angle al range fisico
    angle = constrain(angle, minAngle, maxAngle);
    
    // Mappa angle a PWM
    uint16_t pulse = map(
        angle * 10,
        minAngle * 10,
        maxAngle * 10,
        minPulse,
        maxPulse
    );
    
    // Applica trim
    pulse += trim;
    
    // Assicura limiti PWM
    pulse = constrain(pulse, minPulse, maxPulse);
    
    return pulse;
}

float ServoMotor::applySafetyLimits(float angle) {
    if (!safetyEnabled) {
        return angle;
    }
    
    if (angle < safeMinAngle) {
        Serial.printf(
            "⚠️  Angle %.0f° below safety limit %d°\n",
            angle, safeMinAngle
        );
        return safeMinAngle;
    }
    
    if (angle > safeMaxAngle) {
        Serial.printf(
            "⚠️  Angle %.0f° above safety limit %d°\n",
            angle, safeMaxAngle
        );
        return safeMaxAngle;
    }
    
    return angle;
}

void ServoMotor::printDebug(const char* msg, float angle, uint16_t pulse) {
    Serial.printf(
        "🎯 CH%d: %s → %.1f° (PWM: %d)\n",
        channel, msg, angle, pulse
    );
}