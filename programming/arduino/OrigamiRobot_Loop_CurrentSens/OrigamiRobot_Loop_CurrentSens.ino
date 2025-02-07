// This code is the main loop for the robot which uses the analog inputs from the current sensor.
// To start the cycle, input "w" into the serial monitor.
// The feedback process are as such:

// For compression, the arduino reads the analog values, compares it to the threshold values and runs
// the servos until the analog values are larger than the thresholds. Also, there is an "or" statement that
// deals with the initial spike in current draw by forcing the robot to compress for at least 700 ms. This should
// be examined and possibly lowered for cases where the robot starts very compressed. 

// For decompression, the arduino makes the servos decompress for a set amount of time. This works fine, but there
// is not feedback or intelligence in when it stops expanding. This could be figured out in the future for more
// optimization.


#include <Servo.h>

Servo servo1;
Servo servo2;

int angle1 = 90; 
int angle2 = 90;

const int minAngle = 170; // Angles to set power inputs to the servos
const int maxAngle = 10;

float average1 = 0;
float average2 = 0;

#define sensor1 0
#define sensor2 1

int sensorMax1 = 570;
int sensorMax2 = 570;

int cycleCount = 1;

int timeComp = 0;
int timeCheck = 700;
int swapNum = 0;
int timeDecomp = 0;

float slope1 = 0;
float slope2 = 0;
int slopeCalc1[] = {0, 0, 0};
int slopeCalc2[] = {0, 0, 0};

int difStiffFactor = .95; // Since it is easier for the servos to decompress than compress we need to adjust the time decompressing
                      // Probably could do some calcs to find a more theoretical number rather than empiracal one
                      // Not used currently^^
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

  while (inputChar == 'w') { // Input w to make the robot start cycling
  
  servo1.write(90); // Stop servo 1
  servo2.write(90); // Stop servo 2
  delay(20); // Stop for 20 ms

  
  for (int i = 1; i <= 20; i++) { // Any for loop like this is taking a data sample from the current sensor
      average1 = average1 + analogRead(sensor1);
      average2 = average2 + analogRead(sensor2);
      }

  average1 = average1/20;
  average2 = average2/20;


  //if(cycleCount%2 == 0)
  //{

  // Compress until servos pull enough current
   
          // sensorMax are the thresholds for when the robot is at max compression and should stop compressing
    while(average1 < sensorMax1 && average2 < sensorMax2 || timeComp < timeCheck) // timeComp or statement stabalizes compression
    //                                                                               Against initial compression spikes in current
    {
      servo1.write(minAngle); // Writing to minAngle makes the robot compress 
      servo2.write(minAngle+10); // The closer an angle is to 0 or 180 for this command, the more power will be given to the servo
                                 // One servo stronger than the other so adding 10 to adjust, current sensor thesholds make this not
                                 // matter much but could still optimize it
      for (int i = 1; i <= 20; i++) 
      {
      average1 = average1 + analogRead(sensor1);
      average2 = average2 + analogRead(sensor2);
      }
      average1 = average1/20;
      average2 = average2/20;

      delay(100); // time per cycle, could lower for faster cycles but isn't too critical at this point
                  // Especially since we don't have an exact current sensor theshold value, but could probably optimize these two things

      Serial.print(average1); // Prints servo1 reading
      Serial.print(" ");
      Serial.print(average2); // Prints servo2 reading
      Serial.print(" ");
      Serial.print(timeComp); // Prints time compressed
      Serial.println();

      timeComp = timeComp + 100;
        
    }


    delay(20);

    average1 = 0;
    average2 = 0;
  
    



  // Stop both servos after compression
  servo1.write(90); 
  servo2.write(90);

  // Turn on LEDS
  digitalWrite(12, HIGH); 
  digitalWrite(13, HIGH);

  delay(40); // Stop for 1 second
  
  // Turn off LEDS
  digitalWrite(12, LOW);
  digitalWrite(13, LOW);

  // Decompress for for a part of the compression time
      for (int i = 1; i <= 20; i++) 
      {
      average1 = average1 + analogRead(sensor1);
      average2 = average2 + analogRead(sensor2);
      }
      average1 = average1/20;
      average2 = average2/20;

        Serial.print(average1);
        Serial.print(" ");
        Serial.print(average2);
        Serial.println();
        

// slope1 and slope2 are not currently doing anything, tried to use them to find when the robot at maximum expansion
// but ran out of time to compeltely try and it, and having one expansion time (850 ms) works well since the robot will
// stop at max compression no matter what. 850 ms gets the robot pretty expanded, may need to adjust once proper friction pads
// are implemented.

  while(slope1 < 0 && slope2 < 0 || timeDecomp < 850)
  {
    for (int j = 0; j <= 1; j++)
    {
      for (int i = 1; i <= 20; i++) 
      {
      average1 = average1 + analogRead(sensor1);
      average2 = average2 + analogRead(sensor2);
      }

      average1 = average1/20;
      average2 = average2/20;

      Serial.print(average1);
      Serial.print(" ");
      Serial.print(average2);
      Serial.println();

      slopeCalc1[j] = average1;
      slopeCalc2[j] = average2;
    }

    slope1 = (slopeCalc1[0]-slopeCalc1[1])/.10;
    slope2 = (slopeCalc2[0]-slopeCalc2[1])/2;

    Serial.print(average1);
    Serial.print(" ");
    Serial.print(average2);
    Serial.print(" ");
    Serial.print(slope1);
    Serial.print(" ..");
    Serial.print(slope2);
    Serial.println();
    

  servo1.write(maxAngle); // maxAngle spins the servos the opposite direction of minAngle
  servo2.write(maxAngle-10); // One servo seems to be stronger than the other, needs to be adjusted for on the expansion.
                             

  delay(100); // time per cycle up to 850 as of now
  timeDecomp = timeDecomp + 100;


  }
  // Turn on LEDS
  digitalWrite(12, HIGH);
  digitalWrite(13, HIGH);

  delay(40);  
  
  // Turn off LEDS
  digitalWrite(12, LOW);
  digitalWrite(13, LOW);
  //delay(timeComp*difStiffFactor); // Wait for forward duration
  //delay(1000);

  timeComp = 0;
  timeDecomp = 0;
  cycleCount = cycleCount + 1;
  average1 = 0;
  average2 = 0;
  }
  
  


  
}
}


