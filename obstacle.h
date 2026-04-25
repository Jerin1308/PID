#ifndef OBSTACLE_H
#define OBSTACLE_H

#include "motor.h"
#include <Servo.h>

Servo myservo;

int pos = 90;
float distanceCm;
float distanceInch;

#define trig 8
#define echo 9
uint16_t duration;
uint16_t distance;

uint8_t ser[3] = {0, 90, 180};
uint16_t dis1, dis2, dis3;
void servoMovement();

int reading() {

  digitalWrite(trig, LOW);
  delayMicroseconds(2);
  digitalWrite(trig, HIGH);
  delayMicroseconds(10);
  digitalWrite(trig, LOW);

  duration = pulseIn(echo, HIGH, 25000);
  distance = duration * 0.034 / 2;

  return distance;

}

void servoFullRotation() {
  for (uint8_t i = 0; i <= 180; i++) {
    myservo.write(i);
    delay(20);
  }
}

void servoDistance() {
  motorStop();
  for (int i = 90; i <= 180; i++) {
    myservo.write(i);
    delay(20);
  }
  myservo.write(180);
  delay(500);
  dis2 = reading();
  delay(500);
  for (int i = 180; i >= 0; i--) {
    myservo.write(i);
    delay(20);
  }
  myservo.write(0);
  delay(500);
  dis3 = reading();
  delay(500);

  for (int i = 0; i <= 90; i++) {
    myservo.write(i);
    delay(20);
  }
  myservo.write(90);
  delay(500);
  dis1 = reading();
  delay(500);
  servoMovement();
}


void servoMovement() {

  if (dis1 > dis2  && dis3 < dis1) {
    motorForward();
    return;
  }

  if (dis1 < dis2  && dis3 < dis2) {
    motorRight();
    motorForward();
    return;
  }

  if (dis1 < dis3 && dis3 > dis2) {
    motorLeft();
    motorForward();
    return;
  }
  return;
}


#endif
