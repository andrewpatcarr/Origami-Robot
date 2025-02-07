// This code is for manually controlling the servos of the robot and will give current sensor values
// The main commands are "s" to decompress the robot, "w" to compress the robot, and "x" to stop movement
// The other commands were changed often so can be adjusted to do more specific operations

#include <Servo.h>

Servo servo1;
Servo servo2;

int angle1 = 90; // Start at 90 degrees for servo 1
int angle2 = 90; // Start at 90 degrees for servo 2

float average1 = 0;
float average2 = 0;
#define sensor1 0
#define sensor2 1

const int minAngle = 10; // Minimum angle
const int maxAngle = 170; // Maximum angle
const float servoPowerPercentage = 1; // Servo power percentage (20%)

void setup() {
  servo1.attach(8); // Attach servo 1 to pin 8
  servo2.attach(7); // Attach servo 2 to pin 7
  
  servo1.write(angle1); // Set initial angle for servo 1
  servo2.write(angle2); // Set initial angle for servo 2
  
  Serial.begin(9600); // Initialize serial communication
}

void loop() {

  for (int i = 1; i <= 20; i++) {
      average1 = average1 + analogRead(sensor1);
      average2 = average2 + analogRead(sensor2);
  }
  average1 = average1/20;
  average2 = average2/20;

  float out1 = (1/70)*average1 - (51.5/7); // Trying to get current value out in amps from analog values
  float out2 = (1/70)*average2 - (51.5/7);

  Serial.print(average1);
  Serial.print(" ");
  Serial.print(average2);
  Serial.println();

  average1 = 0;
  average2 = 0;
  delay(50);

  if (Serial.available() > 0) { // If data is available to read
    char inputChar = Serial.read();  // Read the incoming byte

    if (inputChar == 'w') {     // Should compress robot
      angle1 = 90 + int((maxAngle - 90) * servoPowerPercentage);
      angle2 = 90 + int((maxAngle - 90) * servoPowerPercentage); 
    } else if (inputChar == 's') { // Should decompress robot
      angle1 = 90 - int((90 - minAngle) * servoPowerPercentage);
      angle2 = 90 - int((90 - minAngle) * servoPowerPercentage); 
    } else if (inputChar == 'x') { // Stop motion robot
      angle1 = 90;
      angle2 = 90;
    } else if (inputChar == 'a') { 
      angle2 = 90 + int((maxAngle - 90) * servoPowerPercentage); 
    } else if (inputChar == 'd') { 
      angle2 = 90 - int((90 - minAngle) * servoPowerPercentage); 
    } else if (inputChar == 'z') { 
      angle2 = 90;
      angle1 = 90; 
    }
    servo1.write(angle1); // Write the new angle to servo 1
    servo2.write(angle2); // Write the new angle to servo 2
  }
  
  delay(100); // Adjust delay as needed for responsiveness
}

