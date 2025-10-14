/*******************************************************************************
 * GUIDA COMPLETA: ESP8266 + PCA9685 + Servo MG996R
 *
 * Hardware:
 * - ESP8266 (NodeMCU, Wemos D1 Mini, etc.)
 * - PCA9685 16-Channel PWM Driver
 * - Servo MG996R (servo standard ad alta coppia)
 * - Batteria 7.2V 2400mAh NiMH (o 6V per test meno aggressivo)
 *
 * DIFFERENZE MG996R vs Servo 270°:
 * 1. Angolo: 0-180° (non 270°!)
 * 2. Tensione: 4.8-6.6V (MASSIMO 6.6V, non 7.2V!)
 * 3. Corrente: 1.2-2.5A sotto carico
 * 4. Pulse width: 500-2500μs (standard)
 * 5. Ingranaggi: Metallo puro (più robusto)
 *
 * ⚠️  ATTENZIONE BATTERIA 7.2V:
 * Il MG996R è rated per MAX 6.6V!
 * Con 7.2V rischi di danneggiare i circuiti interni del servo!
 * OPZIONI:
 * A) Usa regolatore 6.5V prima del servo (CONSIGLIATO)
 * B) Limita carica batteria a 6V max
 * C) Usa batteria 6V (4xAA NiMH)
 *
 * Autore: Assistente AI
 * Data: Gennaio 2025
 ******************************************************************************/

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
// CONFIGURAZIONE SERVO MG996R (180°)
// ============================================================================

/*
 * SPECIFICHE MG996R:
 * - Coppia: 11kg/cm @ 6V, 9.4kg/cm @ 4.8V
 * - Velocità: 0.19s/60° @ 6V
 * - Tensione operativa: 4.8V - 6.6V (MAX!)
 * - Corrente idle: ~5mA
 * - Corrente no load: ~200mA @ 6V
 * - Corrente sotto carico: 1.2A @ 4.8V, 2.5A @ 6.6V
 * - Angolo di controllo: 180°
 * - Pulse width: 500-2500μs (standard)
 * - Frequenza: 50Hz
 * - Dead band: 5μs
 */

// Valori PWM per PCA9685 (12-bit: 0-4095)
// Formula: pulse_length = (microseconds * 4096 * 50Hz) / 1,000,000
#define SERVOMIN 102 // 500μs  → 0°
#define SERVOMAX 512 // 2500μs → 180°
#define SERVOMID 307 // 1500μs → 90° (centro)

// Per calibrazione fine (regolare per ogni servo!)
#define SERVO_TRIM 0 // Offset in step (positivo o negativo)

// ============================================================================
// OGGETTI GLOBALI
// ============================================================================
Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver(PCA9685_ADDRESS);

// ============================================================================
// VARIABILI STATO
// ============================================================================
uint16_t currentAngle = 90; // Posizione attuale
bool isMoving = false;

// ============================================================================
// FUNZIONI UTILITY
// ============================================================================

/**
 * Converte angolo (0-180°) in pulse width per PCA9685
 * @param angle Angolo desiderato (0-180)
 * @return Valore PWM per PCA9685 (SERVOMIN-SERVOMAX)
 */
uint16_t angleToPulse(float angle)
{
  // Limita angolo a range valido (MG996R è 180°, non 270°!)
  angle = constrain(angle, 0, 180);

  // Mappa angolo su range PWM
  uint16_t pulse = map(angle * 10, 0, 1800, SERVOMIN, SERVOMAX);

  // Applica trim di calibrazione
  pulse += SERVO_TRIM;

  // Assicura che rimanga nei limiti
  pulse = constrain(pulse, SERVOMIN, SERVOMAX);

  return pulse;
}

/**
 * Muove servo a posizione specificata
 * @param channel Canale PCA9685 (0-15)
 * @param angle Angolo desiderato (0-180)
 */
void moveServo(uint8_t channel, float angle)
{
  uint16_t pulse = angleToPulse(angle);
  pwm.setPWM(channel, 0, pulse);

  currentAngle = angle;

  Serial.printf("🎯 Servo CH%d → %.1f° (PWM: %d)\n", channel, angle, pulse);
}

/**
 * Muove servo lentamente da posizione A a B
 * @param channel Canale PCA9685
 * @param startAngle Angolo iniziale
 * @param endAngle Angolo finale
 * @param duration_ms Durata movimento in millisecondi
 * @param steps Numero di step intermedi (default 50)
 */
