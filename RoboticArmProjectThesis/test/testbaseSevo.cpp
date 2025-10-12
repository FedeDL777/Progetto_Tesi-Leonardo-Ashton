
/*******************************************************************************
 * GUIDA COMPLETA: ESP8266 + PCA9685 + Servo 20kg 270°
 *
 * Hardware:
 * - ESP8266 (NodeMCU, Wemos D1 Mini, etc.)
 * - PCA9685 16-Channel PWM Driver
 * - Servo 20kg digitale 270° (tipo DS3218/RDS3218)
 * - Batteria 7.2V 2400mAh NiMH
 *
 * IMPORTANTE: Problemi risolti in questa guida
 * 1. ESP8266 è 3.3V, PCA9685 tollera 3.3V o 5V
 * 2. Servo richiede 4.8-7.2V (OK con batteria 7.2V)
 * 3. Servo 270° usa pulse width diverso da servo standard 180°
 * 4. Corrente servo: fino a 2.2A @ 6.8V - DEVE essere alimentato separato!
 *
 * Autore: Assistente AI
 * Data: Gennaio 2025
 ******************************************************************************/

// ============================================================================
// LIBRERIE NECESSARIE
// ============================================================================
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

// ============================================================================
// CONFIGURAZIONE HARDWARE
// ============================================================================

// Pin I2C ESP8266 (default)
#define SDA_PIN D2 // GPIO4
#define SCL_PIN D1 // GPIO5

// Indirizzo I2C PCA9685 (default 0x40)
#define PCA9685_ADDRESS 0x40

// Canale servo sul PCA9685
#define SERVO_CHANNEL 0

// ============================================================================
// CONFIGURAZIONE SERVO 270°
// ============================================================================
// IMPORTANTE: I servo 270° hanno pulse width diversi dai 180°!

// Servo standard 180°:     500-2500μs (1ms-2.5ms)
// Servo 270° (DS3218/RDS): 500-2500μs ma con diversa mappatura

// Specifiche tipiche servo 20kg 270°:
// - Tensione: 4.8V-7.2V (ottimale 6-6.8V)
// - Corrente idle: ~5mA
// - Corrente sotto carico: 1.8A @ 6V, 2.2A @ 6.8V
// - Dead band: 3μs
// - Pulse width: 500-2500μs
// - Frequenza PWM: 50Hz (20ms periodo)

// Valori PWM per PCA9685 (12-bit: 0-4095)
// Formula: pulse_length = (microseconds * 4096 * 50Hz) / 1,000,000
#define SERVOMIN 102 // 500μs  → 0°
#define SERVOMAX 512 // 2500μs → 270°
#define SERVOMID 307 // 1500μs → 135° (centro)

// Per calibrazione fine (regolare per ogni servo!)
#define SERVO_TRIM 0 // Offset in step (positivo o negativo)

// ============================================================================
// OGGETTI GLOBALI
// ============================================================================
Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver(PCA9685_ADDRESS);

// ============================================================================
// FUNZIONI UTILITY
// ============================================================================

/**
 * Converte angolo (0-270°) in pulse width per PCA9685
 * @param angle Angolo desiderato (0-270)
 * @return Valore PWM per PCA9685 (SERVOMIN-SERVOMAX)
 */
uint16_t angleToPulse(float angle)
{
  // Limita angolo a range valido
  angle = constrain(angle, 0, 270);

  // Mappa angolo su range PWM
  uint16_t pulse = map(angle * 10, 0, 2700, SERVOMIN, SERVOMAX);

  // Applica trim di calibrazione
  pulse += SERVO_TRIM;

  // Assicura che rimanga nei limiti
  pulse = constrain(pulse, SERVOMIN, SERVOMAX);

  return pulse;
}

/**
 * Muove servo a posizione specificata
 * @param channel Canale PCA9685 (0-15)
 * @param angle Angolo desiderato (0-270)
 */
void moveServo(uint8_t channel, float angle)
{
  uint16_t pulse = angleToPulse(angle);
  pwm.setPWM(channel, 0, pulse);

  Serial.printf("Servo CH%d → %.1f° (PWM: %d)\n", channel, angle, pulse);
}

