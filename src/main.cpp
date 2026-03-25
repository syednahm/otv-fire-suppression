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

void setup() {
  // put your setup code here, to run once:
  int result = myFunction(2, 3);
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  Serial.println("Hello, World!");
}

// put function definitions here:
int myFunction(int x, int y) {
  return x + y;
}