void moveServoSmooth(uint8_t channel, float startAngle, float endAngle,
                     uint16_t duration_ms, uint16_t steps = 50)
{
  isMoving = true;

  float angleDiff = endAngle - startAngle;
  float delay_per_step = (float)duration_ms / steps;

  Serial.printf("🔄 Movimento fluido: %.0f° → %.0f° in %dms\n",
                startAngle, endAngle, duration_ms);

  for (int i = 0; i <= steps; i++)
  {
    float currentAngle = startAngle + (angleDiff * i / steps);
    moveServo(channel, currentAngle);
    delay(delay_per_step);
  }

  isMoving = false;
  Serial.println("✅ Movimento completato\n");
}

/**
 * Test completo range servo MG996R (0-180°)
 */
void testServoRange()
{
  Serial.println("\n╔════════════════════════════════════╗");
  Serial.println("║  TEST RANGE MG996R (0° - 180°)    ║");
  Serial.println("╚════════════════════════════════════╝\n");

  Serial.println("📍 Posizione 0° (completamente a sinistra)...");
  moveServo(SERVO_CHANNEL, 0);
  delay(2500);

  Serial.println("📍 Posizione 45°...");
  moveServo(SERVO_CHANNEL, 45);
  delay(2000);

  Serial.println("📍 Posizione 90° (centro)...");
  moveServo(SERVO_CHANNEL, 90);
  delay(2000);

  Serial.println("📍 Posizione 135°...");
  moveServo(SERVO_CHANNEL, 135);
  delay(2000);

  Serial.println("📍 Posizione 180° (completamente a destra)...");
  moveServo(SERVO_CHANNEL, 180);
  delay(2500);

  Serial.println("📍 Ritorno al centro (90°)...");
  moveServo(SERVO_CHANNEL, 90);
  delay(1000);

  Serial.println("✅ Test range completato!\n");
}

/**
 * Sweep continuo per test rapido
 */
void testSweep()
{
  Serial.println("\n╔════════════════════════════════════╗");
  Serial.println("║  TEST SWEEP RAPIDO                ║");
  Serial.println("╚════════════════════════════════════╝\n");

  // Movimento veloce 0 → 180°
  Serial.println("🔄 Sweep: 0° → 180° (1 secondo)");
  moveServoSmooth(SERVO_CHANNEL, 0, 180, 1000, 30);
  delay(500);

  // Movimento veloce 180° → 0
  Serial.println("🔄 Sweep: 180° → 0° (1 secondo)");
  moveServoSmooth(SERVO_CHANNEL, 180, 0, 1000, 30);
  delay(500);

  // Centro
  moveServo(SERVO_CHANNEL, 90);

  Serial.println("✅ Sweep completato!\n");
}

/**
 * Test smoothness (movimento molto lento)
 */
void testSmoothMovement()
{
  Serial.println("\n╔════════════════════════════════════╗");
  Serial.println("║  TEST MOVIMENTO FLUIDO            ║");
  Serial.println("╚════════════════════════════════════╝\n");

  Serial.println("🐢 Movimento lento: 0° → 180° (5 secondi)");
  moveServoSmooth(SERVO_CHANNEL, 0, 180, 5000, 100);
  delay(1000);

  Serial.println("🐢 Movimento lento: 180° → 0° (5 secondi)");
  moveServoSmooth(SERVO_CHANNEL, 180, 0, 5000, 100);
  delay(500);

  moveServo(SERVO_CHANNEL, 90);
}

/**
 * Test di ripetibilità (torna sempre esattamente alla stessa posizione)
 */
void testRepeatability()
{
  Serial.println("\n╔════════════════════════════════════╗");
  Serial.println("║  TEST RIPETIBILITÀ                ║");
  Serial.println("╚════════════════════════════════════╝\n");

  Serial.println("Test: posizione 90° ripetuta 10 volte\n");

  for (int i = 0; i < 10; i++)
  {
    moveServo(SERVO_CHANNEL, 45);
    delay(500);
    moveServo(SERVO_CHANNEL, 90);
    delay(500);

    Serial.printf("  Ciclo %d/10 ✓\n", i + 1);
  }

  Serial.println("\n✅ Se servo era sempre nella stessa posizione:");
  Serial.println("   → MG996R funziona correttamente!\n");
}

/**
 * Test stress (movimento continuo)
 */
