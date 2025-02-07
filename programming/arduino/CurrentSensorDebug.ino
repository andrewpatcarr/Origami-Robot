float average1 = 0;
float average2 = 0;
#define sensor1 0
#define sensor2 1

#include <Servo.h>

Servo servo1;
Servo servo2;

int angle1 = 90; // Start at 90 degrees for servo 1
int angle2 = 90; // Start at 90 degrees for servo 2

const int minAngle = 170; // Minimum angle
const int maxAngle = 10; // Maximum angle

void setup() {
  // initialize serial communications at 9600 bps:
  Serial.begin(9600);
}

void loop() {
if (Serial.available() > 0) {
  char inputChar = Serial.read();

  while (inputChar == 'w') {
  servo1.write(minAngle); // Set servo 1 to minimum angle
  servo2.write(minAngle); // Set servo 2 to minimum angle

  

  for (int i = 1; i <= 20; i++) {
      average1 = average1 + analogRead(sensor1);
      average2 = average2 + analogRead(sensor2);

  }
  average1 = average1/20;
  average2 = average2/20;

  float out1 = (1/70)*average1 - (51.5/7);
  float out2 = (1/70)*average2 - (51.5/7);

  

  Serial.print(average1);
  Serial.print(" ");
  Serial.print(average2);
  Serial.println();
  
  digitalWrite(12, HIGH);
  digitalWrite(13, HIGH);

  average1 = 0;
  average2 = 0;
  delay(500);

  servo1.write(minAngle); // Set servo 1 to minimum angle
  servo2.write(minAngle); // Set servo 2 to minimum angle
  }
  }
}