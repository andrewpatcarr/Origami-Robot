#include <Encoder.h>

#define x_val 4
#define y_val 5

#define M1_IN1_PIN 5
#define M1_IN2_PIN 6
#define M1_ENC_A 2
#define M1_ENC_B 8

#define M2_IN1_PIN 10
#define M2_IN2_PIN 11
#define M2_ENC_A 3
#define M2_ENC_B 9

Encoder enc_1(M1_ENC_A, M1_ENC_B);
Encoder enc_2(M2_ENC_A, M2_ENC_B);
// GLOBALS

double x = 512;
double y = 512;

double middle_y = 15;
double middle_x = 15;
double y_upper = middle_y*1.2;
double y_lower = middle_y*.8;
double x_upper = middle_x*1.2;
double x_lower = middle_x*.8;

int j = 0;

double FULL_TILT = 1;  //degrees. Need to find what is a good number for each jump
int PREV_DIR = 0;

int M1_PINS[2] = {M1_IN1_PIN, M1_IN2_PIN};
double M1_DES = 0;

int M2_PINS[2] = {M2_IN1_PIN, M2_IN2_PIN};
double M2_DES = 0;

int K_P = 10;
int K_I = 1;
int PID_TIME = 100;
unsigned long START = 0;
unsigned long LAST_TIME = 0;

volatile int M1_VEL = 0;
double M1_POS = -999;
volatile int M1_POS_LAST = -999;
volatile int M2_VEL = 0;
double M2_POS = -999;
volatile int M2_POS_LAST = -999;

class Motor_DC
{
  private:
      int in1;
      int in2;
      int encA;
      int encB;
      double k_p;
      double k_i;
      double interval;
      int error;
      double integral;
      unsigned long prev_time;
      int speed;
      double output;
      int pwm;
      double pos;
      double last_pos;
      int angle;
      double vel;
      unsigned long vel_time;
      unsigned long last_vel_time;

  public:
      Motor_DC(int pins[2], int k_p, int k_i){
          in1 = pins[0];
          in2 = pins[1];
          this->k_p = k_p;
          this->k_i = k_i;
          
      }
      void init(){
          pinMode(in1, OUTPUT);
          pinMode(in2, OUTPUT);
          //pinMode(encA, INPUT);
          //pinMode(encB, INPUT);
          
      }
      void pid(int des){
          interval = millis() - prev_time;
          prev_time = millis();
          
          error = pos - des;
          integral += error * interval;
          //Serial.print("Error: ");
          //Serial.println(error);
          output = k_p*error + k_i*integral;
          
      }
      void forward(int pwm){
          analogWrite(in1, pwm);
          analogWrite(in2, 0);
          //Serial.println(" forwarded ");
      }
      void reverse(int pwm){
          analogWrite(in1, 0);
          analogWrite(in2, pwm);
          //Serial.println(" reversed ");
      }
      void stop(){
          analogWrite(in1, 0);
          analogWrite(in2, 0);
      }
      void go(){
          integral = 0;
          pwm = abs(output);
          //Serial.print("Output: ");
          //Serial.print(output);
          //Serial.print(" PWM: ");
          //Serial.print(pwm);
          if (output > 0){
              forward(min(pwm,255));
          }
          else if (output < 0){
              reverse(min(pwm,255));
          }
          else {
              stop();
          }
      }
      void update_pos(double pos){
          this->pos = pos;
          
      }
      void clear_integral(){
          integral = 0;
      }
      void print_info(){
          Serial.print("Output: ");
          Serial.print(output);
          Serial.print(" PWM: ");
          Serial.println(pwm);
      }

};

Motor_DC motor_1(M1_PINS, K_P, K_I);
Motor_DC motor_2(M2_PINS, K_P, K_I);

void setup() {

  motor_1.init();
  motor_2.init();
  motor_1.stop();
  motor_2.stop();
  delay(500); // Stop for 1 second
  
  Serial.begin(9600); // Initialize serial communication
  delay(500);
  read_joystick();
  print_xy();
  delay(1500);
  init_joystick();
  Serial.println();
  Serial.print(y_upper);
  Serial.print(y_lower);

  Serial.println("done with setup");
  delay(1500);

}

