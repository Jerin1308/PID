#ifndef LOGIC_H
#define LOGIC_H

#include <QTRSensors.h>
#include "motor.h"

#define NUM_SENSORS 6
#define EMITTER_PIN A6
extern uint16_t sensorValues[NUM_SENSORS];
QTRSensors qtr;

#define MODE_LINE     'L'
#define MODE_OBSTACLE 'A'
#define MODE_MANUAL   'M'

char Mode = MODE_LINE;

enum ControlState {
  STATE_NORMAL,
  STATE_STOPPED,
  STATE_ACTION
};

ControlState ctrlState = STATE_NORMAL;
char currentAction = '\0';

int baseSpeed = 230;
float Kp = 0.12;
float Kd = 2.8;
int lastError = 0;
const int WHITE_THR = 80;

int leftOffset  = 10;
int rightOffset = 0;


void processSerial() {
  while (Serial.available()) {
    char c = Serial.read();

    if (c == MODE_LINE || c == MODE_OBSTACLE || c == MODE_MANUAL) {
      Mode = c;
      ctrlState = STATE_NORMAL;
      currentAction = '\0';
      return;
    }

    if (c == 'S') {
      stopMotors();
      ctrlState = STATE_STOPPED;
      currentAction = '\0';
      return;
    }

    if (ctrlState == STATE_STOPPED) {
      if (c == 'U' || c == 'D' || c == 'B') {
        stopMotors();
        currentAction = c;
        ctrlState = STATE_ACTION;
        return;
      }
    }

    if (c == 'X') {
      stopMotors();
      ctrlState = STATE_NORMAL;
      currentAction = '\0';
      return;
    }
  }
}

void handleAction() {
  switch (currentAction) {
    case 'U':
      motorUp();
      break;

    case 'D':
      motorDown_1();
      delay(15000);
      break;

    case 'B':
      delay(100);
      motorDown_1();
      delay(6000);
      motorUp();
      break;

    case 'H':
      motorDown_1();
      delay(15000);
      break;
  }
}

void lineFollow() {

  uint16_t position = qtr.readLineBlack(sensorValues);

  bool allWhite = true;
  for (int i = 0; i < 6; i++) {
    if (sensorValues[i] > WHITE_THR) {
      allWhite = false;
      break;
    }
  }

  if (allWhite) {
    stopMotors();
    lastError = 0;
    return;
  }

  // Hard left
  if (sensorValues[0] > 500 || sensorValues[1] > 500) {
    leftBackward(230);
    rightForward(230);
    delay(350);
    return;
  }

  // Hard right
  if (sensorValues[4] > 500) {
    leftForward(230);
    rightBackward(230);
    delay(1200);
    return;
  }

  int error = position - 2500;

  if (abs(error) < 250) {
    leftForward(baseSpeed + leftOffset);
    rightForward(baseSpeed + rightOffset);
    lastError = 0;
    return;
  }

  int correction = (Kp * error) + (Kd * (error - lastError));
  lastError = error;

  int leftSpeed  = baseSpeed + correction;
  int rightSpeed = baseSpeed - correction;

  leftSpeed  = constrain(leftSpeed, 120, 255);
  rightSpeed = constrain(rightSpeed, 120, 255);

  leftForward(leftSpeed);
  rightForward(rightSpeed);
}


#endif
