#include "functions.h"

void setup() {
  // Start serial first for debug output
  Serial.begin(9600);

  // Initialize Enes100 (vision / localization)
  Enes100.begin(teamName, teamType, markerId, roomNumber, wifiModuleTX, wifiModuleRX);

  // At this point we know we are connected.
  if (Enes100.isConnected()) {
    Serial.println("Connected to vision system!");
  } else {
    Serial.println("Failed to connect to vision system.");
  }

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
    Enes100.updateLocation();
    // Top starting point
    if (Enes100.location.y > 1.0) {
      turnToAngle(-90);
      float distanceToTop = calculateDistance(dist_sensor_trigs, dist_sensor_left_echo);
      const float SAFE_STOP_DISTANCE = 0.15; // stop 15cm before the top
      while (distanceToTop > SAFE_STOP_DISTANCE) {
        distanceToTop = calculateDistance(dist_sensor_trigs, dist_sensor_left_echo);
        moveForward(50, 100);
        turnToAngle(-90);
      }
      turnToAngle(-90);
    } else {
      // Bottom starting point
      turnToAngle(90);
      float distanceToBottom = calculateDistance(dist_sensor_trigs, dist_sensor_right_echo);
      const float SAFE_STOP_DISTANCE = 0.15; // stop 15cm before the bottom
      while (distanceToBottom > SAFE_STOP_DISTANCE) {
        distanceToBottom = calculateDistance(dist_sensor_trigs, dist_sensor_right_echo);
        moveForward(50, 100);
        turnToAngle(90);
      }
      turnToAngle(90);
    }

    float leftDistance = calculateDistance(dist_sensor_trigs, dist_sensor_left_echo);
    float rightDistance = calculateDistance(dist_sensor_trigs, dist_sensor_right_echo);
    while (abs(leftDistance - rightDistance) > 0.01) { // 1 cm threshold for correction
      correctAngle(leftDistance, rightDistance);
      leftDistance = calculateDistance(dist_sensor_trigs, dist_sensor_left_echo);
      rightDistance = calculateDistance(dist_sensor_trigs, dist_sensor_right_echo);
    }

    irSensorReadings();
    moveForward(0.15);
    irSensorReadings();
    topography = checkTopography();
    
    if (topography != -1) {
      Enes100.mission(TOPOGRAPHY, topography);
      topographyReached = 1;
      Enes100.mission(NUM_CANDLES, globalFireCount);
    } else {
      Serial.println("Error: Could not determine topography.");
      moveBackward(150, 1000);
    }
    
  }

  // First checks to see if old task is completed and also if new task is uncompleted.
  if (topographyReached == 1 && safeZoneReached == 0) {
    detectTopographyLocationAorB();

    if (topZone == 'A') { 
      navigateToEndZoneWhenTopAtA();
      safeZoneReached = 1;
    } else if (topZone == 'B') {
      navigateToEndZoneWhenTopAtB();
      safeZoneReached = 1;
    }
  }







  // Read localization / vision info and print
  // float x, y, t; bool v;
  // x = Enes100.getX();
  // int y = Enes100.getY();
  // t = Enes100.getTheta();
  // v = Enes100.isVisible();

  // if (v) {
  //   Enes100.print(x);
  //   Enes100.print(",");
  //   Enes100.print(y);
  //   Enes100.print(",");
  //   Enes100.println(t);
  // } else {
  //   Enes100.println("Not visible");
  // }

  // // Transmit mission data (example values)
  // Enes100.mission(NUM_CANDLES, 4);
  // Enes100.mission(TOPOGRAPHY, TOP_A);

  // delay(1000);
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
  digitalWrite(left_motor_forward, LOW);
  digitalWrite(right_motor_forward, LOW);
  analogWrite(enableLeftMotor, 0);
  analogWrite(enableRightMotor, 0);
}

