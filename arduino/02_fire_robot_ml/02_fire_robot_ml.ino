/*
  Fire fighting robot dengan keputusan berbasis model Decision Tree.
  Ganti fungsi predictFireDirection() dengan hasil training data asli besok.
*/

#include <AFMotor.h>
#include <Servo.h>

AF_DCMotor leftMotor(3);
AF_DCMotor rightMotor(2);
AF_DCMotor pumpMotor(4);

Servo sprayServo;

const int flamePins[5] = { A0, A1, A2, A3, A4 };

const int MOTOR_SPEED = 100;
const int PUMP_SPEED = 200;

const int SERVO_LEFT = 55;
const int SERVO_CENTER = 90;
const int SERVO_RIGHT = 125;

enum FirePrediction {
  NO_FIRE = 0,
  FIRE_LEFT = 1,
  FIRE_CENTER = 2,
  FIRE_RIGHT = 3,
  FIRE_CLOSE = 4,
};

void setup() {
  Serial.begin(9600);

  sprayServo.attach(9);
  sprayServo.write(SERVO_CENTER);

  leftMotor.setSpeed(MOTOR_SPEED);
  rightMotor.setSpeed(MOTOR_SPEED);
  pumpMotor.setSpeed(PUMP_SPEED);

  stopMotors();
  pumpMotor.run(RELEASE);
}

void loop() {
  int s1 = analogRead(flamePins[0]);
  int s2 = analogRead(flamePins[1]);
  int s3 = analogRead(flamePins[2]);
  int s4 = analogRead(flamePins[3]);
  int s5 = analogRead(flamePins[4]);

  int prediction = predictFireDirection(s1, s2, s3, s4, s5);

  printDebug(s1, s2, s3, s4, s5, prediction);
  applyPrediction(prediction, s1, s2, s3, s4, s5);

  delay(150);
}

int predictFireDirection(int s1, int s2, int s3, int s4, int s5) {
  // Hasil training dummy dataset. Besok ganti dari hasil training data asli.
  if (s4 <= 595) {
    return FIRE_RIGHT;
  } else {
    if (s2 <= 590) {
      return FIRE_LEFT;
    } else {
      if (s3 <= 878) {
        if (s2 <= 798) {
          return FIRE_CENTER;
        } else {
          return NO_FIRE;
        }
      } else {
        if (s2 <= 882) {
          return FIRE_CLOSE;
        } else {
          return NO_FIRE;
        }
      }
    }
  }
}

void applyPrediction(int prediction, int s1, int s2, int s3, int s4, int s5) {
  if (prediction == FIRE_LEFT) {
    sprayServo.write(SERVO_LEFT);
    pumpMotor.run(RELEASE);
    turnLeft();
    return;
  }

  if (prediction == FIRE_CENTER) {
    sprayServo.write(SERVO_CENTER);
    pumpMotor.run(RELEASE);
    moveForward();
    return;
  }

  if (prediction == FIRE_RIGHT) {
    sprayServo.write(SERVO_RIGHT);
    pumpMotor.run(RELEASE);
    turnRight();
    return;
  }

  if (prediction == FIRE_CLOSE) {
    stopMotors();
    extinguishAtServo(angleFromStrongestSensor(s1, s2, s3, s4, s5));
    return;
  }

  sprayServo.write(SERVO_CENTER);
  pumpMotor.run(RELEASE);
  stopMotors();
}

void extinguishAtServo(int center) {
  pumpMotor.run(FORWARD);

  for (int pos = center - 10; pos <= center + 10; pos += 5) {
    sprayServo.write(pos);
    delay(100);
  }

  for (int pos = center + 10; pos >= center - 10; pos -= 5) {
    sprayServo.write(pos);
    delay(100);
  }

  sprayServo.write(center);
  pumpMotor.run(RELEASE);
}

int angleFromStrongestSensor(int s1, int s2, int s3, int s4, int s5) {
  int values[5] = { s1, s2, s3, s4, s5 };
  int strongestIndex = 0;

  for (int i = 1; i < 5; i++) {
    if (values[i] > values[strongestIndex]) {
      strongestIndex = i;
    }
  }

  if (strongestIndex < 2) {
    return SERVO_LEFT;
  }

  if (strongestIndex > 2) {
    return SERVO_RIGHT;
  }

  return SERVO_CENTER;
}

void turnLeft() {
  leftMotor.run(BACKWARD);
  rightMotor.run(FORWARD);
}

void turnRight() {
  leftMotor.run(FORWARD);
  rightMotor.run(BACKWARD);
}

void moveForward() {
  leftMotor.run(BACKWARD);
  rightMotor.run(BACKWARD);
}

void stopMotors() {
  leftMotor.run(RELEASE);
  rightMotor.run(RELEASE);
}

void printDebug(int s1, int s2, int s3, int s4, int s5, int prediction) {
  Serial.print(s1);
  Serial.print(",");
  Serial.print(s2);
  Serial.print(",");
  Serial.print(s3);
  Serial.print(",");
  Serial.print(s4);
  Serial.print(",");
  Serial.print(s5);
  Serial.print(" => ");
  Serial.println(predictionName(prediction));
}

const char* predictionName(int prediction) {
  if (prediction == FIRE_LEFT) return "FIRE_LEFT";
  if (prediction == FIRE_CENTER) return "FIRE_CENTER";
  if (prediction == FIRE_RIGHT) return "FIRE_RIGHT";
  if (prediction == FIRE_CLOSE) return "FIRE_CLOSE";
  return "NO_FIRE";
}
