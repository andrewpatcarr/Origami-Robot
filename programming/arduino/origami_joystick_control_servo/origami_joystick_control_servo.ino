#include <Servo.h>

Servo servo1;
Servo servo2;

#define sensor1 0
#define sensor2 1

#define x_val 4
#define y_val 5

int key = 2;
int key_before = 0;
bool on = true;


int x = 512;
int y = 512;

int j = 0;

int servo_1_speed = 90;
int servo_2_speed = 90;

void setup() {
  // put your setup code here, to run once:
  servo1.attach(8); // Attach servo 1 to pin 8
  servo2.attach(7); // Attach servo 2 to pin 7

  //pinMode(key, INPUT_PULLUP);
  //attachInterrupt(digitalPinToInterrupt(key), on_off, FALLING);

  servo1.write(servo_1_speed); // Stop servo 1
  servo2.write(servo_2_speed); // Stop servo 2

  delay(1000); // Stop for 1 second

  Serial.begin(9600); // Initialize serial communication
}

int read_joystick(){
    x = analogRead(x_val);
    y = analogRead(y_val);
}

void on_off(){
    on = !on;
}

void compress(){
    double y_norm = (y-510)/510;
    double x_norm = (x-510)/510;
    int max_speed = y_norm * 90 + 89;
    if (x_norm > 0.3){
        servo_1_speed = max_speed;
        servo_2_speed = y_norm*(1-x_norm)*90 +89;
    }
    else if (x_norm < -0.3){
        servo_1_speed = y_norm*(1-x_norm)*90 +89;
        servo_2_speed = max_speed;
    }
    else{
        servo_1_speed = max_speed;
        servo_2_speed = max_speed;
    }
    servo1.write(servo_1_speed);
    servo2.write(servo_2_speed);
}

void decompress(){
    double y_norm = (y-510)/510;
    double x_norm = (x-510)/510;
    int max_speed = y_norm*90 + 1;
    if (x_norm > 0.3){
        servo_1_speed = max_speed;
        servo_2_speed = y_norm*(1-x_norm)*90 + 1;
    }
    else if (x_norm < -0.3){
        servo_1_speed = y_norm*(1-x_norm)*90 + 1;
        servo_2_speed = max_speed;
    }
    else{
        servo_1_speed = max_speed;
        servo_2_speed = max_speed;
    }
    servo1.write(servo_1_speed);
    servo2.write(servo_2_speed);
}

void stop(){
    servo1.write(90);
    servo2.write(90);
}

void print_xy(){
  Serial.print("x = ");
  Serial.print(x);
  Serial.print(" y = ");
  Serial.print(y);
  Serial.println();
}

void loop() {
  
  if (on == 1){
      read_joystick();
      print_xy();

      if (y > 550){
          compress();
      }
      else if (y < 470){
          decompress();
      }
      else{
          stop();
      }
  }
  else {
      stop();

  }
  delay(10);
}
