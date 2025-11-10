/*******************************************************************************
 * SERVO MOTOR MG66R - CLASSE DERIVATA
 * 
 * Servo ad alta coppia 11kg/cm
 * Range: 0-180°
 * Tensione: 4.8-6.6V
 * Velocità: 0.19s/60°
 ******************************************************************************/

#ifndef __SERVO_MOTOR_MG66R__
#define __SERVO_MOTOR_MG66R__

#include "ServoBase.h"

/**
 * Servo MG66R specializzato
 * Eredita da ServoMotor
 */
class ServoMotorMG66R : public ServoMotor {

public:
    /**
     * Costruttore con range personalizzabile
     * @param channel  Canale PCA9685 (0-15)
     * @param safeMin  Limite minimo di sicurezza (default -1 = no limit)
     * @param safeMax  Limite massimo di sicurezza (default -1 = no limit)
     */
    ServoMotorMG66R(
        int channel,
        int safeMin = -1,
        int safeMax = -1
    );
    
    /**
     * Override per aggiungere metodi specifici MG66R
     */
    void calibrate();
};

#endif
