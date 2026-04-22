const int SENSOR_COUNT = 6;
const int sensorPins[SENSOR_COUNT] = {A0, A1, A2, A3, A4, A5}; // A0 = leftmost, A5 = rightmost

const int L_IN1 = 2;
const int L_IN2 = 3;
const int L_PWM  = 5; // ENA (must be PWM)

const int R_IN1 = 4;
const int R_IN2 = 7;
const int R_PWM  = 6; // ENB (must be PWM)

// PID params (start here; tune further)
float Kp = 20.0;   // Proportional
float Ki = 0.05;   // Integral
float Kd = 150.0;  // Derivative

// Base speed (0-255) - change according to your motors/battery
int baseSpeed = 160;

// Sensor weights - gives a continuous position
int weights[SENSOR_COUNT] = {-5, -3, -1, 1, 3, 5};

// Calibration arrays
int sensorMin[SENSOR_COUNT];
int sensorMax[SENSOR_COUNT];

unsigned long lastTime = 0;
float integral = 0.0;
float lastError = 0.0;

void setup() {
  Serial.begin(115200);
  // motor pins
  pinMode(L_IN1, OUTPUT);
  pinMode(L_IN2, OUTPUT);
  pinMode(L_PWM, OUTPUT);

  pinMode(R_IN1, OUTPUT);
  pinMode(R_IN2, OUTPUT);
  pinMode(R_PWM, OUTPUT);

  // initialize calibration extremes
  for (int i = 0; i < SENSOR_COUNT; ++i) {
    sensorMin[i] = 1023;
    sensorMax[i] = 0;
  }

  // Auto-calibration phase: sample for ~700ms and compute min/max
  unsigned long start = millis();
  while (millis() - start < 700UL) {
    for (int i = 0; i < SENSOR_COUNT; ++i) {
      int v = analogRead(sensorPins[i]);
      if (v < sensorMin[i]) sensorMin[i] = v;
      if (v > sensorMax[i]) sensorMax[i] = v;
    }
  }
  // small safety if min == max (flat environment)
  for (int i = 0; i < SENSOR_COUNT; ++i) {
    if (sensorMin[i] >= sensorMax[i]) {
      // set reasonable defaults
      sensorMin[i] = 0;
      sensorMax[i] = 800; // typical
    }
  }

  lastTime = millis();
  stopMotors();
  delay(50);
}

// Map raw analog to 0..1000 (higher means more reflection depending on module orientation).
int readSensorScaled(int idx) {
  int raw = analogRead(sensorPins[idx]);
  // scale and clamp 0..1000
  long scaled = (long)(raw - sensorMin[idx]) * 1000L / (long)(sensorMax[idx] - sensorMin[idx]);
  if (scaled < 0) scaled = 0;
  if (scaled > 1000) scaled = 1000;
  return (int)scaled;
}

void loop() {
  // Read sensors quickly
  int vals[SENSOR_COUNT];
  long weightedSum = 0;
  long sum = 0;
  for (int i = 0; i < SENSOR_COUNT; ++i) {
    vals[i] = readSensorScaled(i); // 0..1000
    // For typical RLS sensors: higher = more reflection (white) — if your line is dark on bright background, invert if needed
    long v = vals[i];
    weightedSum += (long)weights[i] * v;
    sum += v;
  }

  // If sum is very small -> lost line (all black) or all white depending on sensor orientation
  float position = 0.0;
  bool lost = false;
  if (sum < 50) {
    // Lost line: no strong readings. Try to stop or use last error to recover.
    lost = true;
  } else {
    position = (float)weightedSum / (float)sum; // expected range approx -5..+5
  }

  // PID compute
  unsigned long now = millis();
  float dt = (now - lastTime) / 1000.0; // seconds
  if (dt <= 0) dt = 0.001;
  float error = position; // target is 0 (center). positive = drift right
  integral += error * dt;
  float derivative = (error - lastError) / dt;
  float correction = Kp * error + Ki * integral + Kd * derivative;
  lastError = error;
  lastTime = now;

  // Compute motor speeds
  int leftSpeed, rightSpeed;
  if (lost) {
    // recovery policy: stop or spin gently toward last known side.
    // Here we stop motors for safety.
    leftSpeed = 0;
    rightSpeed = 0;
    // Optionally: implement a search turn using lastError sign to rotate and find line
  } else {
    float leftF = (float)baseSpeed + correction;
    float rightF = (float)baseSpeed - correction;
    // constrain
    if (leftF > 255) leftF = 255;
    if (leftF < -255) leftF = -255;
    if (rightF > 255) rightF = 255;
    if (rightF < -255) rightF = -255;
    leftSpeed = (int)leftF;
    rightSpeed = (int)rightF;
  }

  setMotors(leftSpeed, rightSpeed);

  // Optional: debug print (uncomment for tuning)
  // Serial.print("pos:"); Serial.print(position);
  // Serial.print(" corr:"); Serial.print(correction);
  // Serial.print(" L:"); Serial.print(leftSpeed);
  // Serial.print(" R:"); Serial.println(rightSpeed);

  // very short idle to let MCU breathe; not a blocking delay — keeps loop stable
  delayMicroseconds(200);
}

void setMotors(int left, int right) {
  // left: positive -> forward; negative -> backward
  if (left >= 0) {
    digitalWrite(L_IN1, HIGH);
    digitalWrite(L_IN2, LOW);
    analogWrite(L_PWM, left);
  } else {
    digitalWrite(L_IN1, LOW);
    digitalWrite(L_IN2, HIGH);
    analogWrite(L_PWM, -left);
  }

  // right: positive -> forward; negative -> backward
  if (right >= 0) {
    digitalWrite(R_IN1, HIGH);
    digitalWrite(R_IN2, LOW);
    analogWrite(R_PWM, right);
  } else {
    digitalWrite(R_IN1, LOW);
    digitalWrite(R_IN2, HIGH);
    analogWrite(R_PWM, -right);
  }
}

void stopMotors() {
  analogWrite(L_PWM, 0);
  analogWrite(R_PWM, 0);
  digitalWrite(L_IN1, LOW);
  digitalWrite(L_IN2, LOW);
  digitalWrite(R_IN1, LOW);
  digitalWrite(R_IN2, LOW);
}
