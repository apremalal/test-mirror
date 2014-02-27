#define LIMIT_SWITCH_X1_PIN                  20 //int.3
#define LIMIT_SWITCH_X2_PIN                  21 //int.2
#define LIMIT_SWITCH_Y1_PIN                  18 //int.5
#define LIMIT_SWITCH_Y2_PIN                  19 //int.4

volatile int reachedX1 = LOW, reachedX2 = LOW, reachedY1 = LOW, reachedY2 = LOW;

void setup() {
   pinMode(13, OUTPUT);
   attachInterrupt(2, stopX2, CHANGE );
   attachInterrupt(3, stopX1, CHANGE);
   attachInterrupt(4, stopY2, RISING );
   attachInterrupt(5, stopY1, RISING );   
   Serial.begin(9600);
}

void loop()
{
  
}

void stopX1()
{
  reachedX1 = !reachedX1;
  Serial.println("X1 changed");
}

void stopX2(){
    reachedX2 = !reachedX2;
   Serial.println("X2 changed");
}

void stopY1(){
    reachedY1 = !reachedY1;
  Serial.println("Y1 changed");
}

void stopY2(){
  reachedY2 = !reachedY2;
   Serial.println("Y2 changed");
}
