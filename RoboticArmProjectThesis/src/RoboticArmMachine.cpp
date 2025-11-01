/*******************************************************************************
 * ROBOTIC ARM MACHINE - IMPLEMENTAZIONE
 ******************************************************************************/

#include "RoboticArmMachine.h"
#include "include/set_up.h"

// ============================================================================
// COSTRUTTORE
// ============================================================================

RoboticArmMachine::RoboticArmMachine()
{

    // Setup I2C
    Wire.begin(SDA_PIN, SCL_PIN);
    Wire.setClock(400000);

    Serial.printf("I2C: SDA=%d, SCL=%d, Clock=400kHz\n", SDA_PIN, SCL_PIN);

    // Test PCA9685
    Wire.beginTransmission(PCA9685_ADDRESS);
    byte error = Wire.endTransmission();

    if (error != 0)
    {
        Serial.printf("PCA9685 not found at 0x%02X\n", PCA9685_ADDRESS);
        while (1)
            delay(1000);
    }

    Serial.printf("PCA9685 found\n\n");

    this->pwm = Adafruit_PWMServoDriver(PCA9685_ADDRESS);
    this->pwm.begin();
    this->pwm.setPWMFreq(50);
    delay(10);

    Serial.println("PCA9685 configured\n");

    this->ledGreen = new Led(LED_GREEN);
    this->ledRed = new Led(LED_RED);
    this->buttonWhite = new Button(BUTTON_WHITE_PIN);
    this->buttonBlue = new Button(BUTTON_BLUE_PIN);

    Serial.println("LED and Button initialized\n");

    // ==========================================
    // Crea servo motori
    // ==========================================
    // Full range servo
    this->baseServo = new ServoMotor20Diy(BASE_SERVO, 0, MAX_RANGE);
    this->elbowServo = new ServoMotor20Diy(SERVO_ELBOW, 0, MAX_RANGE_ELBOW);

    // Limited range servo (per sicurezza)
    this->wristServo = new ServoMotorMG66R(SERVO_WRIST, 0, MAX_RANGE);
    this->clawServo = new ServoMotorMG66R(SERVO_CLAW, SAFE_MIN_RANGE_CLAW, SAFE_MAX_RANGE_CLAW);

    Serial.println("Servo motors initialized\n");

    // ==========================================
    // Inizializza state
    // ==========================================
    this->currentState = STATE_START;
    this->previousState = STATE_START;
    this->networkConnected = false;
    this->servoErrorFlag = false;
    this->lastErrorMsg = "";
    this->pendingCommand = "";

    this->stateEntryTime = millis();
    this->lastNetworkCheck = millis();
    this->lastServoCheck = millis();

    Serial.println("✅ RoboticArmMachine initialized\n");
}

void RoboticArmMachine::begin()
{
    Serial.println("System starting...\n");

    setLedState(false, false);

    // Posizione sicura iniziale
    moveAllToCenter();
    delay(500);

    // Transizione a START
    transitionTo(STATE_START);
}

void RoboticArmMachine::update()
{
    // Esegui handler dello stato attuale
    switch (currentState)
    {
    case STATE_START:
        handleStart();
        break;
    case STATE_CONNECTED:
        handleConnected();
        break;
    case STATE_WORKING:
        handleWorking();
        break;
    case STATE_PROBLEM_SERVO:
        handleProblemServo();
        break;
    case STATE_NETWORK_LOST:
        handleNetworkLost();
        break;
    case STATE_IDLE:
        handleIdle();
        break;
    }
}

// ============================================================================
// GETTERS
// ============================================================================

int RoboticArmMachine::getCurrentState() const
{
    return currentState;
}

String RoboticArmMachine::getStateString() const
{
    switch (currentState)
    {
    case STATE_START:
        return "START";
    case STATE_CONNECTED:
        return "CONNECTED";
    case STATE_WORKING:
        return "WORKING";
    case STATE_PROBLEM_SERVO:
        return "PROBLEM_SERVO";
    case STATE_NETWORK_LOST:
        return "NETWORK_LOST";
    case STATE_IDLE:
        return "IDLE";
    default:
        return "UNKNOWN";
    }
}

