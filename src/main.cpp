#include "functions.h"

void setup() {
  // Start serial first for debug output
  Serial.begin(9600);

  pinMode(wifiRX, INPUT);
  pinMode(wifiTX, OUTPUT);
  // Initialize Enes100 (vision / localization)
  Enes100.begin(teamName, teamType, markerId, roomNumber, wifiTX, wifiRX);

  // At this point we know we are connected.
  
  Enes100.println("Connected to vision system!");

  // Motor pins
  pinMode(left_motor_forward, OUTPUT);
  pinMode(left_motor_backward, OUTPUT);
  pinMode(right_motor_forward, OUTPUT);
  pinMode(right_motor_backward, OUTPUT);
  pinMode(enableLeftMotor, OUTPUT);
  pinMode(enableRightMotor, OUTPUT);
  digitalWrite(left_motor_forward, LOW);
  digitalWrite(left_motor_backward, LOW);
  digitalWrite(right_motor_forward, LOW);
  digitalWrite(right_motor_backward, LOW);
  analogWrite(enableLeftMotor, 0);
  analogWrite(enableRightMotor, 0);

  // IR sensors and fans
  pinMode(ir_sensor_left, INPUT);
  pinMode(ir_sensor_right, INPUT);
  pinMode(fans, OUTPUT);
  digitalWrite(fans, LOW); // Ensure fans are off at the start

  // Distance sensor pins
  pinMode(dist_sensor_trigs, OUTPUT);
  pinMode(dist_sensor_left_echo, INPUT);
  pinMode(dist_sensor_right_echo, INPUT);

  // Switch pins for topography detection
  pinMode(mid_switch, INPUT_PULLUP);
  pinMode(right_switch, INPUT_PULLUP);
}


void loop() {

  if (topographyReached == 0) {
    globalFireCount = 1;
    // Top starting point
    Enes100.println(getCorrectY());
    Enes100.println(getCorrectTheta());
    if (getCorrectY() > 1.0) {
      turnToAngle(-90);
      float distanceToBottom = getCorrectY() - 0.70;
      const float SAFE_STOP_DISTANCE = 0.15; // stop 15cm before the top
      while (distanceToBottom > SAFE_STOP_DISTANCE) {
        moveForward(130, 500);
        distanceToBottom = getCorrectY() - 0.70;
      }
    } else {
      // Bottom starting point
      turnToAngle(90); // turn 90 degrees CCW to face the topography
      float distanceToTop = 1.30 - getCorrectY();
      const float SAFE_STOP_DISTANCE = 0.15; // stop 15cm before the bottom
      while (distanceToTop > SAFE_STOP_DISTANCE) {
        moveForward(130, 500);
        distanceToTop = 1.30 - getCorrectY();
      }
    }
    delay(500);
    moveBackward(130, 800);
    delay(500);
    irSensorReadings();
    delay (1000);
    moveForward(0.15);
    delay(1000);
    Enes100.println("Now checking second set of flames.");
    irSensorReadings();
    delay (1000);

    int tries = 0;
    while(topography == -1 && tries < 10) {
      if (tries != 0 && tries % 2 == 0){
        digitalWrite(left_motor_forward, HIGH);
        digitalWrite(right_motor_forward, LOW);
        digitalWrite(left_motor_backward, LOW);
        digitalWrite(right_motor_backward, HIGH);
        delay(100);
        stopMotors();
      } else if (tries != 0){
        digitalWrite(left_motor_forward, LOW);
        digitalWrite(right_motor_forward, HIGH);
        digitalWrite(left_motor_backward, HIGH);
        digitalWrite(right_motor_backward, LOW);
        delay(100);
        stopMotors();
      }
      digitalWrite(left_motor_forward, HIGH);
      digitalWrite(right_motor_forward, HIGH);
      digitalWrite(left_motor_backward, LOW);
      digitalWrite(right_motor_backward, LOW);
      analogWrite(enableLeftMotor, 130);
      analogWrite(enableRightMotor, 130);
      delay(300);
      topography = checkTopography();
      Enes100.println("Current topography: " + String(topography));
      delay(100);
      tries++;
      stopMotors();
      moveBackward(130, 250);
    }

    stopMotors();

    if (topography != -1) {
      Enes100.println("Topography detected: " + String(topography));
      Enes100.mission(TOPOGRAPHY, topography);
      topographyReached = 1;
      Enes100.mission(NUM_CANDLES, globalFireCount);
    } else {
      Serial.println("Error: Could not determine topography.");
      moveBackward(130, 5000);
    }
  }

  // First checks to see if old task is completed and also if new task is uncompleted.
  if (topographyReached == 1 && safeZoneReached == 0) {
    detectTopographyLocationAorB();
    moveBackward(130, 300);
    delay(500);
    if (topZone == 'A') { 
      turnToAngle(-90);
      correctToAngle(-90);
      float distanceToTravel = getCorrectY() - 1.1;
      moveForward(distanceToTravel);
      turnToAngle(0);
      correctToAngle(0);
      moveToEnd();
      safeZoneReached = 1;
    } else if (topZone == 'B') {
      turnToAngle(90);
      correctToAngle(90);
      float distanceToTravel = 0.9 - getCorrectY();
      moveForward(distanceToTravel);
      turnToAngle(0);
      correctToAngle(0);
      moveToEnd();
      safeZoneReached = 1;
    }
  }
}