// Move forward using distance (meters) instead of time and speed.
// MAY need to change this to rely on timing and speed.
void moveForward(float distance) {
  float startX = Enes100.getX();
  float startY = Enes100.getY();
  float angle = Enes100.getTheta(); // angle in radians

  // Calculate target position using trig
  float targetX = startX + distance * cos(angle);
  float targetY = startY + distance * sin(angle);

  // Turn on both motors
  digitalWrite(left_motor_forward, HIGH);
  digitalWrite(left_motor_backward, LOW);
  digitalWrite(right_motor_forward, HIGH);
  digitalWrite(right_motor_backward, LOW);
  analogWrite(enableLeftMotor, 150);
  analogWrite(enableRightMotor, 150);

  // Keep moving until we reach the target position
  while (true) {
    float currentX = Enes100.getX();
    float currentY = Enes100.getY();

    // Calculate distance to target
    float distanceToTarget = sqrt(pow(targetX - currentX, 2) + pow(targetY - currentY, 2));

    // Stop when close enough (within 3cm tolerance)
    if (distanceToTarget < 0.03) {  // 0.03 meters = 3 cm
      break;
    }

    delay(50);  // Small delay between position checks
  }

  // Stop both motors
  digitalWrite(left_motor_forward, LOW);
  digitalWrite(right_motor_forward, LOW);
  analogWrite(enableLeftMotor, 0);
  analogWrite(enableRightMotor, 0);
}

void moveBackward(int speed, int duration) {
  digitalWrite(left_motor_backward, HIGH);
  digitalWrite(left_motor_forward, LOW);
  digitalWrite(right_motor_backward, HIGH);
  digitalWrite(right_motor_forward, LOW);
  analogWrite(enableLeftMotor, speed);
  analogWrite(enableRightMotor, speed);
  delay(duration);
  digitalWrite(left_motor_backward, LOW);
  digitalWrite(right_motor_backward, LOW);
  analogWrite(enableLeftMotor, 0);
  analogWrite(enableRightMotor, 0);
}

void turnLeft(float angle) {
  float prevAngle = getAngle();
  float targetRotation = angle;  // how much to rotate (degrees)
  float rotated = 0;

  // Drive left turn motors
  digitalWrite(left_motor_backward, HIGH);
  digitalWrite(left_motor_forward, LOW);
  digitalWrite(right_motor_forward, HIGH);
  digitalWrite(right_motor_backward, LOW);
  analogWrite(enableLeftMotor, 50);
  analogWrite(enableRightMotor, 50);

  // Loop until rotated enough
  while (rotated < targetRotation) {
    delay(10);
    float currentAngle = getAngle();
    float delta = prevAngle - currentAngle;
    if (delta > 180) delta -= 360;
    if (delta < -180) delta += 360;
    rotated += abs(delta);
    prevAngle = currentAngle;
  }

  // Stop motors
  digitalWrite(left_motor_backward, LOW);
  digitalWrite(right_motor_forward, LOW);
  analogWrite(enableLeftMotor, 0);
  analogWrite(enableRightMotor, 0);

}

void turnRight(float angle) {
  float prevAngle = getAngle();
  float targetRotation = angle;
  float rotated = 0;

  digitalWrite(left_motor_forward, HIGH);
  digitalWrite(left_motor_backward, LOW);
  digitalWrite(right_motor_backward, HIGH);
  digitalWrite(right_motor_forward, LOW);

  analogWrite(enableLeftMotor, 50);
  analogWrite(enableRightMotor, 50);

  while (rotated < targetRotation) {
    delay(10);
    float currentAngle = getAngle();
    float delta = currentAngle - prevAngle;
    if (delta > 180) delta -= 360;
    if (delta < -180) delta += 360;
    rotated += abs(delta);
    prevAngle = currentAngle;
  }

  digitalWrite(left_motor_forward, LOW);
  digitalWrite(right_motor_backward, LOW);
  analogWrite(enableLeftMotor, 0);
  analogWrite(enableRightMotor, 0);
}

void turnToAngle(float angle) {
  float currAng = getAngle();
  float diff = normalizedAngleDiff(currAng, angle);
  
  if (diff < 0) {
    turnRight(abs(diff));
  } else {
    turnLeft(abs(diff));
  }
}

float normalizedAngleDiff(float from, float to) {
  float diff = to - from;
  if (diff > 180) diff -= 360;
  if (diff < -180) diff += 360;
  return diff;
}

// Helper to compute the difference between two angles (degrees, always positive)
float angleDifference(float from, float to) {
  float diff = to - from;
  if (diff > 180) diff -= 360;
  if (diff < -180) diff += 360;
  return abs(diff);
}

