/*
  Fire fighting robot dengan keputusan berbasis model Decision Tree.

  Alur keputusan (sesuai laporan):
  1. predictFireDirection() mengklasifikasikan arah api → 4 kelas
  2. Jika hasilnya FIRE_CENTER DAN s3 <= PROXIMITY_THRESHOLD → robot berhenti & padamkan
  3. Selain itu → robot bergerak mengikuti arah klasifikasi

  Ganti fungsi predictFireDirection() dengan hasil dari train_fire_model.py
  setelah training menggunakan data asli dari robot.
*/

#include <AFMotor.h>
#include <Servo.h>

AF_DCMotor leftMotor(3);
AF_DCMotor rightMotor(2);
AF_DCMotor pumpMotor(4);

Servo sprayServo;

const int flamePins[5] = { A0, A1, A2, A3, A4 };

const int MOTOR_SPEED = 100;
const int PUMP_SPEED  = 200;

const int SERVO_LEFT   = 55;
const int SERVO_CENTER = 90;
const int SERVO_RIGHT  = 125;

// Threshold proximity pada channel tengah (A2 / s3).
// Nilai s3 di bawah ini berarti api cukup dekat untuk dipadamkan.
// Sesuaikan setelah pengambilan data asli.
const int PROXIMITY_THRESHOLD = 400;

enum FireDirection {
  NO_FIRE    = 0,
  FIRE_LEFT  = 1,
  FIRE_CENTER = 2,
  FIRE_RIGHT = 3,
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

  int direction = predictFireDirection(s1, s2, s3, s4, s5);
  bool isClose  = (direction == FIRE_CENTER) && (s3 <= PROXIMITY_THRESHOLD);

  printDebug(s1, s2, s3, s4, s5, direction, isClose);
  applyDecision(direction, isClose);

  delay(150);
}

// ---------------------------------------------------------------
// GANTI fungsi ini dengan output dari train_fire_model.py
// setelah training menggunakan data sensor asli.
// ---------------------------------------------------------------
int predictFireDirection(int s1, int s2, int s3, int s4, int s5) {
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
        return NO_FIRE;
      }
    }
  }
}

void applyDecision(int direction, bool isClose) {
  if (direction == FIRE_LEFT) {
    sprayServo.write(SERVO_LEFT);
    pumpMotor.run(RELEASE);
    turnLeft();
    return;
  }

  if (direction == FIRE_RIGHT) {
    sprayServo.write(SERVO_RIGHT);
    pumpMotor.run(RELEASE);
    turnRight();
    return;
  }

  if (direction == FIRE_CENTER) {
    sprayServo.write(SERVO_CENTER);
    if (isClose) {
      stopMotors();
      extinguish();
    } else {
      pumpMotor.run(RELEASE);
      moveForward();
    }
    return;
  }

  // NO_FIRE
  sprayServo.write(SERVO_CENTER);
  pumpMotor.run(RELEASE);
  stopMotors();
}

void extinguish() {
  pumpMotor.run(FORWARD);
  for (int pos = SERVO_CENTER - 10; pos <= SERVO_CENTER + 10; pos += 5) {
    sprayServo.write(pos);
    delay(100);
  }
  for (int pos = SERVO_CENTER + 10; pos >= SERVO_CENTER - 10; pos -= 5) {
    sprayServo.write(pos);
    delay(100);
  }
  sprayServo.write(SERVO_CENTER);
  pumpMotor.run(RELEASE);
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

void printDebug(int s1, int s2, int s3, int s4, int s5, int direction, bool isClose) {
  Serial.print(s1); Serial.print(",");
  Serial.print(s2); Serial.print(",");
  Serial.print(s3); Serial.print(",");
  Serial.print(s4); Serial.print(",");
  Serial.print(s5); Serial.print(" => ");
  Serial.print(directionName(direction));
  if (isClose) Serial.print(" [EXTINGUISH]");
  Serial.println();
}

const char* directionName(int direction) {
  if (direction == FIRE_LEFT)   return "FIRE_LEFT";
  if (direction == FIRE_CENTER) return "FIRE_CENTER";
  if (direction == FIRE_RIGHT)  return "FIRE_RIGHT";
  return "NO_FIRE";
}
