#ifndef MOTOR_H
#define MOTOR_H

#define LEFT_EN 3
#define LEFT_IN1 2
#define LEFT_IN2 4

#define RIGHT_EN 5
#define RIGHT_IN1 6
#define RIGHT_IN2 7

#define UP_1 11
#define UP_2 12

void left(int sp) {

  analogWrite(LEFT_EN, sp);
  analogWrite(RIGHT_EN, sp);
  digitalWrite(LEFT_IN1, LOW);
  digitalWrite(LEFT_IN2, HIGH);
  digitalWrite(RIGHT_IN1, HIGH);
  digitalWrite(RIGHT_IN2, LOW);
  delay(250);

}

void right(int sp) {

  analogWrite(LEFT_EN, sp);
  analogWrite(RIGHT_EN, sp);
  digitalWrite(LEFT_IN1, HIGH);
  digitalWrite(LEFT_IN2, LOW);
  digitalWrite(RIGHT_IN1, LOW);
  digitalWrite(RIGHT_IN2, HIGH);
  delay(1000);

}

void leftForward(int sp) {
  analogWrite(LEFT_EN, sp);
  digitalWrite(LEFT_IN1, HIGH);
  digitalWrite(LEFT_IN2, LOW);
}

void leftBackward(int sp) {
  analogWrite(LEFT_EN, sp);
  digitalWrite(LEFT_IN1, LOW);  
  digitalWrite(LEFT_IN2, HIGH);
}

void rightForward(int sp) {
  analogWrite(RIGHT_EN, sp);
  digitalWrite(RIGHT_IN1, HIGH);
  digitalWrite(RIGHT_IN2, LOW);
}

void rightBackward(int sp) {
  analogWrite(RIGHT_EN, sp);
  digitalWrite(RIGHT_IN1, LOW);
  digitalWrite(RIGHT_IN2, HIGH);
}

void motorDown() {
  digitalWrite(UP_1, LOW);
  digitalWrite(UP_2, HIGH);

  unsigned long start = millis();
  while (millis() - start < 2000) {
  }
  digitalWrite(UP_1, LOW);
  digitalWrite(UP_2, LOW);
}

void motorDown_1() {
  digitalWrite(UP_1, LOW); digitalWrite(UP_2, LOW);
}

void motorUp() {
  digitalWrite(UP_1, HIGH);
  digitalWrite(UP_2, LOW);
  unsigned long start = millis();
  while (millis() - start < 2500) {
  }
  digitalWrite(UP_1, LOW);
  digitalWrite(UP_2, LOW);

  start = millis();
  while (millis() - start < 10000) {
  }
  motorDown();
}


void stopMotors() {
  analogWrite(LEFT_EN, 0); analogWrite(RIGHT_EN, 0);
  digitalWrite(LEFT_IN1, LOW);
  digitalWrite(LEFT_IN2, LOW);
  digitalWrite(RIGHT_IN1, LOW);
  digitalWrite(RIGHT_IN2, LOW);
  digitalWrite(UP_1, LOW); 
  digitalWrite(UP_2, LOW);
}



void motorBackward() {

  analogWrite(LEFT_EN, 180);
  analogWrite(RIGHT_EN, 180);
  digitalWrite(LEFT_IN1, LOW);
  digitalWrite(LEFT_IN2, HIGH);
  digitalWrite(RIGHT_IN1, LOW);
  digitalWrite(RIGHT_IN2, HIGH);//  delay(1000);

}

void motorForward() {

  analogWrite(LEFT_EN, 230);
  analogWrite(RIGHT_EN, 230);
  digitalWrite(LEFT_IN1, HIGH);
  digitalWrite(LEFT_IN2, LOW);
  digitalWrite(RIGHT_IN1, HIGH);
  digitalWrite(RIGHT_IN2, LOW);
  delay(1000);

}

void motorLeft () {

  analogWrite(LEFT_EN, 230);
  analogWrite(RIGHT_EN, 230);
  digitalWrite(LEFT_IN1, HIGH);
  digitalWrite(LEFT_IN2, LOW);
  digitalWrite(RIGHT_IN1, LOW);
  digitalWrite(RIGHT_IN2, HIGH);
  delay(10000);

}

void motorRight() {

  analogWrite(LEFT_EN, 230);
  analogWrite(RIGHT_EN, 230);
  digitalWrite(LEFT_IN1, LOW);
  digitalWrite(LEFT_IN2, HIGH);
  digitalWrite(RIGHT_IN1, HIGH);
  digitalWrite(RIGHT_IN2, LOW);
  delay(10000);

}

void motorStop() {

  analogWrite(LEFT_EN, 0);
  analogWrite(RIGHT_EN, 0);
  digitalWrite(LEFT_IN1, LOW);
  digitalWrite(LEFT_IN2, LOW);
  digitalWrite(RIGHT_IN1, LOW);
  digitalWrite(RIGHT_IN2, LOW);
  digitalWrite(UP_1, LOW); 
  digitalWrite(UP_2, LOW);
//  delay(1000);

}




#endif
