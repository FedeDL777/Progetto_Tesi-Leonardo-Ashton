#include "RoboticArmMachine.h"

#include "include/set_up.h"


RoboticArmMachine::RoboticArmMachine()
{


    this->pwm = Adafruit_PWMServoDriver(PCA9685_ADDRESS);

    Wire.begin(SDA_PIN, SCL_PIN);
    Wire.setClock(400000); // 400kHz Fast I2C

    Serial.printf("I2C: SDA=%d, SCL=%d, Clock=400kHz\n", SDA_PIN, SCL_PIN);

    // Test presenza PCA9685
    Wire.beginTransmission(PCA9685_ADDRESS);
    byte error = Wire.endTransmission();

    if (error != 0)
    {
        Serial.printf("ERRORE: PCA9685 non trovato all'indirizzo 0x%02X\n", PCA9685_ADDRESS);
        Serial.println("Verifica collegamenti I2C!");
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
    this->elbowServo = new ServoMotor20Diy(SERVO_ELBOW);
    this->wristServo = new ServoMotorMG66R(SERVO_WRIST);
    this->clawServo = new ServoMotorMG66R(SERVO_CLAW);
}

//


void RoboticArmMachine::transitionTo(int newState) {
    if (newState == currentState) {
        return;  // Nessuna transizione se lo stato è lo stesso
    }
    
    // Exit dello stato attuale
        switch (currentState) {
        case STATE_START:        exitStart();        break;
        case STATE_CONNECTED:    exitConnected();    break;
        case STATE_WORKING:      exitWorking();     break;
        case STATE_PROBLEM_SERVO: exitProblemServo(); break;
        case STATE_NETWORK_LOST: exitNetworkLost();  break;
        case STATE_IDLE:         exitIdle();         break;
    }
    
    // Log transizione
    logStateChange(currentState, newState);
    previousState = currentState;
    
    // Enter del nuovo stato
    switch (newState) {
        case STATE_START:        enterStart();        break;
        case STATE_CONNECTED:    enterConnected();    break;
        case STATE_WORKING:      enterWorking();      break;
        case STATE_PROBLEM_SERVO: enterProblemServo(); break;
        case STATE_NETWORK_LOST: enterNetworkLost();  break;
        case STATE_IDLE:         enterIdle();         break;
    }
}

void RoboticArmMachine::logStateChange(int oldState, int newState)
{
    Serial.printf("🔄 Cambio stato: %d -> %d\n", oldState, newState);
}

void RoboticArmMachine::enterStart()
{
}
void RoboticArmMachine::exitStart()
{
}
void RoboticArmMachine::enterConnected()
{
    setLedState(true, false); // LED verde acceso
    this->networkConnected = true;
}
void RoboticArmMachine::exitConnected()
{
}
void RoboticArmMachine::enterWorking()
{
    setLedState(true, true); // LED verde e rosso acceso
    // Esegui operazioni di lavoro

}
void RoboticArmMachine::exitWorking()
{
    setLedState(true, false); // LED verde acceso
}

void RoboticArmMachine::enterProblemServo()
{
    setLedState(false, true); // LED rosso acceso
}
void RoboticArmMachine::exitProblemServo()
{
    setLedState(false, false); 
}

void RoboticArmMachine::enterNetworkLost()
{
    setLedState(false, true); // LED rosso acceso
    this->networkConnected = false;
}

void RoboticArmMachine::exitNetworkLost()
{
    setLedState(false, false); 
}
void RoboticArmMachine::enterIdle()
{
    setLedState(true, false); // LED verde acceso
}
void RoboticArmMachine::exitIdle()
{
    setLedState(false, false); 
}



int RoboticArmMachine::getCurrentState()
{
    return this->currentState;
}

void RoboticArmMachine::tryConnectToNetwork()
{
    if (currentState == STATE_START) {
        Serial.println("🔌 Tentativo connessione rete in corso...");
        // Qui implementerai la logica di connessione WiFi/Bluetooth
    }
}

void RoboticArmMachine::connectionEstablished() {
    if (currentState == STATE_START) {
        Serial.println("✅ Connessione stabilita!");
        transitionTo(STATE_CONNECTED);
    }
}

void RoboticArmMachine::connectionLost() {
    Serial.println("❌ Connessione persa!");
    if (currentState != STATE_START && currentState != STATE_NETWORK_LOST) {
        transitionTo(STATE_NETWORK_LOST);
    }
}

void RoboticArmMachine::startWorking() {
    if (currentState == STATE_CONNECTED || currentState == STATE_IDLE) {
        Serial.println("🚀 Inizio operazioni...");
        transitionTo(STATE_WORKING);
    }
}

void RoboticArmMachine::stopWorking() {
    if (currentState == STATE_WORKING) {
        Serial.println("⏹️  Stop operazioni");
        transitionTo(STATE_IDLE);
    }
}

void RoboticArmMachine::servoError(String errorMsg)
{
}

void RoboticArmMachine::servoErrorResolved() {
    Serial.println("✅ Errore servo risolto");
    this->servoError = false;
    this->lastErrorMsg = "";
    transitionTo(STATE_CONNECTED);
}

void RoboticArmMachine::receiveCommand(String command) {
    if (currentState == STATE_CONNECTED || currentState == STATE_IDLE) {
        this->pendingCommand = command;
        startWorking();
    }
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
