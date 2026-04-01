#include "functions.h"



void setup() {
  // Start serial first for debug output
  Serial.begin(9600);

  // WiFi module pins
  pinMode(wifiModuleTX, OUTPUT);
  pinMode(wifiModuleRX, INPUT);

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

  // IR sensors and fans
  pinMode(ir_sensor_left, INPUT);
  pinMode(ir_sensor_right, INPUT);
  pinMode(fans, OUTPUT);
  digitalWrite(fans, LOW); // Ensure fans are off at the start

  // Distance sensor pins
  pinMode(dist_sensor_trigs, OUTPUT);
  pinMode(dist_sensor_left_echo, INPUT);
  pinMode(dist_sensor_right_echo, INPUT);
}


void loop() {

  if (topographyReached == 0) {
    Enes100.updateLocation();
    // Top starting point
    if (Enes100.location.y > 1.0) {
      turnToAngle(-90);
      float distanceToTop = calculateDistance(dist_sensor_trigs, dist_sensor_left_echo);
      const float SAFE_STOP_DISTANCE = 0.15; // stop 15cm before the top
      if (distanceToTop > SAFE_STOP_DISTANCE) {
        distanceToTop = calculateDistance(dist_sensor_trigs, dist_sensor_left_echo);
        moveForward(distanceToTop - SAFE_STOP_DISTANCE);
      }
    }
  }








  // Read localization / vision info and print
  // float x, y, t; bool v;
  // x = Enes100.getX();
  // y = Enes100.getY();
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
  analogWrite(left_motor_forward, speed);
  analogWrite(right_motor_forward, speed);
  delay(duration);
  analogWrite(left_motor_forward, 0);
  analogWrite(right_motor_forward, 0);
}

// Move forward using distance (meters) instead of time and speed.
void moveForward(float distance) {
  const int MOTOR_SPEED = 150;

  float startX = Enes100.getX();
  float startY = Enes100.getY();
  float angle = Enes100.getTheta(); // angle in radians

  // Calculate target position using trig
  float targetX = startX + distance * cos(angle);
  float targetY = startY + distance * sin(angle);

  // Turn on both motors
  analogWrite(left_motor_forward, MOTOR_SPEED);
  analogWrite(right_motor_forward, MOTOR_SPEED);

  // Keep moving until we reach the target position
  while (true) {
    float currentX = Enes100.getX();
    float currentY = Enes100.getY();

    // Calculate distance to target
    float distanceToTarget = sqrt(pow(targetX - currentX, 2) + pow(targetY - currentY, 2));

    // Stop when close enough (within 5cm tolerance)
    if (distanceToTarget < 0.05) {  // 0.05 meters = 5 cm
      break;
    }

    delay(50);  // Small delay between position checks
  }

  // Stop both motors
  analogWrite(left_motor_forward, 0);
  analogWrite(right_motor_forward, 0);
}

void moveBackward(int speed, int duration) {
  analogWrite(left_motor_backward, speed);
  analogWrite(right_motor_backward, speed);
  delay(duration);
  analogWrite(left_motor_backward, 0);
  analogWrite(right_motor_backward, 0);
}

void turnLeft(int angle) {
  int startAngle = getAngle();
  int targetRotation = angle;  // how much to rotate (degrees)
  int rotated = 0;

  // Drive left turn motors
  analogWrite(left_motor_backward, 150);
  analogWrite(right_motor_forward, 150);

  // Loop until rotated enough
  while (rotated < targetRotation) {
    int currentAngle = getAngle();
    rotated = angleDifference(startAngle, currentAngle);
    delay(10);
  }

  // Stop motors
  analogWrite(left_motor_backward, 0);
  analogWrite(right_motor_forward, 0);
}

void turnRight(int angle) {
  int startAngle = getAngle();
  int targetRotation = angle;
  int rotated = 0;

  analogWrite(left_motor_forward, 150);
  analogWrite(right_motor_backward, 150);

  while (rotated < targetRotation) {
    int currentAngle = getAngle();
    rotated = angleDifference(startAngle, currentAngle);
    delay(10);
  }

  analogWrite(left_motor_forward, 0);
  analogWrite(right_motor_backward, 0);
}

// Helper to compute the difference between two angles (degrees, always positive)
int angleDifference(int from, int to) {
  int diff = to - from;
  if (diff > 180) diff -= 360;
  if (diff < -180) diff += 360;
  return abs(diff);
}

// Get a -180 to 180 degree heading from Enes100 theta (-PI..PI)
int getAngle() {
  float theta = Enes100.getTheta();
  if (theta == -1) return 0; // If aruco marker is not visible, return 0 as fallback
  return (int)(theta * 180.0 / PI);
}

void irSensorReadings(){
  int threshold = 500; // threshold value for flame detection, need to test and adjust accordingly

  // Code to detect if flames are present
  int leftFlame = analogRead(ir_sensor_left);
  int rightFlame = analogRead(ir_sensor_right);

  Serial.print("Left IR Sensor: ");
  Serial.print(leftFlame);
  Serial.print("Right IR Sensor: ");
  Serial.println(rightFlame);
  
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
  digitalWrite(trigPin, HIGH);
  delay(10);
  digitalWrite(trigPin, LOW);
  
  duration = pulseIn(echoPin, HIGH);
  
  distance = duration * 0.0343 / 2.0; // Convert duration to distance in cm
  
  return distance;
}
