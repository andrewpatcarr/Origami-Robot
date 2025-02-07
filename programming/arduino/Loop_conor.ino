#include <Servo.h>

Servo servo1;
Servo servo2;

int angle1 = 90; // Start at 90 degrees for servo 1
int angle2 = 90; // Start at 90 degrees for servo 2

const int minAngle = 170; // Minimum angle
const int maxAngle = 10; // Maximum angle

float average1 = 0;
float average2 = 0;
#define sensor1 0
#define sensor2 1

// Define the sequence of commands with descriptive names
enum Commands { FORWARD, BACKWARD };

// Define the sequence of commands
Commands commands[] = {FORWARD, BACKWARD};

// Define the duration of each action for each servo in milliseconds
int forwardDuration = 1200; // 1.3 seconds for forward motion
int backwardDuration = 1400; // 1.3 seconds for backward motion

void setup() {
  servo1.attach(8); // Attach servo 1 to pin 8
  servo2.attach(7); // Attach servo 2 to pin 7
  
  servo1.write(angle1); // Set initial angle for servo 1
  servo2.write(angle2); // Set initial angle for servo 2

  servo1.write(90); // Stop servo 1
  servo2.write(90); // Stop servo 2
  delay(1000); // Stop for 1 second

  Serial.begin(9600); // Initialize serial communication
}

void loop() {
if (Serial.available() > 0) {
  char inputChar = Serial.read();

  while (inputChar == 'w') {
  
  servo1.write(90); // Stop servo 1
  servo2.write(90); // Stop servo 2
  delay(20); // Stop for 1 second

  // Move backward

  
  servo1.write(minAngle); // Set servo 1 to minimum angle
  servo2.write(minAngle); // Set servo 2 to minimum angle
  delay(backwardDuration); // Wait for backward duration

  // Stop both servos after forward movement
  servo1.write(90); // Stop servo 1
  servo2.write(90); // Stop servo 2
  delay(20); // Stop for 1 second
  
    // Move forward
  servo1.write(maxAngle); // Set servo 1 to maximum angle
  servo2.write(maxAngle); // Set servo 2 to maximum angle
  delay(forwardDuration); // Wait for forward duration
  }
  if(inputChar == 'x'){
    servo1.write(90); // Write the new angle to servo 1
    servo2.write(90);
  }
  
  // Stop both servos after backward movement
  //servo1.write(90); // Stop servo 1
  //servo2.write(90); // Stop servo 2
  //delay(20); // Stop for 1 second

  //if
}
}


