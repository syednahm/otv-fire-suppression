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

void setup() {
    Enes100.begin("Phoenix ", FIRE, markerId, 1120, 11, 12);
    // At this point we know we are connected.
    Enes100.println("Connected...");
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(left_motor_forward, OUTPUT);
  pinMode(left_motor_backward, OUTPUT);
  pinMode(right_motor_forward, OUTPUT);
  pinMode(right_motor_backward, OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  
   float x, y, t; bool v; // Declare variables to hold the data
    //Enes100.getX will make sure you get the latest data available to you about your OTV's location.
    //The first time getX is called, X, Y, theta and visibility are queried and cached.
    //Subsequent calls return from the cache, so there is no performance gain to saving the function response to a variable.

    x = Enes100.getX();  // Your X coordinate! 0-4, in meters, -1 if no aruco is not visibility (but you should use Enes100.isVisible to check that instead)
    y = Enes100.getY();  // Your Y coordinate! 0-2, in meters, also -1 if your aruco is not visible.
    t = Enes100.getTheta();  //Your theta! -pi to +pi, in radians, -1 if your aruco is not visible.
    v = Enes100.isVisible(); // Is your aruco visible? True or False.

    if (v) // If the ArUco marker is visible
    {
        Enes100.print(x); // print out the location
        Enes100.print(",");
        Enes100.print(y);
        Enes100.print(",");
        Enes100.println(t);
    }
    else { // otherwise
        Enes100.println("Not visible"); // print not visible
    }

    // Transmit the number of candles that are lit
    Enes100.mission(NUM_CANDLES, 4);
    // Transmit the topography of the fire mission (TOP_A for topography A)
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

// Move forward using distance instead of time and speed.
void moveForward(int distance) {
  const int MOTOR_SPEED = 150;
  
  float startX = Enes100.getX();
  float startY = Enes100.getY();
  float angle = Enes100.getTheta();
  
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
  int targetRotation = angle;  // how much to rotate
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

// Helper to compute the difference between two angles (always positive)
int angleDifference(int from, int to) {
  int diff = to - from;
  if (diff > 180) diff -= 360;
  if (diff < -180) diff += 360;
  return abs(diff);
}