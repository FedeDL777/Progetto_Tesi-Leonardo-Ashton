/*******************************************************************************
 * SERVO MOTOR 20KG DIY - CLASSE DERIVATA
 *
 * Servo ad altissima coppia 20kg/cm
 * Range: 0-270°
 * Tensione: 6-7.2V (con regolatore)
 * Velocità: 0.13s/60°
 ******************************************************************************/

#ifndef __SERVO_20_DIY__
#define __SERVO_20_DIY__

#include "ServoBase.h"

/**
 * Servo 20kg 270° specializzato
 * Eredita da ServoMotor
 */
class ServoMotor20Diy : public ServoMotor
{

public:
    /**
     * Costruttore con range personalizzabile
     *
     * Esempio uso:
     * - ServoMotor20Diy base(0);                     // Full range 0-270°
     * - ServoMotor20Diy base(0, -1, -1, 45, 225);    // Safe range 45-225°
     * - ServoMotor20Diy base(0, -1, -1, 90, 180);    // Limited 90-180°
     *
     * @param channel  Canale PCA9685 (0-15)
     * @param safeMin  Limite minimo di sicurezza (default -1 = no limit)
     * @param safeMax  Limite massimo di sicurezza (default -1 = no limit)
     */
    ServoMotor20Diy(
        int channel,
        int safeMin = -1,
        int safeMax = -1);

    /**
     * Override per aggiungere metodi specifici servo 270°
     */
    void moveOneFullRotation(Adafruit_PWMServoDriver pwm, uint16_t duration = 2000);
};

#endif