void loop() {
  read_joystick();
  print_xy();
  M1_POS = get_m1_angle();
  M2_POS = get_m2_angle();
  print_angles();


  if (x > x_upper || x < x_lower){
    if (x > x_upper){
      motor_1.stop();
      motor_2.forward(100);
      Serial.println("m2 forward");
    }
    else if (x < x_lower){
      motor_1.stop();
      motor_2.reverse(100);
      Serial.println("m2 reverse");
    }
  }
  else {
    if (y > y_upper){
      motor_1.forward(100);
      Serial.println("m1 forward");
    }
    else if (y < y_lower){
      motor_1.reverse(100);
      Serial.println("m1 reverse");
    }
  }
  if (x < x_upper && x > x_lower){
    motor_2.stop();
    
  }
  if (y < y_upper && y > y_lower){
    motor_1.stop();
    
  }
  delay(100);

}

int read_joystick(){
    x = analogRead(x_val);
    y = analogRead(y_val);
}

double get_m1_angle(){
    long pos = enc_1.read();
    double angle = pos*180/(12*297.92);  //degree
    return angle;
}
double get_m2_angle(){
    long pos = enc_2.read();
    double angle = pos*180/(12*297.92);  //degree
    return angle;
}

void compress(){
    //Serial.print("y: ");
    //Serial.print(y);
    double y_norm = (y-middle_y)/middle_y;
    //Serial.print(" y_norm: ");
    //Serial.print(y_norm);
    
    double x_norm = (x-middle_x)/middle_x;
    double max_speed = abs(y_norm) * FULL_TILT;
    //double max_speed = 1;
    if (x_norm > 0.3){
        M1_DES += max_speed;
        M2_DES += x_norm * FULL_TILT;
    }
    else if (x_norm < -0.3){
        M1_DES += abs(x_norm) * FULL_TILT;
        M2_DES += max_speed;
    }
    else{
        M1_DES += max_speed;
        M2_DES += max_speed;
    }
    
    //Serial.print(" compress: ");
    //Serial.println(max_speed);
}

void decompress(){
    double y_norm = (y-middle_y)/middle_y;
    double x_norm = (x-middle_x)/middle_x;
    double max_speed = -abs(y_norm) * FULL_TILT;
    //double max_speed = -1;
    if (x_norm > 0.3){
        M1_DES += max_speed;
        M2_DES += -abs(y_norm)*x_norm * FULL_TILT;
    }
    else if (x_norm < -0.3){
        M1_DES += -y_norm*abs(x_norm) * FULL_TILT;
        M2_DES += max_speed;
    }
    else{
        M1_DES += max_speed;
        M2_DES += max_speed;
    }
    //Serial.print("y: ");
    //Serial.print(y);
    //Serial.print(" y_norm: ");
    //Serial.print(y_norm);
    //Serial.print(" decompress: ");
    //Serial.println(max_speed);

}
void print_xy(){
  Serial.print("x = ");
  Serial.print(x);
  Serial.print(" y = ");
  Serial.print(y);
  //Serial.println();
}
void do_pid(){
  M1_POS = get_m1_angle();
  M2_POS = get_m2_angle();
  Serial.print("M1 Angle: ");
  Serial.print(M1_POS);
  Serial.print("M2 Angle: ");
  Serial.println(M2_POS);
  motor_1.update_pos(M1_POS);
  motor_2.update_pos(M2_POS);
  motor_1.pid(M1_DES);
  motor_2.pid(M2_DES);
  motor_1.go();
  motor_2.go();
  delay(5);
}
void check_dir(int dir){
  if (dir != PREV_DIR){
    motor_1.clear_integral();
    motor_2.clear_integral();
    M1_DES = M1_POS;
    M2_DES = M2_POS;
    PREV_DIR = dir;
  }
}
void init_joystick(){
  middle_x = x;
  middle_y = y;
  y_upper = middle_y*1.2;
  y_lower = middle_y*.8;
  x_upper = middle_x*1.2;
  x_lower = middle_x*.8;
}
void print_angles(){
  Serial.print("M1 Angle: ");
  Serial.print(M1_POS);
  Serial.print(" M2 Angle: ");
  Serial.println(M2_POS);
}