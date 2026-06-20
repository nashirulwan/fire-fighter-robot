/*
  Fire fighting robot dengan keputusan berbasis model Decision Tree.
  Urutan logis sensor: s1=A5, s2=A4, s3=A3, s4=A2, s5=A1.

  Alur keputusan:
  1. predictFireDirection() mengklasifikasikan arah api ke 4 kelas.
  2. Jika hasilnya FIRE_CENTER dan s3 >= PROXIMITY_THRESHOLD, robot berhenti dan padamkan.
  3. Selain itu, robot bergerak mengikuti arah klasifikasi.

  Ganti fungsi predictFireDirection() dengan hasil dari train_fire_model.py
  setelah training menggunakan data asli dari robot.
*/

#include <AFMotor.h>
#include <Servo.h>

AF_DCMotor leftMotor(3);
AF_DCMotor rightMotor(2);
AF_DCMotor pumpMotor(4);

Servo sprayServo;

// Urutan logis dataset: s1=A5, s2=A4, s3=A3, s4=A2, s5=A1.
const int flamePins[5] = { A5, A4, A3, A2, A1 };

const int MOTOR_SPEED = 100;
const int PUMP_SPEED  = 200;
const int MOTOR_RAMP_STEP = 20;
const int MOTOR_RAMP_DELAY_MS = 25;
const int MOTOR_DIRECTION_CHANGE_PAUSE_MS = 60;

const int SERVO_LEFT   = 55;
const int SERVO_CENTER = 90;
const int SERVO_RIGHT  = 125;

// Threshold proximity pada channel tengah (A3 / s3).
// Untuk dataset ini, nilai s3 yang lebih tinggi berarti api tengah lebih kuat/dekat.
// Nilai 1000 dipakai sebagai default konservatif dan tetap perlu kalibrasi robot fisik.
const int PROXIMITY_THRESHOLD = 1000;

enum FireDirection {
  NO_FIRE = 0,
  FIRE_LEFT = 1,
  FIRE_CENTER = 2,
  FIRE_RIGHT = 3,
};

int currentDriveSpeed = 0;
uint8_t currentLeftDirection = RELEASE;
uint8_t currentRightDirection = RELEASE;

void setup() {
  Serial.begin(9600);

  sprayServo.attach(9);
  sprayServo.write(SERVO_CENTER);

  leftMotor.setSpeed(0);
  rightMotor.setSpeed(0);
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
  bool isClose = (direction == FIRE_CENTER) && (s3 >= PROXIMITY_THRESHOLD);

  printDebug(s1, s2, s3, s4, s5, direction, isClose);
  applyDecision(direction, isClose);

  delay(150);
}

// ---------------------------------------------------------------
// GANTI fungsi ini dengan output dari train_fire_model.py
// setelah training menggunakan data sensor asli.
// ---------------------------------------------------------------
int predictFireDirection(int s1, int s2, int s3, int s4, int s5) {
  if (s4 <= 172) {
    return NO_FIRE;
  } else {
    if (s3 <= 795) {
      if (s1 <= 485) {
        return FIRE_LEFT;
      } else {
        return FIRE_RIGHT;
      }
    } else {
      if (s2 <= 704) {
        return FIRE_LEFT;
      } else {
        if (s1 <= 998) {
          return FIRE_CENTER;
        } else {
          return FIRE_RIGHT;
        }
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

  sprayServo.write(SERVO_CENTER);
  pumpMotor.run(RELEASE);
  stopMotors();
}

void rampDriveSpeed(int targetSpeed) {
  targetSpeed = constrain(targetSpeed, 0, MOTOR_SPEED);

  while (currentDriveSpeed != targetSpeed) {
    if (currentDriveSpeed < targetSpeed) {
      currentDriveSpeed += MOTOR_RAMP_STEP;
      if (currentDriveSpeed > targetSpeed) currentDriveSpeed = targetSpeed;
    } else {
      currentDriveSpeed -= MOTOR_RAMP_STEP;
      if (currentDriveSpeed < targetSpeed) currentDriveSpeed = targetSpeed;
    }

    leftMotor.setSpeed(currentDriveSpeed);
    rightMotor.setSpeed(currentDriveSpeed);
    delay(MOTOR_RAMP_DELAY_MS);
  }
}

bool isDirectionChange(uint8_t currentDirection, uint8_t nextDirection) {
  return currentDirection != RELEASE &&
         nextDirection != RELEASE &&
         currentDirection != nextDirection;
}

void setDriveState(uint8_t leftDirection, uint8_t rightDirection) {
  if (currentLeftDirection == leftDirection &&
      currentRightDirection == rightDirection) {
    if (leftDirection == RELEASE && rightDirection == RELEASE && currentDriveSpeed == 0) {
      return;
    }

    if (leftDirection != RELEASE && rightDirection != RELEASE && currentDriveSpeed == MOTOR_SPEED) {
      return;
    }
  }

  bool shouldPauseForReverse =
    isDirectionChange(currentLeftDirection, leftDirection) ||
    isDirectionChange(currentRightDirection, rightDirection);

  rampDriveSpeed(0);

  leftMotor.run(RELEASE);
  rightMotor.run(RELEASE);
  currentLeftDirection = RELEASE;
  currentRightDirection = RELEASE;

  if (leftDirection == RELEASE && rightDirection == RELEASE) {
    return;
  }

  if (shouldPauseForReverse) {
    delay(MOTOR_DIRECTION_CHANGE_PAUSE_MS);
  }

  leftMotor.run(leftDirection);
  rightMotor.run(rightDirection);
  currentLeftDirection = leftDirection;
  currentRightDirection = rightDirection;

  rampDriveSpeed(MOTOR_SPEED);
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
  setDriveState(BACKWARD, FORWARD);
}

void turnRight() {
  setDriveState(FORWARD, BACKWARD);
}

void moveForward() {
  setDriveState(BACKWARD, BACKWARD);
}

void stopMotors() {
  setDriveState(RELEASE, RELEASE);
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
  if (direction == FIRE_LEFT) return "FIRE_LEFT";
  if (direction == FIRE_CENTER) return "FIRE_CENTER";
  if (direction == FIRE_RIGHT) return "FIRE_RIGHT";
  return "NO_FIRE";
}