/**
 * Muove servo lentamente da posizione A a B
 * @param channel Canale PCA9685
 * @param startAngle Angolo iniziale
 * @param endAngle Angolo finale
 * @param duration_ms Durata movimento in millisecondi
 */
void moveServoSmooth(uint8_t channel, float startAngle, float endAngle, uint16_t duration_ms)
{
  float steps = abs(endAngle - startAngle);
  float delay_per_step = duration_ms / steps;

  if (startAngle < endAngle)
  {
    // Movimento crescente
    for (float angle = startAngle; angle <= endAngle; angle += 1.0)
    {
      moveServo(channel, angle);
      delay(delay_per_step);
    }
  }
  else
  {
    // Movimento decrescente
    for (float angle = startAngle; angle >= endAngle; angle -= 1.0)
    {
      moveServo(channel, angle);
      delay(delay_per_step);
    }
  }
}

/**
 * Test completo range servo
 */
void testServoRange()
{
  Serial.println("\n=== TEST RANGE SERVO 270° ===");

  Serial.println("Posizione 0°...");
  moveServo(SERVO_CHANNEL, 0);
  delay(2000);

  Serial.println("Posizione 135° (centro)...");
  moveServo(SERVO_CHANNEL, 135);
  delay(2000);

  Serial.println("Posizione 270°...");
  moveServo(SERVO_CHANNEL, 270);
  delay(2000);

  Serial.println("Ritorno al centro...");
  moveServo(SERVO_CHANNEL, 135);
  delay(1000);

  Serial.println("Test completato!\n");
}

/**
 * Sweep continuo per test
 */
void testSweep()
{
  Serial.println("\n=== TEST SWEEP 0-270° ===");

  // Movimento lento 0 → 270°
  moveServoSmooth(SERVO_CHANNEL, 0, 270, 3000);
  delay(500);

  // Movimento lento 270° → 0
  moveServoSmooth(SERVO_CHANNEL, 270, 0, 3000);
  delay(500);

  Serial.println("Sweep completato!\n");
}

// ============================================================================
// FUNZIONE: Calibrazione interattiva
// ============================================================================
void calibrateServo()
{
  Serial.println("\n=== CALIBRAZIONE SERVO 270° ===");
  Serial.println("Trova i valori PWM corretti per il tuo servo specifico\n");

  uint16_t currentPulse = SERVOMID;

  Serial.println("Comandi:");
  Serial.println("  + / - : Incrementa/decrementa PWM di 10");
  Serial.println("  u / d : Incrementa/decrementa PWM di 1");
  Serial.println("  m : Torna al valore medio");
  Serial.println("  q : Esci dalla calibrazione");
  Serial.println("  s : Salva valore attuale\n");

  pwm.setPWM(SERVO_CHANNEL, 0, currentPulse);
  Serial.printf("PWM attuale: %d\n", currentPulse);

  while (true)
  {
    if (Serial.available())
    {
      char cmd = Serial.read();

      if (cmd == '+')
      {
        currentPulse += 10;
      }
      else if (cmd == '-')
      {
        currentPulse -= 10;
      }
      else if (cmd == 'u')
      {
        currentPulse += 1;
      }
      else if (cmd == 'd')
      {
        currentPulse -= 1;
      }
      else if (cmd == 'm')
      {
        currentPulse = SERVOMID;
      }
      else if (cmd == 's')
      {
        Serial.printf("\n✅ Valore salvato: %d\n", currentPulse);
        Serial.println("Aggiorna #define nel codice!\n");
      }
      else if (cmd == 'q')
      {
        Serial.println("Calibrazione terminata\n");
        break;
      }

      // Limita range
      currentPulse = constrain(currentPulse, 50, 650);

      // Applica e mostra
      pwm.setPWM(SERVO_CHANNEL, 0, currentPulse);
      Serial.printf("PWM: %d\n", currentPulse);
    }

    delay(10);
  }
}



