/*obstacle avoiding, Bluetooth control, voice control robot car.
   http://srituhobby.com
*/
#include <Servo.h>
#include <AFMotor.h>
#include "Ultrasonic.h" //distance sensor
#include <LiquidCrystal_I2C.h> //library for lcd

//speed
#define Speed 200

//ultrasonic distance sensor
#define SIG_PIN A0

//ir sensor
#define irLeft 22
#define irRight 23


char value; // caracter read by the serial input
int actualSpeed; 

//obstacle detection
Ultrasonic sonar(SIG_PIN);
Servo servo;

//motors control
AF_DCMotor M1(1);
AF_DCMotor M2(2);
AF_DCMotor M3(3);
AF_DCMotor M4(4);

//global variable
int distance = 0;
int leftDistance;
int rightDistance;
boolean object;
boolean lanefollowing = false; 
//mode script
boolean script = false ;
//indique si les input son du text a afficher ou des commande
boolean print_lcd = false;

char printedrow2[16];
int index = 0;
LiquidCrystal_I2C lcd(0x27,16,2); 

//initial configurations   
void setup() {
  //bluetooth serial port
  Serial.begin(9600);

 //object detection 
  servo.attach(10);
  servo.write(150);
  
 //lane following 
  pinMode(irLeft, INPUT);
  pinMode(irRight, INPUT);
 
  set_Speed(Speed);

  //afficher lcd initialisation
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("verouiller");
}
//loop program
void loop() {

 Bluetoothcontrol();
 //to activate the lane following send a F (forward) to stop it send S
 //per default it's lane following+ object detection to passe to script mode send # 
 if (lanefollowing){
  lanefollow();
 }
}

//permet de suivre la line avec les capteurs infra rouge
void lanefollow(){
  //the signal is returned : n'importe quel matiere 
  if (digitalRead(irLeft) == 0 && digitalRead(irRight) == 0 ) {
    objectAvoid();
    //forword
  }
  //la ligne est detecter par le capteur gauche
  else if (digitalRead(irLeft) == 1 && digitalRead(irRight) == 0 ) {
    objectAvoid();
    Serial.println("TL");
    //leftturn
    left();
  }
  //la ligne est detecter par le capteur droit
  else if (digitalRead(irLeft) == 0 && digitalRead(irRight) == 1 ) {
    objectAvoid();
    Serial.println("TR");
    //rightturn
    right();
  }
  //la ligne d'arrive ou de demarrage : detecter par les deux capteurs
  else if (digitalRead(irLeft) == 1 && digitalRead(irRight) == 1 ) {
    //Stop
    Stop();
  }
  }
  
// object detection and avoidance
 void objectAvoid() {
  distance = getDistance();
  if (distance <= 15) { // obstacle
    //stop
    Stop();
    Serial.println("Stop");

    lookLeft();
    lookRight();
    delay(100);

    //decision de quel cote surpasser l'obstacle
    if (rightDistance <= leftDistance) {
      //left
      object = true;
      turn();
      Serial.println("move Left");
    } else {
      //right
      object = false;
      turn();
      Serial.println("move Right");
    }
    delay(100);
  }
  else {// pas d'obstacle 
    //forword
    Serial.println("move forword");
    forward();
  }
}

// calcule des distance using ultrasonic detector
int getDistance() {
  delay(50);
  int cm = sonar.MeasureInCentimeters();
  Serial.println("distance");
  Serial.println(cm);
  return cm;
}

//get la distance des objets sur la gauche
int lookLeft () {
  //lock left
  servo.write(200);
  delay(500);
  leftDistance = getDistance();
  delay(100);
  servo.write(150);
  Serial.print("Left:");
  Serial.print(leftDistance);
  return leftDistance;
  delay(100);
}

// get la distance des objet sur la droite du vehicule
int lookRight() {
  //lock right
  servo.write(100);
  delay(500);
  rightDistance = getDistance();
  delay(100);
  servo.write(150);
  Serial.print("   ");
  Serial.print("Right:");
  Serial.println(rightDistance);
  return rightDistance;
  delay(100);
}

// permet au vehicule de depasser l'obstacle 
void turn() {
  if (object == false) {
    Serial.println("turn Right");
    left();
    delay(700);
    forward();
    delay(1000);
    right();
    delay(900);
    if (digitalRead(irRight) == 1) {
      loop();
    } else {
      forward();
    }
  }
  else {
    Serial.println("turn left");
    right();
    delay(700);
    forward();
    delay(1000);
    left();
    delay(900);
    if (digitalRead(irLeft) == 1) {
      loop();
    } else {
      forward();
    }
  }
}

//permet de communiquer avec le telephone a traver le bluetooth  
void Bluetoothcontrol() {
  //la lecture se fait caractere par caractere
  if (Serial.available() > 0) {
    value = Serial.read();
    Serial.println(value);
  }
  if (value == '@'){
    print_lcd = !print_lcd ;
    index = 0 ;
    value = ' ';
    if(print_lcd){
       lcd.clear();
       lcd.setCursor(0,0);
       lcd.print("deverrouiller");
       lcd.setCursor(0,1);
       lcd.print(printedrow2); 
       
    }
    else{
       delay(5000); 
       lcd.clear();
       lcd.print("verouiller");
       lcd.setCursor(0,0);
    }
  }
  if(print_lcd){
    //recuper la chaine de caractere a afficher sur le LCD 
     printedrow2[index] = value;
     index++;
  }
  else{
    //des commandes et non pas un text a afficher
     if(value == '#'){
      script = ! script;
      value = ' ';
    }else if (value == 'F') {
      if (!script) lanefollowing = true;
     else forward();
    } else if (value == 'B') {
      backward();
    } else if (value == 'L') {
      left();
    } else if (value == 'R') {
      right();
    } else if (value == 'S') {
      if (!script){ 
        lanefollowing = false; }
        Stop();
    } else if (value == 'N') {
      normalSpeed();
    } else if (value == 'P') {
      rondpoint();
    } else if (value == 'A') {
      accelerer();
    }else if (value == 'D') {
      decelerer();
    }
  }
  
}
/************ controle de la vitesse *******************/
void set_Speed(int s){
  actualSpeed = s;
  M1.setSpeed(s);
  M2.setSpeed(s);
  M3.setSpeed(s);
  M4.setSpeed(s);
  }
  
void normalSpeed(){
  set_Speed(Speed);
}
void accelerer(){
 // set_Speed((int)(1.2 * actualSpeed));
 set_Speed(240);
  }
void decelerer(){
  //set_Speed((int)(0.8 * actualSpeed));
  set_Speed(190);
}

/***************** controle de la direction *********************/
void forward() {
 
  M1.run(FORWARD);
  M2.run(FORWARD);
  M3.run(FORWARD);
  M4.run(FORWARD);
}

void backward() {
  M1.run(BACKWARD);
  M2.run(BACKWARD);
  M3.run(BACKWARD);
  M4.run(BACKWARD);
}
void right() {
  M1.run(BACKWARD);
  M2.run(BACKWARD);
  M3.run(FORWARD);
  M4.run(FORWARD);
}
void left() {
  M1.run(FORWARD);
  M2.run(FORWARD);
  M3.run(BACKWARD);
  M4.run(BACKWARD);
}
void Stop() {
  M1.run(RELEASE);
  M2.run(RELEASE);
  M3.run(RELEASE);
  M4.run(RELEASE);
}
  
void rondpoint(){
  set_Speed(200);
  M1.run(FORWARD);
  M2.run(FORWARD);
  M3.run(RELEASE);
  M4.run(FORWARD);  
  }
