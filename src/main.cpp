#include <Arduino.h>
#include <Enes100.h>

// put pin definitions here:
const int left_switch = 2;
const int right_switch = 4;
const int left_motor_forward = 3;
const int left_motor_backward = 5;
const int right_motor_forward = 6;
const int right_motor_backward = 9;
const int dist_sensor_trigs = 7;
const int dist_sensor_left_echo = 8;
const int dist_sensor_right_echo = 13;
const int ir_sensor_left = A0;
const int ir_sensor_right = A1;
const int fans = 10;
const int wifiModuleTX = 11;
const int wifiModuleRX = 12;

byte teamType = FIRE;
const char* teamName = "Phoenix";
// replace with Aruco Marker ID.
int markerId;
int roomNumber = 1120;


// put function declarations here:
// Movement functions
// speed should be in the range of 0-255, where 0 is stopped and 255 is full speed.
void moveForward(int speed, int duration);
void moveForward(int distance);
void moveBackward(int speed, int duration);
void turnLeft(int angle);
void turnRight(int angle);
int angleDifference(int from, int to);
int getAngle();

// IR function
void irSensorReadings();

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

  // IR sensors and fans
  pinMode(ir_sensor_left, INPUT);
  pinMode(ir_sensor_right, INPUT);
  pinMode(fans, OUTPUT);
  digitalWrite(fans, LOW); // Ensure fans are off at the start
}

void loop() {
  // Read IR sensors frequently
  irSensorReadings();
  delay(100);

  // Read localization / vision info and print
  float x, y, t; bool v;
  x = Enes100.getX();
  y = Enes100.getY();
  t = Enes100.getTheta();
  v = Enes100.isVisible();

  if (v) {
    Enes100.print(x);
    Enes100.print(",");
    Enes100.print(y);
    Enes100.print(",");
    Enes100.println(t);
  } else {
    Enes100.println("Not visible");
  }

  // Transmit mission data (example values)
  Enes100.mission(NUM_CANDLES, 4);
  Enes100.mission(TOPOGRAPHY, TOP_A);

  delay(1000);
}

void moveForward(int speed, int duration) {
  analogWrite(left_motor_forward, speed);
  analogWrite(right_motor_forward, speed);
  delay(duration);
  analogWrite(left_motor_forward, 0);
  analogWrite(right_motor_forward, 0);
}

// Move forward using distance (meters) instead of time and speed.
void moveForward(int distance) {
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

// Get a 0..359 degree heading from Enes100 theta (-PI..PI)
int getAngle() {
  float theta = Enes100.getTheta();
  if (theta == -1) return 0; // If not visible / not available, return 0 as fallback
  int deg = (int)(theta * 180.0 / PI);
  if (deg < 0) deg += 360;
  return deg;
}

void irSensorReadings(){
  int threshold = 500; // threshold value for flame detection, need to test and adjust accordingly

  // Code to detect if flames are present
  int leftFlame = analogRead(ir_sensor_left);
  int rightFlame = analogRead(ir_sensor_right);

  Serial.print("Left IR Sensor: ");
  Serial.print(leftFlame);
  Serial.print(" | Right IR Sensor: ");
  Serial.println(rightFlame);
  
  if (leftFlame > threshold && rightFlame > threshold){
    digitalWrite(fans, HIGH);
    Serial.println("Flame detected on both sides!");
  }
  else if (leftFlame > threshold){
    digitalWrite(fans, HIGH);
    Serial.println("Flame detected on the left side!");
  }
  else if (rightFlame > threshold){
    digitalWrite(fans, HIGH);
    Serial.println("Flame detected on the right side!");
  }
  else{
    digitalWrite(fans, LOW);
    Serial.println("No flames detected.");
  }
}