// ============================================================================
// SETUP
// ============================================================================
void setup()
{
  // Inizializza seriale
  Serial.begin(115200);
  delay(1000);

  Serial.println("\n\n");
  Serial.println("=====================================");
  Serial.println("  ESP8266 + PCA9685 + Servo 20kg");
  Serial.println("  Progetto Braccio Robotico");
  Serial.println("=====================================\n");

  // ==========================================
  // INIZIALIZZA I2C
  // ==========================================
  Wire.begin(SDA_PIN, SCL_PIN);
  Wire.setClock(400000); // 400kHz Fast I2C

  Serial.printf("I2C inizializzato su SDA=%d, SCL=%d\n", SDA_PIN, SCL_PIN);

  // Test presenza PCA9685
  Wire.beginTransmission(PCA9685_ADDRESS);
  byte error = Wire.endTransmission();

  if (error != 0)
  {
    Serial.printf("❌ ERRORE: PCA9685 non trovato all'indirizzo 0x%02X\n", PCA9685_ADDRESS);
    Serial.println("Verifica collegamenti I2C!");
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
  pwm.begin();
  pwm.setPWMFreq(50); // 50Hz per servo standard
  delay(10);

  Serial.println("✅ PCA9685 configurato a 50Hz\n");

  // ==========================================
  // INFORMAZIONI ALIMENTAZIONE
  // ==========================================
  Serial.println("⚡ VERIFICA ALIMENTAZIONE:");
  Serial.println("   ESP8266: Alimentato via USB (5V) o 3.3V regolato");
  Serial.println("   PCA9685 VCC: Collegato a ESP8266 3.3V");
  Serial.println("   PCA9685 V+: Collegato a batteria 7.2V");
  Serial.println("   Servo: Alimentato da PCA9685 V+ (7.2V)\n");

  Serial.println("⚠️  ATTENZIONE CORRENTE:");
  Serial.println("   Servo 20kg può assorbire fino a 2.2A!");
  Serial.println("   Verifica che la batteria supporti questo carico.\n");

  // ==========================================
  // POSIZIONE INIZIALE SICURA
  // ==========================================
  Serial.println("🔧 Movimento a posizione iniziale (135°)...");
  moveServo(SERVO_CHANNEL, 135);
  delay(1000);

  Serial.println("\n✅ Setup completato!");
  Serial.println("=====================================\n");

  // ==========================================
  // MENU INTERATTIVO
  // ==========================================
  Serial.println("COMANDI DISPONIBILI:");
  Serial.println("  t - Test range completo (0-135-270°)");
  Serial.println("  s - Sweep continuo");
  Serial.println("  0-9 - Posizioni preset");
  Serial.println("  a XXX - Vai ad angolo XXX (es: 'a 180')");
  Serial.println("  c - Calibrazione fine\n");
}

// ============================================================================
// LOOP PRINCIPALE
// ============================================================================
void loop()
{
  if (Serial.available())
  {
    String command = Serial.readStringUntil('\n');
    command.trim();

    if (command == "t")
    {
      // Test range
      testServoRange();
    }
    else if (command == "s")
    {
      // Sweep
      testSweep();
    }
    else if (command == "c")
    {
      // Calibrazione
      calibrateServo();
    }
    else if (command.startsWith("a "))
    {
      // Comando angolo custom: "a 180"
      float angle = command.substring(2).toFloat();
      if (angle >= 0 && angle <= 270)
      {
        Serial.printf("Movimento a %.1f°\n", angle);
        moveServo(SERVO_CHANNEL, angle);
      }
      else
      {
        Serial.println("❌ Angolo fuori range (0-270)");
      }
    }
    else if (command.length() == 1 && isDigit(command[0]))
    {
      // Posizioni preset 0-9
      int preset = command.toInt();
      float angle = preset * 30; // 0=0°, 1=30°, ..., 9=270°

      Serial.printf("Preset %d → %.0f°\n", preset, angle);
      moveServo(SERVO_CHANNEL, angle);
    }
    else
    {
      Serial.println("❌ Comando non riconosciuto");
      Serial.println("Usa: t, s, c, 0-9, o 'a XXX'");
    }
  }

  delay(10);
}
