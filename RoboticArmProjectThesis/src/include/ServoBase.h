#ifndef __SERVO_MOTOR_BASE__
#define __SERVO_MOTOR_BASE__

#include <Arduino.h>
#include <Adafruit_PWMServoDriver.h>

/**
 * Classe base per tutti i servo motori
 * Supporta movimenti NON-BLOCCANTI per scheduler
 */
class ServoMotor {

public:
    /**
     * Costruttore
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
    
// MOVIMENTO IMMEDIATO

    /**
     * Muove servo istantaneamente all'angolo specificato
     */
    void moveServo(Adafruit_PWMServoDriver& pwm, float angle);
    
    /**
     * Muove servo relativamente alla posizione attuale
     */
    void moveRelative(Adafruit_PWMServoDriver& pwm, int delta);
    
// MOVIMENTO SMOOTH NON-BLOCCANTE

    /**
     * Avvia movimento smooth (NON-BLOCCANTE)
     * Da chiamare UNA VOLTA per iniziare il movimento
     * 
     * @param pwm         Driver PWM
     * @param targetAngle Angolo destinazione
     * @param duration    Durata totale (ms)
     * @param steps       Numero di step (ignorato, usa tempo)
     */
    void startSmoothMove(
        Adafruit_PWMServoDriver& pwm, 
        float targetAngle, 
        uint16_t duration, 
        uint16_t steps = 0  // Ignorato, compatibilità
    );
    
    /**
     * Aggiorna movimento smooth in corso
     * Da chiamare CONTINUAMENTE dallo scheduler
     * 
     * @param pwm Driver PWM
     * @return true se movimento completato, false altrimenti
     */
    bool updateSmoothMove(Adafruit_PWMServoDriver& pwm);
    
    /**
     * Ferma movimento in corso
     */
    void stopMove();
    
// POSIZIONI PREDEFINITE

    void moveToMin(Adafruit_PWMServoDriver& pwm);
    void moveToMax(Adafruit_PWMServoDriver& pwm);
    void moveToCenter(Adafruit_PWMServoDriver& pwm);
    void moveToSafePosition(Adafruit_PWMServoDriver& pwm, int angle);
    
// GETTERS

    int getCurrentAngle() const;
    int getChannel() const;
    bool isMoving() const;
    bool isAngleSafe(int angle) const;
    String getDebugInfo() const;
    
// SETTERS

    void setSafetyLimits(int min, int max);
    void setSafetyEnabled(bool enabled);
    void setTrim(int trim);

protected:
// VARIABILI PROTETTE (NO DUPLICATI!)

    // Hardware
    int channel;
    
    // Range fisico
    int minAngle;
    int maxAngle;
    uint16_t minPulse;
    uint16_t maxPulse;
    
    // Range sicurezza
    int safeMinAngle;
    int safeMaxAngle;
    bool safetyEnabled;
    
    // Stato attuale
    int currentAngle;
    int trim;
    
    // Movimento smooth NON-BLOCCANTE
    bool moving;
    float moveStartAngle;
    float moveTargetAngle;
    unsigned long moveStartTime;
    uint16_t moveDuration;
    
// UTILITY PROTETTE

    uint16_t angleToPulse(float angle);
    float applySafetyLimits(float angle);
};

#endif