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

double FULL_TILT = 5;  //degrees. Need to find what is a good number for each jump
int PREV_DIR = 0;

int M1_PINS[2] = {M1_IN1_PIN, M1_IN2_PIN};
double M1_DES = 0;

int M2_PINS[2] = {M2_IN1_PIN, M2_IN2_PIN};
double M2_DES = 0;

int K_P = 10;
int K_I = 1;
int PID_TIME = 50;
unsigned long START = 0;
unsigned long LAST_TIME = 0;

volatile int M1_VEL = 0;
double M1_POS = 0;
volatile int M1_POS_LAST = 0;
volatile int M2_VEL = 0;
double M2_POS = 0;
volatile int M2_POS_LAST = 0;

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
          
          error = des - pos;
          integral += error * interval;
          Serial.print("Actual: ");
          Serial.print(pos);
          Serial.print("Desired: ");
          Serial.print(des);
          Serial.print("Error: ");
          Serial.println(error);
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
              reverse(min(pwm,255)); // reverse should compress robot
          }
          else {
              stop();
          }
      }
      void update_pos(double posi){
          pos = posi;
          
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
  delay(500);
  init_joystick();
  Serial.println();
  Serial.print(middle_x);
  Serial.print(middle_y);
  Serial.println("done with setup");
  delay(1500);

}

void loop() {

  read_joystick();
  //print_xy();
  if (y > y_upper){
      Serial.println("decomp");
      check_dir(1);
      decompress();
      //Serial.print("M1 Pos: ");
      //Serial.print(get_m1_angle());
      //Serial.print(" Des pos: ");
      //Serial.println(M1_DES);
      // add way to ensure back to even tension when coming from turn back to compression
      START = millis();
      while (millis() - START < PID_TIME){
        do_pid();
        //motor_1.print_info();
        
      }
      
  }
  else if (y < y_lower){
      Serial.println("comp");
      check_dir(0);
      compress();
      //Serial.print("M1 Pos: ");
      //Serial.print(get_m1_angle());
      //Serial.print(" Des pos: ");
      //Serial.println(M1_DES);
      // add way to ensure back to even tension when coming from turn back to decompression
      START = millis();
      while (millis()-START < PID_TIME){
        do_pid();
        //motor_1.print_info();
      }
  }
  else{
      motor_1.stop();
      motor_2.stop();
      delay(PID_TIME);
  }
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
        M2_DES += (1-x_norm) * FULL_TILT;
    }
    else if (x_norm < -0.3){
        M1_DES += (1 - abs(x_norm)) * FULL_TILT;
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
        double added = -abs(y_norm)*x_norm * FULL_TILT;
        Serial.print("Added: ");
        Serial.println(added);
        M2_DES += (1-x_norm) * max_speed;
    }
    else if (x_norm < -0.3){
        double added = -abs(y_norm)*x_norm * FULL_TILT;
        Serial.print("Added: ");
        Serial.println(added);
        M1_DES += (1-abs(x_norm)) * max_speed;
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
    Serial.print(" decompress: ");
    Serial.println(max_speed);

}
void print_xy(){
  Serial.print("x = ");
  Serial.print(x);
  Serial.print(" y = ");
  Serial.print(y);
  Serial.println();
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
  Serial.print("M1, M2 DES:");
  Serial.print(M1_DES);
  Serial.print(", ");
  Serial.println(M2_DES);
  motor_1.pid(M1_DES);
  motor_2.pid(M2_DES);
  motor_1.go();
  motor_2.go();
  delay(5);
}
void check_dir(int dir){
  
  if (dir != PREV_DIR){
    Serial.print(dir);
    Serial.println(", in check dir");
    motor_1.clear_integral();
    motor_2.clear_integral();
    get_m1_angle();
    get_m2_angle();
    if (dir == 1){
      M1_DES = -1*abs(M1_POS);
      M2_DES = -1*abs(M1_POS);
    }
    else {
      M1_DES = 1*abs(M1_POS);
      M2_DES = 1*abs(M1_POS);
    }
    //M1_DES = M1_POS;
    //M2_DES = M2_POS;
    //Serial.print("M1_POS");
    //Serial.print(M1_POS);
    Serial.print("M1_DES");
    Serial.println(M1_DES);

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
