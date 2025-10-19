
#ifndef __SERVO_MOTOR_BASE__
#define __SERVO_MOTOR_BASE__

#include <Arduino.h>
#include <Adafruit_PWMServoDriver.h>

/**
 * Classe base per tutti i servo motori
 * 
 * Caratteristiche:
 * - Range di movimento personalizzabile
 * - Limiti di sicurezza configurabili
 * - Tracking posizione attuale
 * - Conversione automatica angolo → PWM
 */
class ServoMotor {

public:
    /**
     * @param channel    Canale PCA9685 (0-15)
     * @param minPulse   PWM minimo (default 102 = 500μs)
     * @param maxPulse   PWM massimo (default 512 = 2500μs)
     * @param minAngle   Angolo minimo di movimento (default 0°)
     * @param maxAngle   Angolo massimo di movimento (default 180°)
     * @param safeMin    Limite di sicurezza minimo (default = minAngle)
     * @param safeMax    Limite di sicurezza massimo (default = maxAngle)
     */
    ServoMotor(
        int channel,
        uint16_t minPulse = 102,
        uint16_t maxPulse = 512,
        int minAngle = 0,
        int maxAngle = 180,
        int safeMin = -1,
        int safeMax = -1
    );
    
    // ========================================
    // MOVIMENTO SERVO
    // ========================================
    
    /**
     * Muove servo a angolo specificato
     * Applica i limiti di sicurezza
     * 
     * @param pwm   Oggetto PCA9685
     * @param angle Angolo desiderato (0-180)
     */
    void moveServo(Adafruit_PWMServoDriver pwm, float angle);
    
    /**
     * Muove servo lentamente
     * 
     * @param pwm       Oggetto PCA9685
     * @param angle     Angolo desiderato
     * @param duration  Durata movimento in ms
     * @param steps     Numero step (default 50)
     */
    void moveServoSmooth(
        Adafruit_PWMServoDriver pwm,
        float angle,
        uint16_t duration,
        uint16_t steps = 50
    );
    
    /**
     * Sposta servo di X gradi dalla posizione attuale
     * 
     * @param pwm   Oggetto PCA9685
     * @param delta Incremento/decremento gradi
     */
    void moveRelative(Adafruit_PWMServoDriver pwm, int delta);
    
    // ========================================
    // POSIZIONI PREDEFINITE
    // ========================================
    
    /**
     * Muove servo alla posizione minima
     */
    void moveToMin(Adafruit_PWMServoDriver pwm);
    
    /**
     * Muove servo alla posizione massima
     */
    void moveToMax(Adafruit_PWMServoDriver pwm);
    
    /**
     * Muove servo al centro (min + max) / 2
     */
    void moveToCenter(Adafruit_PWMServoDriver pwm);
    
    /**
     * Muove servo alla posizione di sicurezza
     */
    void moveToSafePosition(Adafruit_PWMServoDriver pwm, int angle);
    
    // ========================================
    // GETTERS
    // ========================================
    
    /**
     * Ritorna angolo attuale
     */
    int getCurrentAngle() const;
    
    /**
     * Ritorna channel PCA9685
     */
    int getChannel() const;
    
    /**
     * Ritorna true se servo è in movimento
     */
    bool isMoving() const;
    
    /**
     * Ritorna true se angolo è dentro i limiti di sicurezza
     */
    bool isAngleSafe(int angle) const;
    
    /**
     * Ritorna info servo come stringa
     */
    String getDebugInfo() const;
    
    // ========================================
    // SETTERS
    // ========================================
    
    /**
     * Imposta nuovi limiti di sicurezza
     * 
     * @param min Angolo minimo di sicurezza
     * @param max Angolo massimo di sicurezza
     */
    void setSafetyLimits(int min, int max);
    
    /**
     * Abilita/disabilita controllo di sicurezza
     */
    void setSafetyEnabled(bool enabled);
    
    /**
     * Imposta trim offset in PWM
     */
    void setTrim(int trim);

protected:
    // ========================================
    // VARIABILI PROTETTE
    // ========================================
    
    int channel;                // Canale PCA9685
    int currentAngle;           // Posizione attuale (gradi)
    
    // Range fisico servo
    int minAngle;               // Min angolo fisico
    int maxAngle;               // Max angolo fisico
    uint16_t minPulse;          // PWM per minAngle
    uint16_t maxPulse;          // PWM per maxAngle
    
    // Range sicurezza
    int safeMinAngle;           // Min angolo SICURO
    int safeMaxAngle;           // Max angolo SICURO
    bool safetyEnabled;         // Abilita controlli sicurezza
    
    // Calibrazione
    int trim;                   // Offset PWM di calibrazione
    
    // Stato
    bool moving;                // True se in movimento
    unsigned long moveStartTime;
    unsigned long moveDuration;
    float moveStartAngle;
    float moveTargetAngle;
    
    // ========================================
    // UTILITY PROTETTE
    // ========================================
    
    /**
     * Converte angolo a PWM
     */
    uint16_t angleToPulse(float angle);
    
    /**
     * Applica limiti di sicurezza
     */
    float applySafetyLimits(float angle);
    
    /**
     * Stampa debug info
     */
    void printDebug(const char* msg, float angle, uint16_t pulse);
};

#endif