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
    
    // Limiti di sicurezza (default = limiti fisici)
    this->safeMinAngle = (safeMin >= 0) ? safeMin : minAngle;
    this->safeMaxAngle = (safeMax >= 0) ? safeMax : maxAngle;
    
    // Inizializza al centro del range sicuro
    this->currentAngle = (safeMinAngle + safeMaxAngle) / 2;
    this->trim = 0;
    this->safetyEnabled = true;
    this->moving = false;
    
    Serial.printf(
        "ServoMotor Ch%d | Range: %d°-%d° | Safe: %d°-%d° | PWM: %d-%d\n",
        channel, minAngle, maxAngle, safeMinAngle, safeMaxAngle, minPulse, maxPulse
    );
}

// ============================================================================
// MOVIMENTO IMMEDIATO
// ============================================================================

void ServoMotor::moveServo(Adafruit_PWMServoDriver& pwm, float angle) {
    // Applica limiti di sicurezza
    angle = applySafetyLimits(angle);
    
    // Converti a PWM
    uint16_t pulse = angleToPulse(angle);
    
    // Invia al servo
    pwm.setPWM(channel, 0, pulse);
    
    // Aggiorna stato
    currentAngle = (int)angle;
    moving = false;
}

void ServoMotor::moveRelative(Adafruit_PWMServoDriver& pwm, int delta) {
    float newAngle = currentAngle + delta;
    moveServo(pwm, newAngle);
}

// ============================================================================
// MOVIMENTO SMOOTH NON-BLOCCANTE
// ============================================================================

void ServoMotor::startSmoothMove(
    Adafruit_PWMServoDriver& pwm, 
    float targetAngle, 
    uint16_t duration, 
    uint16_t steps  // Ignorato
) {
    targetAngle = applySafetyLimits(targetAngle);
    
    // Se già in movimento, completa quello attuale
    if (moving) {
        currentAngle = (int)moveTargetAngle;
    }
    
    // Inizializza nuovo movimento
    moving = true;
    moveStartAngle = currentAngle;
    moveTargetAngle = targetAngle;
    moveStartTime = millis();
    moveDuration = duration;
    
    Serial.printf(
        "Ch%d: %.0f° → %.0f° in %dms\n",
        channel, moveStartAngle, moveTargetAngle, duration
    );
}

bool ServoMotor::updateSmoothMove(Adafruit_PWMServoDriver& pwm) {
    if (!moving) {
        return true;  // Nessun movimento attivo
    }
    
    unsigned long elapsed = millis() - moveStartTime;
    
    // Movimento completato
    if (elapsed >= moveDuration) {
        // Posizione finale esatta
        uint16_t pulse = angleToPulse(moveTargetAngle);
        pwm.setPWM(channel, 0, pulse);
        
        currentAngle = (int)moveTargetAngle;
        moving = false;
        
        Serial.printf("Ch%d: Raggiunto %.0f°\n", channel, moveTargetAngle);
        return true;
    }
    
    // Calcola posizione corrente (interpolazione lineare)
    float progress = (float)elapsed / moveDuration;  // 0.0 → 1.0
    float angleDiff = moveTargetAngle - moveStartAngle;
    float currentPos = moveStartAngle + (angleDiff * progress);
    
    // Aggiorna servo
    uint16_t pulse = angleToPulse(currentPos);
    pwm.setPWM(channel, 0, pulse);
    
    return false;  // Movimento in corso
}

void ServoMotor::stopMove() {
    if (moving) {
        moving = false;
        Serial.printf("Ch%d: Movimento interrotto\n", channel);
    }
}

// ============================================================================
// POSIZIONI PREDEFINITE
// ============================================================================

void ServoMotor::moveToMin(Adafruit_PWMServoDriver& pwm) {
    moveServo(pwm, safeMinAngle);
}

void ServoMotor::moveToMax(Adafruit_PWMServoDriver& pwm) {
    moveServo(pwm, safeMaxAngle);
}

void ServoMotor::moveToCenter(Adafruit_PWMServoDriver& pwm) {
    int center = (safeMinAngle + safeMaxAngle) / 2;
    moveServo(pwm, center);
}

void ServoMotor::moveToSafePosition(Adafruit_PWMServoDriver& pwm, int angle) {
    moveServo(pwm, angle);
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
    info += "║  SERVO DEBUG Ch" + String(channel) + "                  ║\n";
    info += "╚════════════════════════════════════╝\n";
    info += "Current:  " + String(currentAngle) + "°\n";
    info += "Range:    " + String(minAngle) + "° - " + String(maxAngle) + "°\n";
    info += "Safety:   " + String(safeMinAngle) + "° - " + String(safeMaxAngle) + "°\n";
    info += "Moving:   " + String(moving ? "YES" : "NO") + "\n";
    if (moving) {
        info += "Target:   " + String((int)moveTargetAngle) + "°\n";
        info += "Progress: " + String((int)((millis() - moveStartTime) * 100.0 / moveDuration)) + "%\n";
    }
    return info;
}

// ============================================================================
// SETTERS
// ============================================================================

void ServoMotor::setSafetyLimits(int min, int max) {
    if (min < minAngle || max > maxAngle) {
        Serial.printf("Ch%d: Safety limits out of range!\n", channel);
        return;
    }
    
    if (min > max) {
        int temp = min;
        min = max;
        max = temp;
    }
    
    safeMinAngle = min;
    safeMaxAngle = max;
    
    Serial.printf("Ch%d: Safety %d° - %d°\n", channel, min, max);
}

void ServoMotor::setSafetyEnabled(bool enabled) {
    safetyEnabled = enabled;
    Serial.printf("Ch%d: Safety %s\n", channel, enabled ? "ON" : "OFF");
}

void ServoMotor::setTrim(int trimValue) {
    trim = trimValue;
    Serial.printf("Ch%d: Trim = %d\n", channel, trim);
}

// ============================================================================
// UTILITY PROTETTE
// ============================================================================

uint16_t ServoMotor::angleToPulse(float angle) {
    // Limita al range fisico
    angle = constrain(angle, minAngle, maxAngle);
    
    // Mappa angolo → PWM
    uint16_t pulse = map(
        angle * 10,
        minAngle * 10,
        maxAngle * 10,
        minPulse,
        maxPulse
    );
    
    // Applica trim
    pulse += trim;
    
    // Sicurezza finale
    pulse = constrain(pulse, minPulse, maxPulse);
    
    return pulse;
}

float ServoMotor::applySafetyLimits(float angle) {
    if (!safetyEnabled) {
        return angle;
    }
    
    if (angle < safeMinAngle) {
        return safeMinAngle;
    }
    
    if (angle > safeMaxAngle) {
        return safeMaxAngle;
    }
    
    return angle;
}