// ---------------------------------------------------------------------------------
// Function definitions are below.
// ---------------------------------------------------------------------------------


void moveForward(int speed, int duration) {
  digitalWrite(left_motor_forward, HIGH);
  digitalWrite(right_motor_forward, HIGH);
  digitalWrite(left_motor_backward, LOW);
  digitalWrite(right_motor_backward, LOW);
  analogWrite(enableLeftMotor, speed);
  analogWrite(enableRightMotor, speed);
  delay(duration);
  stopMotors();
}

// Move forward using distance (meters) instead of time and speed.
void moveForward(float distance) {
  float startX = getCorrectX();
  float startY = getCorrectY();
  float angle = getCorrectTheta(); // angle in radians

  // Calculate target position using trig
  float targetX = startX + distance * cos(angle);
  float targetY = startY + distance * sin(angle);

  
  int tries = 0;
  // Keep moving until we reach the target position
  while (true) {
    float currentX = getCorrectX();
    float currentY = getCorrectY();
    // Turn on both motors
    digitalWrite(left_motor_forward, HIGH);
    digitalWrite(left_motor_backward, LOW);
    digitalWrite(right_motor_forward, HIGH);
    digitalWrite(right_motor_backward, LOW);
    analogWrite(enableLeftMotor,130);
    analogWrite(enableRightMotor, 130);

    // Calculate distance to target
    float distanceToTarget = sqrt(pow(targetX - currentX, 2) + pow(targetY - currentY, 2));

    // Stop when close enough (within 3cm tolerance)
    if (distanceToTarget < 0.03 || (distanceToTarget < 0.5 && tries == 10)) {  // 0.03 meters = 3 cm
      break;
    }

    delay(100);  // Small delay between position checks
    stopMotors();
    delay(100);
    tries++;
  }

  // Stop both motors
  stopMotors();
}


void moveBackward(int speed, int duration) {
  digitalWrite(left_motor_backward, HIGH);
  digitalWrite(left_motor_forward, LOW);
  digitalWrite(right_motor_backward, HIGH);
  digitalWrite(right_motor_forward, LOW);
  analogWrite(enableLeftMotor, speed);
  analogWrite(enableRightMotor, speed);
  delay(duration);
  stopMotors();
}