int RoboticArmMachine::getBaseAngle() const
{
    return baseServo->getCurrentAngle();
}

int RoboticArmMachine::getElbowAngle() const
{
    return elbowServo->getCurrentAngle();
}

int RoboticArmMachine::getWristAngle() const
{
    return wristServo->getCurrentAngle();
}

int RoboticArmMachine::getClawAngle() const
{
    return clawServo->getCurrentAngle();
}

bool RoboticArmMachine::isButtonWhitePressed()
{
    buttonWhite->sync();
    return buttonWhite->isPressed();
}

bool RoboticArmMachine::isButtonBluePressed()
{
    buttonBlue->sync();
    return buttonBlue->isPressed();
}

// QUEUE COMANDI

bool RoboticArmMachine::pushCommand(const String& newCmd) {

    
    if (newCmd.length() == 0) {
        Serial.println("Comando non valido: " + newCmd);
        return false;
    }

    commandQueue.push(newCmd);
    numCommands++;

    return true;
}

String RoboticArmMachine::popCommand() {
    if (commandQueue.empty()) {
        return "";  // Comando vuoto
    }

    String cmd = commandQueue.front();
    commandQueue.pop();
    numCommands--;

    Serial.printf("Comando estratto: %s (coda: %d)\n",
        cmd.c_str(), numCommands);
    
    return cmd;
}

bool RoboticArmMachine::hasCommands() const {
    return !commandQueue.empty();
}

int RoboticArmMachine::getCommandCount() const {
    return numCommands;
}

void RoboticArmMachine::clearCommands() {
    while (!commandQueue.empty()) {
        commandQueue.pop();
    }
    numCommands = 0;
    Serial.println("Coda comandi svuotata");
}

// ============================================================================
// COMMAND EXECUTION
// ============================================================================

bool RoboticArmMachine::executeCommand(const String& cmd) {
    int angle = DEFAULT_ANGLE_MOVE;

    if (cmd == "Base SX") {
        int currentAngle = baseServo->getCurrentAngle();
        moveBaseServo(currentAngle - angle);
        return true;
    }
    else if (cmd == "Base DX") {
        int currentAngle = baseServo->getCurrentAngle();
        moveBaseServo(currentAngle + angle);
        return true;
    }
    else if (cmd == "Elbow SX") {
        int currentAngle = elbowServo->getCurrentAngle();
        moveElbowServo(currentAngle - angle);
        return true;
    }
    else if (cmd == "Elbow DX") {
        int currentAngle = elbowServo->getCurrentAngle();
        moveElbowServo(currentAngle + angle);
        return true;
    }
    else if (cmd == "Wrist SX") {
        int currentAngle = wristServo->getCurrentAngle();
        moveWristServo(currentAngle - angle);
        return true;
    }
    else if (cmd == "Wrist DX") {
        int currentAngle = wristServo->getCurrentAngle();
        moveWristServo(currentAngle + angle);
        return true;
    }
    else if (cmd == "Claw Open") {
        int currentAngle = clawServo->getCurrentAngle();
        moveClawServo(currentAngle + angle);
        return true;
    }
    else if (cmd == "Claw Close") {
        int currentAngle = clawServo->getCurrentAngle();
        moveClawServo(currentAngle - angle);
        return true;
    }

    Serial.println("❌ Comando non riconosciuto");
    return false;
}


// TRANSIZIONI PUBBLICHE
// ============================================================================

void RoboticArmMachine::tryConnectToNetwork()
{
    if (currentState == STATE_START)
    {
        Serial.println("Attempting network connection...");
        // TODO: Implementare connessione WiFi/Bluetooth
    }
}

void RoboticArmMachine::connectionEstablished()
{
    if (currentState == STATE_START)
    {
        networkConnected = true;
        Serial.println("Connection established!");
        transitionTo(STATE_CONNECTED);
    }
}

