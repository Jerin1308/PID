🚗 PID Line Follower Robot (Arduino)

This project implements a PID-controlled line follower robot using an Arduino.
It reads data from 6 IR sensors and adjusts motor speeds dynamically to follow a line smoothly.

📌 Features
🔍 6 IR sensor array for line detection
⚙️ PID control (Proportional–Integral–Derivative)
🎯 Smooth and stable line tracking
🔄 Auto calibration of sensors at startup
🛑 Line loss detection and safe stop
🧠 How It Works

The robot:

Reads analog values from 6 sensors
Converts them into a weighted position
Uses PID algorithm to calculate error
Adjusts motor speed to stay centered on the line
🧰 Hardware Required
Arduino Uno (or compatible)
IR sensor array (6 sensors)
Motor driver (e.g., L298N)
2 DC motors
Robot chassis
Battery pack
🔌 Pin Configuration
Sensors
Sensor	Pin
Leftmost → Rightmost	A0 → A5
Motors
Motor	Pin
Left IN1	2
Left IN2	3
Left PWM	5
Right IN1	4
Right IN2	7
Right PWM	6
⚙️ PID Parameters
float Kp = 20.0;
float Ki = 0.05;
float Kd = 150.0;

👉 You may need to tune these values depending on:

Motor speed
Track type
Sensor sensitivity
🚀 Getting Started
Upload the code using Arduino IDE
Place the robot on the track
Power it ON
It will auto-calibrate sensors (~700 ms)
The robot starts following the line
🔧 Tuning Tips
If robot oscillates → decrease Kd
If robot reacts slowly → increase Kp
If robot drifts → slightly increase Ki
Adjust baseSpeed for stability vs speed trade-off
⚠️ Notes
Ensure proper sensor alignment
Clean track surface improves accuracy
Calibration assumes mixed black/white surface during startup
📊 Debugging

Enable Serial Monitor for debugging:

// Uncomment debug lines in loop()
Serial.print("pos:");
Serial.print(position);
📄 License

This project is open-source and free to use for learning and development.

🙌 Acknowledgment

Built for learning robotics and control systems using PID.
