#include "RoboticArmMachine.h"
#include "include/set_up.h"
#include "kernel/Scheduler.h"
#include "kernel/Logger.h"
#include "kernel/MsgService.h"


RoboticArmMachine *machine;
Scheduler scheduler;
void setup() {
  Serial.begin(115200);
  Serial.println("\n🤖 Avvio Robotic Arm Controller...");
  MsgService.init();
  scheduler.init(200); // 200 ms base period

  // Inizializza macchina a stati
  machine = new RoboticArmMachine();
  machine->begin();

  Serial.println("✅ Sistema inizializzato.");
  Serial.println("Comandi disponibili:");
  Serial.println("  b → Test base");
  Serial.println("  e → Test elbow");
  Serial.println("  w → Test wrist");
  Serial.println("  c → Test claw");
  Serial.println("  center → Posizione centrale");
  Serial.println("  safe → Posizione di sicurezza");
  Serial.println();
}

/*quando presenti i task
void loop() {
    scheduler.schedule();
}
*/
void loop()
{
  if (Serial.available())
  {
    String command = Serial.readStringUntil('\n');
    command.trim();

    if (command == "b")
    {
      // Test range
      Serial.printf("Base angle: %.1f°\n", machine->getBaseAngle());
      machine->moveBaseServo(60);
      delay(1000);
      machine->moveBaseServo(120);
      delay(1000);
      machine->moveBaseServo(90);
    }
    else if (command == "e")
    {
      // Test range
      Serial.printf("Elbow angle: %.1f°\n", machine->getElbowAngle());
      machine->moveElbowServo(60);
      delay(1000);
      machine->moveElbowServo(120);
      delay(1000);
      machine->moveElbowServo(90);
    }
    else if (command == "w")
    {
      // Test range
      Serial.printf("Wrist angle: %.1f°\n", machine->getWristAngle());
      machine->moveWristServo(60);
      delay(1000);
      machine->moveWristServo(120);
      delay(1000);
      machine->moveWristServo(90);
    }
    else if (command == "c")
    {
      // Test range
      Serial.printf("Claw angle: %.1f°\n", machine->getClawAngle());
      machine->moveClawServo(60);
      delay(1000);
      machine->moveClawServo(120);
      delay(1000);
      machine->moveClawServo(90);
    }
    else if (command == "safe")
    {
      // Posizione sicura
      machine->moveAllToSafePosition();
    }
    else if (command == "center")
    {
      // Posizione centrale
      machine->moveAllToCenter();
    }
    
    else
    {
      Serial.println("❌ Comando non riconosciuto");
      Serial.println("Usa: t, s, c, 0-9, o 'a XXX'");
    }
  }

  delay(10);
}