void turnLeft(float angle) {
    float stopEarly = 1.0;
    float initialAngle = getAngle();
    float distance = 0.0;
    while (distance < angle - stopEarly) {
      digitalWrite(left_motor_forward, LOW);
      digitalWrite(left_motor_backward, HIGH);
      digitalWrite(right_motor_backward, LOW);
      digitalWrite(right_motor_forward, HIGH);
      analogWrite(enableLeftMotor, 130);
      analogWrite(enableRightMotor, 130);
      delay(50);
      float currAngle = getAngle();
      distance += abs(normalizedAngleDiff(currAngle, initialAngle));
      initialAngle = currAngle;
      stopMotors();
      delay(100);
    }
}

void turnRight(float angle) {
    float stopEarly = 1.0;

    float initialAngle = getAngle();
    float distance = 0.0;

    while (distance < angle - stopEarly) {
      digitalWrite(left_motor_forward, HIGH);
      digitalWrite(left_motor_backward, LOW);
      digitalWrite(right_motor_backward, HIGH);
      digitalWrite(right_motor_forward, LOW);
      analogWrite(enableLeftMotor, 130);
      analogWrite(enableRightMotor, 130);
      delay(50);
      float currAngle = getAngle();
      distance += abs(normalizedAngleDiff(initialAngle, currAngle));
      initialAngle = currAngle;
      stopMotors();
      delay(100);
    }
}

void turnToAngle(float targetAngle) {
    float currAng = getAngle();
    float diff = normalizedAngleDiff(currAng, targetAngle);

    if (diff < 0) {
        turnRight(abs(diff)); // pass the absolute target directly
    } else {
        turnLeft(abs(diff));  // pass the absolute target directly
    }
}

float normalizedAngleDiff(float from, float to) {
  float diff = to - from;
  if (diff > 180) diff -= 360;
  if (diff < -180) diff += 360;
  return diff;
}

// Get a -180 to 180 degree heading from Enes100 theta (-PI..PI)
float getAngle() {
  float theta = getCorrectTheta(); // get the latest theta value
  return theta * 180.0 / PI;
}

void irSensorReadings(){
  int threshold = 700; // threshold value for flame detection, need to test and adjust accordingly

  // Code to detect if flames are present
  int leftFlame = averageIRRead(ir_sensor_left);
  int rightFlame = averageIRRead(ir_sensor_right);

  Enes100.print("Left IR Sensor: ");
  Enes100.println(leftFlame);
  Enes100.print("Right IR Sensor: ");
  Enes100.println(rightFlame);
  
  if (leftFlame < threshold && rightFlame < threshold) {
    digitalWrite(fans, HIGH);
    //turnRight(7);
    //turnLeft(7);
    Enes100.println("Flame detected on both sides!");
    delay(3000);
    digitalWrite(fans, LOW);
    globalFireCount+=2;

  } else if (leftFlame < threshold || rightFlame < threshold) {
    digitalWrite(fans, HIGH);
    //turnRight(7);
    //turnLeft(7);
    Enes100.println("Flame detected on the right or the left side!");
    delay(3000);
    digitalWrite(fans, LOW);
    globalFireCount++;
  } else {
    digitalWrite(fans, LOW);
    Enes100.println("No flames detected.");
  }
}

int averageIRRead(int pin, int samples) {
    long sum = 0;
    for (int i = 0; i < samples; i++) {
        sum += analogRead(pin);
        delay(40);
    }
    return sum / samples;
}

float calculateDistance(int trigPin, int echoPin) {
  int duration;
  float distance;
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  duration = pulseIn(echoPin, HIGH);
  
  distance = duration * 0.0343 / 2.0; // Convert duration to distance in cm
  
  return distance;
}

void correctToAngle(float targetAngle, int maxAttempts) {
    for (int i = 0; i < maxAttempts; i++) {
      delay(200); // let the OTV fully settle before checking
      float currAngle = getAngle();
      float diff = normalizedAngleDiff(currAngle, targetAngle);

      if (abs(diff) <= 2.0) break; // within tolerance, good enough

      // Make a small correction
      if (diff > 0) {
          turnLeft(abs(diff));
      } else {
          turnRight(abs(diff));
      }
    }
}

