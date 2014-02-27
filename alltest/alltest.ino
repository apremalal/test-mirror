#define LIMIT_SWITCH_X1_PIN                  20 //int.3
#define LIMIT_SWITCH_X2_PIN                  21 //int.2
#define LIMIT_SWITCH_Y1_PIN                  18 //int.5
#define LIMIT_SWITCH_Y2_PIN                  19 //int.4

int val; 
int encoder0PinA = 2;
int encoder0PinB = 40;
int encoder0Pos = 0;
int encoder0PinALast = LOW;
int n = LOW;

int motorXPin1=22;
int motorXPin2=23;

int motorYPin1=24;
int motorYPin2=25;

int pwmX=8;
int pwmY =9;
int inByte;

volatile int reachedX1 = LOW, reachedX2 = LOW, reachedY1 = LOW, reachedY2 = LOW;

 void setup() {
   attachInterrupt(2, stopX2, HIGH);
   attachInterrupt(3, stopX1, HIGH);
   attachInterrupt(4, stopY2, HIGH);
   attachInterrupt(5, stopY1, HIGH); 
   pinMode (encoder0PinA,INPUT);
   pinMode (encoder0PinB,INPUT);
   Serial.begin (9600);
  analogWrite(pwmX,90);
  analogWrite(pwmY,60);
 } 

 void loop() { 
  while(!reachedX2)
  {
    digitalWrite(motorXPin1,HIGH);
    digitalWrite(motorXPin2,LOW);
  }
  while(!reachedX1){
    digitalWrite(motorXPin1,LOW);
    digitalWrite(motorXPin2,HIGH);
  }
  digitalWrite(motorXPin1,LOW);
  digitalWrite(motorXPin2,LOW);

 } 
 void stopX1()
{
  reachedX1 = !reachedX1;
}

void stopX2(){
    reachedX2 = !reachedX2;
}

void stopY1(){
    reachedY1 = !reachedY1;
}

void stopY2(){
  reachedY2 = !reachedY2;
}
