int x1 = 512;
int y1 = 512;

int key = 2;
bool on = false;

void on_off(){
    on = !on;
    Serial.print(on);
    Serial.println("swapped");
}

void print_xy(){
  Serial.print(x1);
  Serial.print("\t");
  Serial.print(y1);
  Serial.println();

}
void setup(){
  pinMode(key, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(key), on_off, FALLING);
  
  Serial.begin(9600); // Initialize serial communication
  Serial.print("we here");
  Serial.println();
}

void loop(){
  x1 = analogRead(4);
  y1 = analogRead(5);
  print_xy();
  //Serial.print("pin 2: ");
  //Serial.print(digitalRead(key));
  //Serial.print(" on: ");
  //Serial.print(on);
  //Serial.println();
  delay(1000);
}