void RoboticArmMachine::connectionLost()
{
    Serial.println("Connection lost!");
    if (currentState != STATE_START && currentState != STATE_NETWORK_LOST)
    {
        networkConnected = false;
        transitionTo(STATE_NETWORK_LOST);
    }
}

void RoboticArmMachine::startWorking()
{
    if (currentState == STATE_CONNECTED || currentState == STATE_IDLE)
    {
        Serial.println("Starting operations...");
        transitionTo(STATE_WORKING);
    }
}

void RoboticArmMachine::stopWorking()
{
    if (currentState == STATE_WORKING)
    {
        Serial.println("⏹️  Stopping operations");
        transitionTo(STATE_IDLE);
    }
}

void RoboticArmMachine::servoError(String errorMsg)
{
    Serial.printf("SERVO ERROR: %s\n", errorMsg.c_str());
    this->lastErrorMsg = errorMsg;
    this->servoErrorFlag = true;
    transitionTo(STATE_PROBLEM_SERVO);
}

void RoboticArmMachine::servoErrorResolved()
{
    Serial.println("Servo error resolved");
    this->servoErrorFlag = false;
    this->lastErrorMsg = "";
    transitionTo(STATE_CONNECTED);
}

void RoboticArmMachine::receiveCommand(String command)
{
    if (currentState == STATE_CONNECTED || currentState == STATE_IDLE)
    {
        this->pendingCommand = command;
        startWorking();
    }
}

// ============================================================================
// MOVIMENTO SERVO
// ============================================================================

void RoboticArmMachine::moveBaseServo(int angle)
{
    baseServo->startSmoothMove(pwm, angle, 500);
}

void RoboticArmMachine::moveElbowServo(int angle)
{
    elbowServo->startSmoothMove(pwm, angle, 500);
}

void RoboticArmMachine::moveWristServo(int angle)
{
    wristServo->startSmoothMove(pwm, angle, 500);
}

void RoboticArmMachine::moveClawServo(int angle)
{
    clawServo->startSmoothMove(pwm, angle, 500);
}

void RoboticArmMachine::moveAllToSafePosition()
{
    Serial.println("Moving all servos to SAFE position...");
    baseServo->moveToSafePosition(pwm, SAFE_RANGE_DEFAULT);
    elbowServo->moveToSafePosition(pwm, SAFE_MIN_RANGE_ELBOW);
    wristServo->moveToSafePosition(pwm, SAFE_RANGE_DEFAULT);
    clawServo->moveToSafePosition(pwm, SAFE_RANGE_DEFAULT);
}

void RoboticArmMachine::moveAllToCenter()
{
    Serial.println("Moving all servos to CENTER...");
    baseServo->moveToCenter(pwm);
    elbowServo->moveToCenter(pwm);
    wristServo->moveToCenter(pwm);
    clawServo->moveToCenter(pwm);
}

bool RoboticArmMachine::areAllAngleSafe()
{
    return baseServo->isAngleSafe(this->baseServo->getCurrentAngle()) &&
           elbowServo->isAngleSafe(this->elbowServo->getCurrentAngle()) &&
           wristServo->isAngleSafe(this->wristServo->getCurrentAngle()) &&
           clawServo->isAngleSafe(this->clawServo->getCurrentAngle());
}

void RoboticArmMachine::setSafetyEnabled(bool enabled)
{
    baseServo->setSafetyEnabled(enabled);
    elbowServo->setSafetyEnabled(enabled);
    wristServo->setSafetyEnabled(enabled);
    clawServo->setSafetyEnabled(enabled);
}

// ============================================================================
// STATO: START
// ============================================================================

void RoboticArmMachine::enterStart()
{
    setLedState(false, false);
    stateEntryTime = millis();
}

void RoboticArmMachine::handleStart()
{
    unsigned long elapsed = millis() - stateEntryTime;

    // Simula connessione dopo 2 secondi
    if (elapsed > 2000)
    {
        connectionEstablished();
    }
}

void RoboticArmMachine::exitStart()
{
    Serial.println("Exit START\n");
}

