#include "motor.h"
#include "logic.h"
#include "obstacle.h"

uint16_t sensorValues[NUM_SENSORS];
uint16_t dis;

void setup() {
  Serial.begin(115200);

  myservo.attach(10);
  myservo.write(90);

  pinMode(trig, OUTPUT);
  pinMode(echo, INPUT);

  pinMode(LEFT_EN, OUTPUT);
  pinMode(LEFT_IN1, OUTPUT);
  pinMode(LEFT_IN2, OUTPUT);

  pinMode(RIGHT_EN, OUTPUT);
  pinMode(RIGHT_IN1, OUTPUT);
  pinMode(RIGHT_IN2, OUTPUT);

  pinMode(UP_1, OUTPUT);
  pinMode(UP_2, OUTPUT);

  analogWrite(LEFT_EN, 0); analogWrite(RIGHT_EN, 0);
  digitalWrite(LEFT_IN1, LOW); digitalWrite(LEFT_IN2, LOW);
  digitalWrite(RIGHT_IN1, LOW); digitalWrite(RIGHT_IN2, LOW);
  digitalWrite(UP_1, LOW); digitalWrite(UP_2, LOW);

  qtr.setTypeAnalog();
  qtr.setSensorPins((const uint8_t[]) {
    A0, A1, A2, A3, A4, A5
  }, NUM_SENSORS);
  qtr.setEmitterPin(EMITTER_PIN);

  for (int i = 0; i < 250; i++) {
    qtr.calibrate();
    delay(10);
  }
}

void loop() {

  processSerial();

  if (ctrlState == STATE_ACTION) {
    handleAction();
    return;
  }

  if (ctrlState == STATE_NORMAL) {
    switch (Mode) {

      case MODE_LINE:
        lineFollow();
        break;

      case MODE_OBSTACLE:
        dis1 = reading();
        if (dis1 < 50) {
          servoDistance();
        } else {
          motorForward();
        }
        break;

    }
  }
}
