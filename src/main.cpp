#include <Arduino.h>

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
int myFunction(int, int);

// Movement functions
void moveForward(int speed, int duration);
void moveBackward(int speed, int duration);
void turnLeft(int speed, int duration);
void turnRight(int speed, int duration);

//IR function 
void irSensorReadings();

void setup() {
  // put your setup code here, to run once:
  int result = myFunction(2, 3);
  Serial.begin(9600);

  pinMode(ir_sensor_left, INPUT);
  pinMode(ir_sensor_right, INPUT);
  pinMode(fans, OUTPUT);

  digitalWrite(fans, LOW); // Ensure fans are off at the start
}

void loop() {
  irSensorReadings();
  delay(100);
  // put your main code here, to run repeatedly:
  Serial.println("Hello, World!");
}

// put function definitions here:
int myFunction(int x, int y) {
  return x + y;
}

void moveForward(int speed, int duration) {
  // Code to move the robot forward at the specified speed and duration


}

void moveBackward(int speed, int duration) {
  // Code to move the robot backward at the specified speed and duration
}

void turnLeft(int speed, int duration) {
  // Code to turn the robot left at the specified speed and duration
}

void turnRight(int speed, int duration) {
  // Code to turn the robot right at the specified speed and duration
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
  
  if (leftFlame > threshold && rightFlame >threshold){
    digitalWrite(fans,HIGH);
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
    digitalWrite(fans,LOW);
    Serial.println("No flames detected.");
}
  
  
}