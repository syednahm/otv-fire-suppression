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
      correctToAngle(-90);
      float distanceToBottom = getCorrectY() - 0.70;
      const float SAFE_STOP_DISTANCE = 0.15; // stop 15cm before the top
      while (distanceToBottom > SAFE_STOP_DISTANCE) {
        moveForward(130, 500);
        distanceToBottom = getCorrectY() - 0.70;
      }
    } else {
      // Bottom starting point
      turnToAngle(90); // turn 90 degrees CCW to face the topography
      correctToAngle(90);
      float distanceToTop = 1.30 - getCorrectY();
      const float SAFE_STOP_DISTANCE = 0.15; // stop 15cm before the bottom
      while (distanceToTop > SAFE_STOP_DISTANCE) {
        moveForward(130, 500);
        distanceToTop = 1.30 - getCorrectY();
      }
    }

    float leftDistance = calculateDistance(dist_sensor_trigs, dist_sensor_left_echo);
    float rightDistance = calculateDistance(dist_sensor_trigs, dist_sensor_right_echo);
    while (abs(leftDistance - rightDistance) > 5.0) { // 5 cm threshold for correction
      correctAngle(leftDistance, rightDistance);
      moveForward(130, 200);
      moveBackward(130, 200);
      leftDistance = calculateDistance(dist_sensor_trigs, dist_sensor_left_echo);
      rightDistance = calculateDistance(dist_sensor_trigs, dist_sensor_right_echo);
    }



    moveBackward(130, 800);
    irSensorReadings();
    moveForward(0.15);
    irSensorReadings();

    int tries = 0;
    while(topography == -1 && tries < 10) {
      digitalWrite(left_motor_forward, HIGH);
      digitalWrite(right_motor_forward, HIGH);
      digitalWrite(left_motor_backward, LOW);
      digitalWrite(right_motor_backward, LOW);
      analogWrite(enableLeftMotor, 130);
      analogWrite(enableRightMotor, 130);
      topography = checkTopography();
      Enes100.println("Current topography: " + String(topography));
      delay(100);
      tries++;
    }

    stopMotors();

    if (topography != -1) {
      Enes100.println("Topography detected: " + String(topography));
      Enes100.mission(TOPOGRAPHY, topography);
      topographyReached = 1;
      Enes100.mission(NUM_CANDLES, globalFireCount);
    } else {
      Serial.println("Error: Could not determine topography.");
      moveBackward(130, 3000);
    }
  }

  // First checks to see if old task is completed and also if new task is uncompleted.
  if (topographyReached == 1 && safeZoneReached == 0) {
    detectTopographyLocationAorB();


    if (topZone == 'A') { 
      turnToAngle(-90);
      correctToAngle(-90);
      float distanceToTravel = Enes100.getY() - 1.0;
      moveForward(distanceToTravel);
      turnToAngle(0);
      correctToAngle(0);
      moveToEnd();
      safeZoneReached = 1;
    } else if (topZone == 'B') {
      turnToAngle(90);
      correctToAngle(90);
      float distanceToTravel = 1.0 - Enes100.getY();
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
// MAY need to change this to rely on timing and speed.
void moveForward(float distance) {
  float startX = getCorrectX();
  float startY = getCorrectY();
  float angle = getCorrectTheta(); // angle in radians

  // Calculate target position using trig
  float targetX = startX + distance * cos(angle);
  float targetY = startY + distance * sin(angle);

  // Turn on both motors
  digitalWrite(left_motor_forward, HIGH);
  digitalWrite(left_motor_backward, LOW);
  digitalWrite(right_motor_forward, HIGH);
  digitalWrite(right_motor_backward, LOW);
  analogWrite(enableLeftMotor,130);
  analogWrite(enableRightMotor, 130);

  // Keep moving until we reach the target position
  while (true) {
    float currentX = getCorrectX();
    float currentY = getCorrectY();

    // Calculate distance to target
    float distanceToTarget = sqrt(pow(targetX - currentX, 2) + pow(targetY - currentY, 2));

    // Stop when close enough (within 3cm tolerance)
    if (distanceToTarget < 0.03) {  // 0.03 meters = 3 cm
      break;
    }

    delay(50);  // Small delay between position checks
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

void turnLeft(float targetAngle) {
    float stopEarly = 7.5;

    digitalWrite(left_motor_backward, HIGH);
    digitalWrite(left_motor_forward, LOW);
    digitalWrite(right_motor_forward, HIGH);
    digitalWrite(right_motor_backward, LOW);
    analogWrite(enableLeftMotor, 130);
    analogWrite(enableRightMotor, 130);

    while (normalizedAngleDiff(getAngle(), targetAngle) > stopEarly) {
        delay(10);
    }

    stopMotors();
}

void turnRight(float targetAngle) {
    float stopEarly = 7.5;

    digitalWrite(left_motor_forward, HIGH);
    digitalWrite(left_motor_backward, LOW);
    digitalWrite(right_motor_backward, HIGH);
    digitalWrite(right_motor_forward, LOW);
    analogWrite(enableLeftMotor, 130);
    analogWrite(enableRightMotor, 130);

    while (normalizedAngleDiff(targetAngle, getAngle()) > stopEarly) {
        delay(10);
    }

    stopMotors();
}

void turnToAngle(float targetAngle) {
    float currAng = getAngle();
    float diff = normalizedAngleDiff(currAng, targetAngle);

    if (diff < 0) {
        turnRight(targetAngle); // pass the absolute target directly
    } else {
        turnLeft(targetAngle);  // pass the absolute target directly
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
  int threshold = 500; // threshold value for flame detection, need to test and adjust accordingly

  // Code to detect if flames are present
  int leftFlame = averageIRRead(ir_sensor_left);
  int rightFlame = averageIRRead(ir_sensor_right);

  Enes100.println("Left IR Sensor: ");
  Enes100.print(leftFlame);
  Enes100.println("Right IR Sensor: ");
  Enes100.print(rightFlame);
  
  if (leftFlame < threshold && rightFlame < threshold){
    digitalWrite(fans, HIGH);
    Enes100.println("Flame detected on both sides!");
    delay(2000);
    digitalWrite(fans, LOW);
    globalFireCount+=2;
  }
  else if (leftFlame < threshold || rightFlame < threshold){
    digitalWrite(fans, HIGH);
    delay(2000);
    digitalWrite(fans, LOW);
    Enes100.println("Flame detected on the right or the left side!");
    globalFireCount++;
  }
  else{
    digitalWrite(fans, LOW);
    Enes100.println("No flames detected.");
  }
}

int averageIRRead(int pin, int samples) {
    long sum = 0;
    for (int i = 0; i < samples; i++) {
        sum += analogRead(pin);
        delay(2);
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

void correctAngle(float leftDistance, float rightDistance) {
  if (leftDistance < rightDistance) {
    // Too close to the topography, turn slightly left
    turnLeft(2);
  } else {
    // Too close to the topography, turn slightly right
    turnRight(2);
  }
}

void correctToAngle(float targetAngle, int maxAttempts) {
    for (int i = 0; i < maxAttempts; i++) {
        delay(200); // let the OTV fully settle before checking
        float currAngle = getAngle();
        float diff = normalizedAngleDiff(currAngle, targetAngle);

        if (abs(diff) <= 10.0) break; // within tolerance, good enough

        // Make a small correction
        if (diff > 0) {
            turnLeft(targetAngle);
        } else {
            turnRight(targetAngle);
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
    while (getCorrectX() < 2.8 && calculateDistance(dist_sensor_trigs, dist_sensor_left_echo) > 4.0) { // Move forward until 4 cm from the wall
      digitalWrite(left_motor_forward, HIGH);
      digitalWrite(right_motor_forward, HIGH);
      digitalWrite(left_motor_backward, LOW);
      digitalWrite(right_motor_backward, LOW);
      analogWrite(enableLeftMotor, 130);
      analogWrite(enableRightMotor, 130);
      correctToAngle(0); // keep facing the wall
      delay(50);
    }

    stopMotors();
    if (isRightFree()) {
      correctToAngle(0);
    } else if (isLeftFree()) {
      correctToAngle(0);
    }
  }

  if (getCorrectY() < 1.0) {
    turnToAngle(90);
    correctToAngle(90);
    while (getCorrectY() < 1.5) {
      moveForward(130, 100);
      turnToAngle(90);
      delay(50);
    }
    turnToAngle(0);
    correctToAngle(0);
  } else {
    correctToAngle(0);
  }

  moveForward(1.0);
}

bool isRightFree() {
  turnRight(90);
  float distanceToTravel = getCorrectY() - 0.5;
  moveForward(distanceToTravel);
  turnLeft(90);
  if (calculateDistance(dist_sensor_trigs, dist_sensor_left_echo) > 15.0) { // if more than 15 cm on the right, consider it free
    return true;
  } else {
    return false;
  }
}

bool isLeftFree() {
  turnLeft(90);
  float distanceToTravel = 1.5 - getCorrectY();
  moveForward(distanceToTravel);
  turnRight(90);
  if (calculateDistance(dist_sensor_trigs, dist_sensor_left_echo) > 15.0) { // if more than 15 cm on the left, consider it free
    return true;
  } else {
    return false;
  }
}


float getCorrectX() {
  float x = Enes100.getX();
  while (x == -1) {
    x = Enes100.getX();
    delay(100);
  }
  return x;
}

float getCorrectY() {
  float y = Enes100.getY();
  while (y == -1) {
    y = Enes100.getY();
    delay(100);
  }
  return y;
}

float getCorrectTheta() {
  float theta = Enes100.getTheta();
  while (theta == -1) {
    theta = Enes100.getTheta();
    delay(100);
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