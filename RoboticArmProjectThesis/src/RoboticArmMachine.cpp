#include "RoboticArmMachine.h"

#include "include/set_up.h"


RoboticArmMachine::RoboticArmMachine()
{

    this->currentState = STATE_START;
    this->previousState = STATE_START;
    
    this->networkConnected = false;
    this->servoError = false;
    this->lastErrorMsg = "";
    this->pendingCommand = "";


    pinMode(LED_GREEN, OUTPUT);
    pinMode(LED_RED, OUTPUT);

    setLedState(false, false);
    

    this->baseServo = new ServoMotor20Diy(BASE_SERVO);
    this->elbowServo = new ServoMotorMG66R(SERVO_ELBOW);
    this->wristServo = new ServoMotorMG66R(SERVO_WRIST);
    this->clawServo = new ServoMotorMG66R(SERVO_CLAW);


    this->pwm = Adafruit_PWMServoDriver(PCA9685_ADDRESS);

    Wire.begin(SDA_PIN, SCL_PIN);
    Wire.setClock(400000); // 400kHz Fast I2C

    Serial.printf("📡 I2C: SDA=%d, SCL=%d, Clock=400kHz\n", SDA_PIN, SCL_PIN);

    // Test presenza PCA9685
    Wire.beginTransmission(PCA9685_ADDRESS);
    byte error = Wire.endTransmission();

    if (error != 0)
    {
        Serial.printf("❌ ERRORE: PCA9685 non trovato all'indirizzo 0x%02X\n", PCA9685_ADDRESS);
        Serial.println("⚙️  Verifica collegamenti I2C!");
        Serial.println("   - SDA (D2) collegato?");
        Serial.println("   - SCL (D1) collegato?");
        Serial.println("   - PCA9685 VCC collegato a 3.3V ESP8266?");
        while (1)
        {
        delay(1000);
        Serial.print(".");
        }
    }

    Serial.printf("✅ PCA9685 trovato all'indirizzo 0x%02X\n\n", PCA9685_ADDRESS);

    // ==========================================
    // INIZIALIZZA PCA9685
    // ==========================================
    this->pwm.begin();
    this->pwm.setPWMFreq(50); // 50Hz per servo standard MG996R
    delay(10);
}

void RoboticArmMachine::begin()
{
    
}

void RoboticArmMachine::moveBaseServo(int angle)
{

}

void RoboticArmMachine::moveElbowServo(int angle)
{
    this->elbowServo->moveServo(this->pwm, angle);
}

void RoboticArmMachine::moveWristServo(int angle)
{
    this->wristServo->moveServo(this->pwm, angle);
}

void RoboticArmMachine::moveClawServo(int angle)
{
    this->clawServo->moveServo(this->pwm, angle);
}


void RoboticArmMachine::setLedState(bool green, bool red) {
    digitalWrite(LED_GREEN, green ? HIGH : LOW);
    digitalWrite(LED_RED, red ? HIGH : LOW);

    /* Debug */
    /*String ledState = "LED: ";
    ledState += green ? "ON" : "OFF";
    ledState += " ";
    ledState += red ? "ON" : "OFF";
    Serial.println(ledState);*/
}
