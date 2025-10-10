#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();

#define SERVOMIN  150 // valore minimo (dipende dal servo)
#define SERVOMAX  600 // valore massimo (dipende dal servo)
#define NUM_SERVO 4   // cambia in base a quanti servo hai collegato

void setup() {
  Serial.begin(115200);
  Serial.println("Test PCA9685 con servo...");

  pwm.begin();
  pwm.setPWMFreq(50); // 50 Hz per servo standard
  delay(10);
}

void loop() {
  for (int servo = 0; servo < NUM_SERVO; servo++) {
    Serial.print("Muovo servo ");
    Serial.println(servo);

    // muove il servo da minimo a massimo
    for (int pulselen = SERVOMIN; pulselen < SERVOMAX; pulselen++) {
      pwm.setPWM(servo, 0, pulselen);
      delay(2);
    }

    delay(500);

    // muove il servo indietro
    for (int pulselen = SERVOMAX; pulselen > SERVOMIN; pulselen--) {
      pwm.setPWM(servo, 0, pulselen);
      delay(2);
    }

    delay(1000);
  }
}