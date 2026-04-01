#include <Arduino.h>
#include <Enes100.h>

// put pin definitions here:
const int mid_switch = 2;
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


// Global Variables
short topographyReached = 0;
short safeZoneReached = 0;
int topography;
int globalFireCount = 1;

// Team info for Enes100
byte teamType = FIRE;
const char* teamName = "Phoenix";
// replace with Aruco Marker ID.
int markerId;
int roomNumber = 1120;

// put function declarations here:
// Movement functions
// speed should be in the range of 0-255, where 0 is stopped and 255 is full speed.
void moveForward(int speed, int duration);
void moveForward(float distance);
void moveBackward(int speed, int duration);
void turnLeft(int angle);
void turnRight(int angle);
void turnToAngle(int angle);
int angleDifference(int from, int to);
int normalizedAngleDiff(int from, int to);
float getAngle();

// IR function
void irSensorReadings();

int checkTopgraphy();