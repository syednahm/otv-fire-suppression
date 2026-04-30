#include <Arduino.h>
#include <Enes100.h>

// put pin definitions here:
const int mid_switch = A2;
const int right_switch = A3;
const int left_motor_forward = 4;
const int left_motor_backward = 3;
const int right_motor_forward = 6;
const int right_motor_backward = 5;
const int enableLeftMotor = 2;
const int enableRightMotor = 7;
const int dist_sensor_trigs = 10;
const int dist_sensor_left_echo = 8;
const int dist_sensor_right_echo = 9;
const int ir_sensor_left = A0;
const int ir_sensor_right = A1;
const int fans = 13;
const int wifiTX = 11;
const int wifiRX = 12;
// changes pins, 2,3,4,5,7 pins for the motors to move OTV/ and then rechanged them back to orginal pins to test if that is the issue

// Global Variables
short topographyReached = 0;
short safeZoneReached = 0;
int topography = -1;
int globalFireCount = 1;
char topZone = 'X';

// Team info for Enes100
byte teamType = FIRE;
const char* teamName = "Phoenix";
// replace with Aruco Marker ID.
int markerId = 489; // Set your marker ID here
int roomNumber = 1120;

// put function declarations here:
// Movement functions
// speed should be in the range of 0-255, where 0 is stopped and 255 is full speed.
void moveForward(int speed, int duration);
void moveForward(float distance);
void moveBackward(int speed, int duration);
void turnLeft(float angle);
void turnRight(float angle);
void turnToAngle(float angle);
void correctAngle(float leftDistance, float rightDistance);
void correctToAngle(float targetAngle, int maxAttempts = 5);
float angleDifference(float from, float to);
float normalizedAngleDiff(float from, float to);
float calculateDistance(int trigPin, int echoPin);
float getAngle();
float getCorrectX();
float getCorrectY();
float getCorrectTheta();
void stopMotors();

// IR function
void irSensorReadings();
int averageIRRead(int pin, int samples = 10);
void detectTopographyLocationAorB();
int checkTopography();

// Safe zone navigation functions
void moveToEnd();
void moveForwardUntilWall();
bool isRightFree();
bool isLeftFree();