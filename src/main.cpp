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
int myFunction(int, int);

// Movement functions
void moveForward(int speed, int duration);
void moveBackward(int speed, int duration);
void turnLeft(int speed, int duration);
void turnRight(int speed, int duration);

void setup() {
    Enes100.begin("Phoenix ", FIRE, markerId, 1120, 11, 12);
    // At this point we know we are connected.
    Enes100.println("Connected...");
  // put your setup code here, to run once:
  int result = myFunction(2, 3);
  Serial.begin(9600);
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