void testStress()
{
  Serial.println("\n╔════════════════════════════════════╗");
  Serial.println("║  TEST STRESS (30 secondi)         ║");
  Serial.println("║  MONITORARE TEMPERATURA SERVO     ║");
  Serial.println("╚════════════════════════════════════╝\n");

  Serial.println("Movimento continuo 0° ↔ 180° per 30 secondi\n");
  unsigned long startTime = millis();
  int cycles = 0;

  while (millis() - startTime < 30000)
  {
    moveServoSmooth(SERVO_CHANNEL, 0, 180, 500, 20);
    moveServoSmooth(SERVO_CHANNEL, 180, 0, 500, 20);
    cycles++;
  }

  Serial.printf("✅ Test completato!");
  Serial.printf(" %d cicli completi\n", cycles);
  Serial.println("⚠️  Se servo è molto caldo: ridurre cicli di lavoro\n");
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
  Serial.println("╔════════════════════════════════════╗");
  Serial.println("║  ESP8266 + PCA9685 + MG996R       ║");
  Serial.println("║  Progetto Braccio Robotico        ║");
  Serial.println("╚════════════════════════════════════╝\n");

  // ==========================================
  // INIZIALIZZA I2C
  // ==========================================
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
  pwm.begin();
  pwm.setPWMFreq(50); // 50Hz per servo standard MG996R
  delay(10);

  Serial.println("✅ PCA9685 configurato a 50Hz (servo standard)\n");

  // ==========================================
  // INFORMAZIONI ALIMENTAZIONE
  // ==========================================
  Serial.println("⚡ CONFIGURAZIONE ALIMENTAZIONE:");
  Serial.println("   ├─ ESP8266: USB 5V (onboard regolatore → 3.3V)");
  Serial.println("   ├─ PCA9685 VCC (logica): 3.3V da ESP8266");
  Serial.println("   ├─ PCA9685 V+ (servo): BATTERIA 6V (o regolatore da 7.2V)");
  Serial.println("   └─ GND: COMUNE tra tutti\n");

  Serial.println("⚠️  ATTENZIONE - BATTERIA:");
  Serial.println("   └─ MG996R MAX 6.6V!");
  Serial.println("   └─ Se usi 7.2V → USA REGOLATORE 6.5V\n");

  // ==========================================
  // VERIFICA CORRENTE
  // ==========================================
  Serial.println("📊 SPECIFICHE MG996R:");
  Serial.println("   ├─ Angolo: 0° - 180°");
  Serial.println("   ├─ Coppia: 11kg/cm @ 6V");
  Serial.println("   ├─ Velocity: 0.19s/60°");
  Serial.println("   ├─ Corrente idle: ~5mA");
  Serial.println("   ├─ Corrente no load: ~200mA");
  Serial.println("   └─ Corrente MAX: 2.5A @ 6.6V\n");

  Serial.println("🔋 BATTERIA CONSIGLIATA:");
  Serial.println("   ├─ 4x AA NiMH = 4.8V (più sicuro)");
  Serial.println("   ├─ 5x AA NiMH = 6.0V (ottimale)");
  Serial.println("   └─ 7.2V LiPo con REGOLATORE 6.5V\n");

  // ==========================================
  // POSIZIONE INIZIALE SICURA
  // ==========================================
  Serial.println("🔧 Movimento a posizione iniziale (90°)...");
  moveServo(SERVO_CHANNEL, 90);
  delay(1500);

  Serial.println("\n✅ Setup completato!\n");
  Serial.println("╔════════════════════════════════════╗");
  Serial.println("║  COMANDI DISPONIBILI               ║");
  Serial.println("╚════════════════════════════════════╝");
  Serial.println("");
  Serial.println("  t - Test range completo (0-45-90-135-180°)");
  Serial.println("  s - Sweep rapido (0-180-0)");
  Serial.println("  m - Movimento fluido lento");
  Serial.println("  r - Test ripetibilità (10 cicli)");
  Serial.println("  x - Test stress (30 secondi)");
  Serial.println("  0-9 - Posizioni preset (0°, 20°, 40°, ..., 180°)");
  Serial.println("  a XXX - Vai ad angolo XXX (es: 'a 90')");
  Serial.println("  c - Calibrazione fine");
  Serial.println("  ? - Mostra questi comandi\n");
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
    command.toLowerCase();

    if (command == "t")
    {
      // Test range completo
      testServoRange();
    }
    else if (command == "s")
    {
      // Sweep rapido
      testSweep();
    }
    else if (command == "m")
    {
      // Movimento fluido
      testSmoothMovement();
    }
    else if (command == "r")
    {
      // Ripetibilità
      testRepeatability();
    }
    else if (command == "x")
    {
      // Stress test
      testStress();
    }
    else if (command == "c")
    {
      // Calibrazione
      // calibrateServo();
    }
    else if (command.startsWith("a "))
    {
      // Comando angolo custom: "a 90"
      float angle = command.substring(2).toFloat();
      if (angle >= 0 && angle <= 180)
      {
        Serial.printf("Movimento a %.1f°\n\n", angle);
        moveServo(SERVO_CHANNEL, angle);
      }
      else
      {
        Serial.println("❌ Angolo fuori range (0-180)");
        Serial.println("   Usa: 'a XXX' dove XXX è tra 0 e 180\n");
      }
    }
    else if (command.length() == 1 && isDigit(command[0]))
    {
      // Posizioni preset 0-9
      int preset = command.toInt();
      float angle = preset * 20; // 0=0°, 1=20°, ..., 9=180°

      Serial.printf("Preset %d → %.0f°\n\n", preset, angle);
      moveServo(SERVO_CHANNEL, angle);
    }
    else if (command == "?")
    {
      // Mostra comandi
      Serial.println("\n╔════════════════════════════════════╗");
      Serial.println("║  COMANDI DISPONIBILI               ║");
      Serial.println("╚════════════════════════════════════╝");
      Serial.println("");
      Serial.println("  t - Test range completo");
      Serial.println("  s - Sweep rapido");
      Serial.println("  m - Movimento fluido");
      Serial.println("  r - Test ripetibilità");
      Serial.println("  x - Test stress (30s)");
      Serial.println("  0-9 - Preset (0-20-40...180°)");
      Serial.println("  a XXX - Angolo custom");
      Serial.println("  c - Calibrazione\n");
    }
    else if (command == "")
    {
      // Invio vuoto, ignora
    }
    else
    {
      Serial.println("❌ Comando non riconosciuto!");
      Serial.println("   Digita '?' per lista comandi\n");
    }
  }

  delay(10);
}