int checkTopography() {
  bool midPressed = digitalRead(mid_switch) == LOW; // Assuming active LOW
  bool rightPressed = digitalRead(right_switch) == LOW;

  if (midPressed && rightPressed) {
    Enes100.println("Topography: Both switches pressed");
    return TOP_C; // Both
  } else if (midPressed) {
    Enes100.println("Topography: Mid switch pressed");
    return TOP_A; // Mid
  } else if (rightPressed) {
    Enes100.println("Topography: Right switch pressed");
    return TOP_B; // Right
  } else {
    Enes100.println("Topography: No switches pressed");
    return -1; // None
  }
}


void detectTopographyLocationAorB(){
  float y = getCorrectY();
  if (y > 1.0) { //need to adjust after testing
    topZone = 'A';
    Enes100.println("Topography in Zone A");
  } else {
    topZone = 'B';
    Enes100.println("Topography in Zone B");
  }
}

void moveToEnd() {
  while (getCorrectX() < 2.8) {
    while (getCorrectX() < 2.8 && calculateDistance(dist_sensor_trigs, dist_sensor_left_echo) > 21.0) { // Move forward until 21 cm from the wall
      digitalWrite(left_motor_forward, HIGH);
      digitalWrite(right_motor_forward, HIGH);
      digitalWrite(left_motor_backward, LOW);
      digitalWrite(right_motor_backward, LOW);
      analogWrite(enableLeftMotor, 130);
      analogWrite(enableRightMotor, 130);
      delay(300);
      stopMotors();
      delay(500);
    }

    if (getCorrectX() >= 2.8) {
      break; // Reached the end
    }

    if (!(getCorrectY() < 0.8) && isRightFree()) {
      correctToAngle(0);
    } else if (!(getCorrectY() > 0.8) && isLeftFree()) {
      correctToAngle(0);
    } else {
      turnToAngle(90);
      float distToTravel = 1.1 - getCorrectY();
      moveForward(distToTravel);
      turnToAngle(0);
    }
  }

  if (getCorrectY() < 1.3) {
    turnToAngle(90);
    correctToAngle(90);
    while (getCorrectY() < 1.5) {
      moveForward(130, 100);
      delay(150);
    }
    turnToAngle(0);
  } else {
    correctToAngle(0);
  }

  moveForward(1.0);
}

bool isRightFree() {
  turnToAngle(-90);
  float distanceToTravel = getCorrectY() - 0.35;
  moveForward(distanceToTravel);
  turnToAngle(0);
  if (calculateDistance(dist_sensor_trigs, dist_sensor_left_echo) > 21.0) { // if more than 21 cm on the right, consider it free
    return true;
  } else {
    return false;
  }
}

bool isLeftFree() {
  turnToAngle(90);
  float distanceToTravel = 1.6 - getCorrectY();
  moveForward(distanceToTravel);
  turnToAngle(0);
  if (calculateDistance(dist_sensor_trigs, dist_sensor_left_echo) > 21.0) { // if more than 21 cm on the left, consider it free
    return true;
  } else {
    return false;
  }
}


float getCorrectX() {
  float x = Enes100.getX();
  while (x == -1) {
    x = Enes100.getX();
    delay(50);
  }
  return x;
}

float getCorrectY() {
  float y = Enes100.getY();
  while (y == -1) {
    y = Enes100.getY();
    delay(50);
  }
  return y;
}

float getCorrectTheta() {
  float theta = Enes100.getTheta();
  while (theta == -1) {
    theta = Enes100.getTheta();
    delay(50);
  }
  return theta;
}

void stopMotors() {
  digitalWrite(left_motor_forward, LOW);
  digitalWrite(left_motor_backward, LOW);
  digitalWrite(right_motor_forward, LOW);
  digitalWrite(right_motor_backward, LOW);
  analogWrite(enableLeftMotor, 0);
  analogWrite(enableRightMotor, 0);
}