// Get a -180 to 180 degree heading from Enes100 theta (-PI..PI)
float getAngle() {
  if (!Enes100.isVisible()) return 0; // Or handle appropriately
  float theta = Enes100.getTheta();
  return theta * 180.0 / PI;
}

void irSensorReadings(){
  int threshold = 500; // threshold value for flame detection, need to test and adjust accordingly

  // Code to detect if flames are present
  int leftFlame = analogRead(ir_sensor_left);
  int rightFlame = analogRead(ir_sensor_right);

  Serial.println("Left IR Sensor: ");
  Serial.print(leftFlame);
  Serial.println("Right IR Sensor: ");
  Serial.print(rightFlame);
  
  if (leftFlame > threshold && rightFlame > threshold){
    digitalWrite(fans, HIGH);
    Serial.println("Flame detected on both sides!");
    globalFireCount+=2;
  }
  else if (leftFlame > threshold || rightFlame > threshold){
    digitalWrite(fans, HIGH);
    Serial.println("Flame detected on the right or the left side!");
    globalFireCount++;
  }
  else{
    digitalWrite(fans, LOW);
    Serial.println("No flames detected.");
  }
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
    turnLeft(1);
  } else {
    // Too close to the topography, turn slightly right
    turnRight(1);
  }
}

int checkTopography() {
  bool midPressed = digitalRead(mid_switch) == LOW; // Assuming active LOW
  bool rightPressed = digitalRead(right_switch) == LOW;

  if (midPressed && rightPressed) {
    Serial.println("Topography: Both switches pressed");
    return TOP_C; // Both
  } else if (midPressed) {
    Serial.println("Topography: Mid switch pressed");
    return TOP_A; // Mid
  } else if (rightPressed) {
    Serial.println("Topography: Right switch pressed");
    return TOP_B; // Right
  } else {
    Serial.println("Topography: No switches pressed");
    return -1; // None
  }
}


void detectTopographyLocationAorB(){
  float y = Enes100.getY();
  if (y > 1.0) { //need to adjust after testing
    topZone = 'A';
    Serial.println("Topography in Zone A");
  } else {
    topZone = 'B';
    Serial.println("Topography in Zone B");
  }
}

//void navigateToEndZoneWhenTopAtB()
void navigateToEndZoneWhenTopAtB(){
  moveBackward(150,150); // move backward to clear the topograpghy area
  turnRight(90); // turn 90 degrees CW and face right (need to test)
  moveForward(150,900); //move forward to leave mission area
  turnLeft(90); // turn 90 degrees CCW and face right (need to test)

  //otv will be facing the wall and will move until its 3cm from the wall
  while(calculateDistance(dist_sensor_trigs, dist_sensor_left_echo)>0.03) {
    analogWrite(left_motor_forward, 150);
    analogWrite(right_motor_forward, 150);
    delay(50);
  }
  analogWrite(left_motor_forward, 150);
  analogWrite(right_motor_forward, 150);

  turnRight(90); //turn 90 degrees CW and face direction of end zone
  moveForward(150, 1000); //move forward into end zone and under the arch, may need to adjust when testing

  Serial.println("Reached end zone from topography location B");
}

//navigating to end zone from top location A
void navigateToEndZoneWhenTopAtA(){
  moveBackward(150, 500); //move backward to clear the topography area
  turnRight(90); //turn 90 degrees clockwise and face right
  moveForward(150,900); //move forward to leave landing/mission zone
  turnLeft(90); //turn 90 degrees CCW and face up

  //otv moves ahead until its 3 cm from the wall, 
  //will probably have to adjust the 3 cm threshold depending on how wide the turn is
  while(calculateDistance(dist_sensor_trigs, dist_sensor_left_echo)>0.03) {
    analogWrite(left_motor_forward, 150);
    analogWrite(right_motor_forward, 150);
    turnToAngle(90);
    delay(50);
  }

  analogWrite(left_motor_forward, 0);
  analogWrite(right_motor_forward, 0);

  turnRight(90); //turn 90 degrees clockwise and face direction of end zone

  moveForward(150, 1000); //move forward into end zone, may need to adjust after testing

  analogWrite(left_motor_forward, 0);
  analogWrite(right_motor_forward, 0);

  Serial.println("Reached end zone from topography location A");
}