// ============================================================================
// FUNZIONE: Calibrazione interattiva
// ============================================================================
void calibrateServo()
{
  Serial.println("\n╔════════════════════════════════════╗");
  Serial.println("║  CALIBRAZIONE MG996R              ║");
  Serial.println("╚════════════════════════════════════╝\n");

  Serial.println("Trova i valori PWM esatti per il tuo servo:\n");
  Serial.println("Comandi:");
  Serial.println("  + / - : ±10 PWM");
  Serial.println("  u / d : ±1 PWM");
  Serial.println("  m : Torna al centro");
  Serial.println("  0 : Torna a 0°");
  Serial.println("  1 : Torna a 180°");
  Serial.println("  s : Salva valore");
  Serial.println("  q : Esci\n");

  uint16_t currentPulse = SERVOMID;

  pwm.setPWM(SERVO_CHANNEL, 0, currentPulse);
  Serial.printf("PWM attuale: %d (centro)\n", currentPulse);
  Serial.println("Muovi il servo al punto desiderato, poi premi 's'\n");

  while (true)
  {
    if (Serial.available())
    {
      char cmd = Serial.read();

      if (cmd == '+')
      {
        currentPulse += 10;
        Serial.printf("  → PWM: %d\n", currentPulse);
      }
      else if (cmd == '-')
      {
        currentPulse -= 10;
        Serial.printf("  → PWM: %d\n", currentPulse);
      }
      else if (cmd == 'u')
      {
        currentPulse += 1;
        Serial.printf("  → PWM: %d\n", currentPulse);
      }
      else if (cmd == 'd')
      {
        currentPulse -= 1;
        Serial.printf("  → PWM: %d\n", currentPulse);
      }
      else if (cmd == 'm')
      {
        currentPulse = SERVOMID;
        Serial.printf("  → PWM: %d (centro)\n", currentPulse);
      }
      else if (cmd == '0')
      {
        currentPulse = SERVOMIN;
        Serial.printf("  → PWM: %d (0°)\n", currentPulse);
      }
      else if (cmd == '1')
      {
        currentPulse = SERVOMAX;
        Serial.printf("  → PWM: %d (180°)\n", currentPulse);
      }
      else if (cmd == 's')
      {
        Serial.printf("\n✅ VALORE SALVATO: %d\n", currentPulse);
        Serial.println("Aggiorna il #define nel codice:");
        Serial.printf("  #define SERVO_[NAME]  %d\n\n", currentPulse);
        continue;
      }
      else if (cmd == 'q')
      {
        Serial.println("\n✅ Calibrazione terminata\n");
        break;
      }

      // Limita range sicuro
      currentPulse = constrain(currentPulse, 50, 650);

      // Applica PWM
      pwm.setPWM(SERVO_CHANNEL, 0, currentPulse);
    }

    delay(10);
  }
}