// ============================================================================
// STATO: CONNECTED
// ============================================================================

void RoboticArmMachine::enterConnected()
{
    setLedState(true, false);
    stateEntryTime = millis();
    Serial.println("System ready for commands\n");
}

void RoboticArmMachine::handleConnected()
{
    // Monitoraggio rete
    if (millis() - lastNetworkCheck > NETWORK_CHECK_INTERVAL)
    {
        checkNetwork();
        lastNetworkCheck = millis();
    }
}

void RoboticArmMachine::exitConnected()
{
    Serial.println("Exit CONNECTED\n");
}

// ============================================================================
// STATO: WORKING
// ============================================================================

void RoboticArmMachine::enterWorking()
{
    setLedState(true, true); // Entrambi LED accesi
    stateEntryTime = millis();

    // Processa comando in sospeso
    if (pendingCommand.length() > 0)
    {
        processCommand(pendingCommand);
        pendingCommand = "";
    }
}

void RoboticArmMachine::handleWorking()
{
    // Controlla salute servo
    if (millis() - lastServoCheck > SERVO_CHECK_INTERVAL)
    {
        checkServoHealth();
        lastServoCheck = millis();
    }

    // Timeout dopo 30 secondi
    if (millis() - stateEntryTime > 30000)
    {
        stopWorking();
    }
}

void RoboticArmMachine::exitWorking()
{
    Serial.println("Exit WORKING\n");
}

// ============================================================================
// STATO: PROBLEM_SERVO
// ============================================================================

void RoboticArmMachine::enterProblemServo()
{

    setLedState(false, true); // Solo LED rosso
    Serial.printf(" Error: %s\n\n", lastErrorMsg.c_str());

    bringToSafePosition();
    stateEntryTime = millis();
}

void RoboticArmMachine::handleProblemServo()
{
    // Auto-recovery dopo 3 secondi
    if (millis() - stateEntryTime > ERROR_RECOVERY_TIMEOUT)
    {
        servoErrorResolved();
    }
}

void RoboticArmMachine::exitProblemServo()
{
    Serial.println("✅ Exit PROBLEM_SERVO\n");
}

// ============================================================================
// STATO: NETWORK_LOST
// ============================================================================

void RoboticArmMachine::enterNetworkLost()
{

    setLedState(false, true); // Solo LED rosso
    bringToSafePosition();
    stateEntryTime = millis();
}

void RoboticArmMachine::handleNetworkLost()
{
    // Tenta riconnessione
    if (millis() - lastNetworkCheck > 2000)
    {
        Serial.println("🔄 Attempting reconnection...");
        // TODO: Implementare logica riconnessione
        lastNetworkCheck = millis();
    }
}

void RoboticArmMachine::exitNetworkLost()
{
    Serial.println("✅ Exit NETWORK_LOST\n");
}

// ============================================================================
// STATO: IDLE
// ============================================================================

void RoboticArmMachine::enterIdle()
{
    setLedState(true, false); // Solo LED verde
    stateEntryTime = millis();
    Serial.println("💤 System in standby\n");
}

void RoboticArmMachine::handleIdle()
{
    // Monitoraggio periodico
    if (millis() - lastNetworkCheck > NETWORK_CHECK_INTERVAL)
    {
        checkNetwork();
        lastNetworkCheck = millis();
    }
}

void RoboticArmMachine::exitIdle()
{
    Serial.println("✅ Exit IDLE\n");
}

// ============================================================================
// UTILITY PRIVATE
// ============================================================================

