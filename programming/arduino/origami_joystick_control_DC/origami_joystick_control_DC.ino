#include <Encoder.h>

#define x_val 4
#define y_val 5

#define M1_IN1_PIN 5
#define M1_IN2_PIN 6
#define M1_ENC_A 2
#define M1_ENC_B 3

#define M2_IN1_PIN 10
#define M2_IN2_PIN 11
#define M2_ENC_A 8
#define M2_ENC_B 9

Encoder enc_1(M1_ENC_A, M1_ENC_B);
Encoder enc_2(M2_ENC_A, M2_ENC_B);
// GLOBALS

double x = 512;
double y = 512;

int j = 0;

double FULL_TILT = 1;  //rad/s

int M1_PINS[2] = {M1_IN1_PIN, M1_IN2_PIN};
double M1_DES = 0;

int M2_PINS[2] = {M2_IN1_PIN, M2_IN2_PIN};
double M2_DES = 0;

int K_P = 100;
int K_I = 10;
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
          
          error = vel - des;
          integral += error * interval;
          Serial.print("Error: ");
          Serial.println(error);
          output = k_p*error + k_i*integral;
          
      }
      void forward(int pwm){
          analogWrite(in1, pwm);
          analogWrite(in2, 0);
          Serial.println(" forwarded ");
      }
      void reverse(int pwm){
          analogWrite(in1, 0);
          analogWrite(in2, pwm);
          Serial.println(" reversed ");
      }
      void stop(){
          analogWrite(in1, 0);
          analogWrite(in2, 0);
      }
      void go(){
          integral = 0;
          pwm = abs(output);
          Serial.print("Output: ");
          Serial.print(output);
          Serial.print(" PWM: ");
          Serial.print(pwm);
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
      void update_vel(double pos){
          vel_time = millis();
          vel = (last_pos-pos)/(vel_time-last_vel_time);
          last_vel_time = vel_time;
      }

};

Motor_DC motor_1(M1_PINS, K_P, K_I);
Motor_DC motor_2(M2_PINS, K_P, K_I);

void setup() {
  
  // Initialize Timer1 for 10 ms interrupt
  //TCNT1 = 0;
  //OCR1A = 1600;  // 100 ms interval with prescaler = 1
  //TCCR1A = 0;   // CTC mode
  //TCCR1B = (1 << WGM12) | (1 << CS10); // CTC mode, no prescaler
  //TIMSK1 |= (1 << OCIE1A); // Enable Timer1 compare interrupt
  
  motor_1.init();
  motor_2.init();
  motor_1.stop();
  motor_2.stop();
  delay(500); // Stop for 1 second

  Serial.begin(9600); // Initialize serial communication
  delay(500);
  Serial.println("done with setup");
  //sei();  // Enable global interrupts

}
int i = 0;
void loop() {
  if (i != 0){
      motor_1.forward(100);
      delay(1000);
      motor_1.reverse(100);
      delay(1000);
      i++;
  }
  

  read_joystick();
  //print_xy();
  if (y > 550){
      compress();
      START = millis();
      
      while (millis() - START < 20){
        M1_POS = get_m1_angle();  //rad
        M2_POS = get_m2_angle();
        
        motor_1.update_vel(M1_POS);
        motor_2.update_vel(M2_POS);
        motor_1.pid(M1_DES);
        motor_2.pid(M2_DES);
        motor_1.go();
        motor_2.go();
      }
  }
  else if (y < 470){
      decompress();
      START = millis();
      while (millis()-START < 20){
        M1_POS = get_m1_angle();  //rad
        M2_POS = get_m2_angle();
        motor_1.update_vel(M1_POS);
        motor_2.update_vel(M2_POS);
        motor_1.pid(M1_DES);
        motor_2.pid(M2_DES);
        motor_1.go();
        motor_2.go();
        delay(5);
      }
  }
  else{
      motor_1.stop();
      motor_2.stop();
  }
}

int read_joystick(){
    x = analogRead(x_val);
    y = analogRead(y_val);
}

double get_m1_angle(){
    long pos = enc_1.read();
    double angle = pos*PI/(12*297.92);  //rad
    return angle;
}
double get_m2_angle(){
    long pos = enc_2.read();
    double angle = pos*PI/(12*297.92);  //rad
    return angle;
}

void compress(){
    //Serial.print("y: ");
    //Serial.print(y);
    double y_norm = (y-510)/510;
    //Serial.print(" y_norm: ");
    //Serial.print(y_norm);
    
    double x_norm = (x-482)/510;
    double max_speed = ((y-510)/510) * FULL_TILT;
    //double max_speed = 1;
    if (x_norm > 0.3){
        M1_DES = max_speed;
        M2_DES = x_norm * FULL_TILT;
    }
    else if (x_norm < -0.3){
        M1_DES = abs(x_norm) * FULL_TILT;
        M2_DES = max_speed;
    }
    else{
        M1_DES = max_speed;
        M2_DES = max_speed;
    }
    
    //Serial.print(" compress: ");
    //Serial.println(max_speed);
}

void decompress(){
    double y_norm = (y-510)/510;
    double x_norm = (x-510)/510;
    double max_speed = -abs(y_norm) * FULL_TILT;
    //double max_speed = -1;
    if (x_norm > 0.3){
        M1_DES = max_speed;
        M2_DES = -abs(y_norm)*x_norm * FULL_TILT;
    }
    else if (x_norm < -0.3){
        M1_DES = -y_norm*abs(x_norm) * FULL_TILT;
        M2_DES = max_speed;
    }
    else{
        M1_DES = max_speed;
        M2_DES = max_speed;
    }
    //Serial.print("y: ");
    //Serial.print(y);
    //Serial.print(" decompress: ");
    //Serial.println(max_speed);

}
void print_xy(){
  Serial.print("x = ");
  Serial.print(x);
  Serial.print(" y = ");
  Serial.print(y);
  Serial.println();
}

ISR(TIMER1_COMPA_vect){
    M1_POS = get_m1_angle();  //rad
    M2_POS = get_m2_angle();  //rad
    M1_VEL = (M1_POS-M1_POS_LAST)/.01;  //rad/s
    M2_VEL = (M2_POS-M2_POS_LAST)/.01;  //rad/s
    M1_POS_LAST = M1_POS;
    M2_POS_LAST = M2_POS;
}