/*******************************************************************************
 * SCHEMA COLLEGAMENTI
 ******************************************************************************/
/*

ESP8266 (NodeMCU)       PCA9685                Servo MG996R
┌────────────┐      ┌──────────────┐       ┌─────────────┐
│            │      │              │       │             │
│ 3.3V ──────┼─────►│ VCC (logica) │       │             │
│ GND  ──────┼──┬──►│ GND          │◄──────┤ GND (nero)  │
│            │  │   │              │       │             │
│ D2(GPIO4)──┼──┼──►│ SDA          │       │             │
│ D1(GPIO5)──┼──┼──►│ SCL          │       │             │
│            │  │   │              │       │             │
│ USB 5V     │  │   │ V+ (servo)   │◄──────┤ VCC (rosso) │
│ (per ESP)  │  │   │ (morsetto)   │       │             │
└────────────┘  │   └──────┬───────┘       │ Signal      │
                │          │                │ (giallo)    │
Batteria 6V*    │          └───────────────►│             │
┌────────────┐  │                          └─────────────┘
│  (+) ──────┼──┘
│  (-) ──────┼──────────────────────┘
└────────────┘

* SE BATTERIA 7.2V: aggiungi regolatore 6.5V tra batteria e servo!

REGOLATORE STEP-DOWN (Se necessario):
7.2V Batteria → [LM1117-6.5] → 6.5V → Servo + PCA9685 V+
                    │
                 GND (comune)

MOLTO IMPORTANTE:
✅ 1. ESP8266 3.3V → PCA9685 VCC (logica I2C)
✅ 2. Batteria → PCA9685 V+ (servo power)
✅ 3. GND COMUNE tra TUTTI
✅ 4. Mai alimentare servo da pin ESP8266!
✅ 5. Servo rosso → V+, Nero → GND, Giallo → CH0

*/

/*******************************************************************************
 * DIFFERENZE: MG996R vs Servo 270°
 ******************************************************************************/
/*

PARAMETRO           │  MG996R       │  Servo 270°
────────────────────┼───────────────┼─────────────────
Angolo             │  0-180°       │  0-270°
Coppia             │  11kg @ 6V    │  20-25kg
Tensione MAX       │  6.6V ⚠️      │  7.2V ✅
Corrente MAX       │  2.5A @ 6.6V  │  2.2A @ 6.8V
Velocità           │  0.19s/60°    │  0.13s/60°
Ingranaggi         │  Metallo      │  Metallo
Prezzo             │  €5-10        │  €12-20
Pulse Width        │  500-2500μs   │  500-2500μs

PER IL TUO PROGETTO:
├─ Se base rotante → MG996R (più economico, coppia sufficiente)
└─ Se base mobile pesante → Servo 270° (più potente)

*/

/*******************************************************************************
 * TROUBLESHOOTING MG996R
 ******************************************************************************/
/*

❌ PROBLEMA: Servo si muove solo in una direzione
✅ SOLUZIONE:
   - Calibra SERVOMIN (0°) e SERVOMAX (180°)
   - Usa funzione calibrateServo() per trovare valori esatti
   - Se servo spinge contro limite meccanico → aumenta range PWM

❌ PROBLEMA: Servo scalda molto
✅ SOLUZIONI:
   - Riduci cicli di lavoro (non movimento continuo)
   - Riduci tensione: usa 4.8V invece di 6.6V
   - Verifica non sia bloccato meccanicamente
   - Aggiungi dissipatore (per uso intenso)

❌ PROBLEMA: Movimento a scatti
✅ SOLUZIONI:
   - Batteria scarica → ricarica
   - Condensatore 470µF su V+ servo
   - Cavi più corti/spessi per ridurre resistenza
   - Verifica voltaggio reale (multimetro)

❌ PROBLEMA: Posizione instabile
✅ SOLUZIONI:
   - Servo MG996R ha ±2.5° isteresi (normale)
   - Se >5° → calibra fine
   - Verifica carico meccanico

❌ PROBLEMA: Servo zittisce/buzz
✅ SOLUZIONE:
   - Buzz = servo cerca di stare nella posizione
   - Se continuo: posizione fuori range PWM
   - Calibra o riduci carico

*/