void RoboticArmMachine::transitionTo(int newState)
{
    if (newState == currentState)
        return;

    logStateChange(currentState, newState);
    previousState = currentState;
    currentState = newState;

    // Exit precedente
    switch (previousState)
    {
    case STATE_START:
        exitStart();
        break;
    case STATE_CONNECTED:
        exitConnected();
        break;
    case STATE_WORKING:
        exitWorking();
        break;
    case STATE_PROBLEM_SERVO:
        exitProblemServo();
        break;
    case STATE_NETWORK_LOST:
        exitNetworkLost();
        break;
    case STATE_IDLE:
        exitIdle();
        break;
    }

    // Enter nuovo
    switch (newState)
    {
    case STATE_START:
        enterStart();
        break;
    case STATE_CONNECTED:
        enterConnected();
        break;
    case STATE_WORKING:
        enterWorking();
        break;
    case STATE_PROBLEM_SERVO:
        enterProblemServo();
        break;
    case STATE_NETWORK_LOST:
        enterNetworkLost();
        break;
    case STATE_IDLE:
        enterIdle();
        break;
    }
}

void RoboticArmMachine::setLedState(bool green, bool red)
{
    if (green)
        ledGreen->switchOn();
    else
        ledGreen->switchOff();

    if (red)
        ledRed->switchOn();
    else
        ledRed->switchOff();
}

void RoboticArmMachine::logStateChange(int oldState, int newState)
{
    String oldStr = (oldState == STATE_START) ? "START" : (oldState == STATE_CONNECTED)   ? "CONNECTED"
                                                      : (oldState == STATE_WORKING)       ? "WORKING"
                                                      : (oldState == STATE_PROBLEM_SERVO) ? "PROBLEM_SERVO"
                                                      : (oldState == STATE_NETWORK_LOST)  ? "NETWORK_LOST"
                                                                                          : "IDLE";

    String newStr = (newState == STATE_START) ? "START" : (newState == STATE_CONNECTED)   ? "CONNECTED"
                                                      : (newState == STATE_WORKING)       ? "WORKING"
                                                      : (newState == STATE_PROBLEM_SERVO) ? "PROBLEM_SERVO"
                                                      : (newState == STATE_NETWORK_LOST)  ? "NETWORK_LOST"
                                                                                          : "IDLE";

    Serial.printf("📊 STATE: %s → %s\n\n", oldStr.c_str(), newStr.c_str());
}

void RoboticArmMachine::checkNetwork()
{
    // TODO: Implementare verifica connessione reale
    networkConnected = true;
}

void RoboticArmMachine::checkServoHealth()
{
    // TODO: Implementare controllo salute servo
    // - Leggere feedback
    // - Verificare corrente
}

void RoboticArmMachine::processCommand(String command)
{
    Serial.printf("📨 Processing command: %s\n\n", command.c_str());

    if (command.startsWith("BASE:"))
    {
        int angle = command.substring(5).toInt();
        moveBaseServo(angle);
    }
    else if (command.startsWith("ELBOW:"))
    {
        int angle = command.substring(6).toInt();
        moveElbowServo(angle);
    }
    else if (command.startsWith("WRIST:"))
    {
        int angle = command.substring(6).toInt();
        moveWristServo(angle);
    }
    else if (command.startsWith("CLAW:"))
    {
        int angle = command.substring(5).toInt();
        moveClawServo(angle);
    }
    else if (command == "SAFE")
    {
        moveAllToSafePosition();
    }
    else if (command == "CENTER")
    {
        moveAllToCenter();
    }
}

void RoboticArmMachine::bringToSafePosition()
{
    moveAllToSafePosition();
}

String RoboticArmMachine::getDebugInfo() const
{
    String info = "\n╔════════════════════════════════════╗\n";
    info += "║  DEBUG INFO                        ║\n";
    info += "╚════════════════════════════════════╝\n\n";

    info += "State: " + getStateString() + "\n";
    info += "Base: " + String(baseServo->getCurrentAngle()) + "°\n";
    info += "Elbow: " + String(elbowServo->getCurrentAngle()) + "°\n";
    info += "Wrist: " + String(wristServo->getCurrentAngle()) + "°\n";
    info += "Claw: " + String(clawServo->getCurrentAngle()) + "°\n";
    info += "Network: " + String(networkConnected ? "Connected" : "Disconnected") + "\n";
    info += "Error: " + String(servoErrorFlag ? lastErrorMsg : "None") + "\n\n";

    return info;
}