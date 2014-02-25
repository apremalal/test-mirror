#include <EEPROM.h>

#define MANUAL_MODE              100
#define AUTOMATIC_MODE           200

#define CLOCKWISE                1
#define COUNTER_CLOCKWISE       -1
#define MOTOR_X                  15
#define MOTOR_Y                  16  
#define STEPPER_DELAY            4

#define STOP                     0
#define IMERGENCY_STOP           1
#define MOVE_TO_INITIAL_POSITION 2
#define MOVING_TO_NEXT_TILE            3
#define FOCUS_LENSE              4

#define ENCODER_X_PIN_A          12   
#define ENCODER_X_PIN_B          11
#define ENCODER_Y_PIN_A          10
#define ENCODER_Y_PIN_B          8

#define LIMIT_SWITCH_X1_PIN                  20 //int.3
#define LIMIT_SWITCH_X2_PIN                  21 //int.2
#define LIMIT_SWITCH_Y1_PIN                  18 //int.5
#define LIMIT_SWITCH_Y2_PIN                  19 //int.4

/*STATUS CODES*/
#define SUCCESS                  300
#define FAILED                   505

int encoder_x_position      = 0;
int encoder_y_position      = 0;
int encoder_x_pin_A_last    = LOW;
int encoder_y_pin_A_last    = LOW;
int nX = 0, nY = 0;

int horizontal_block_length = 0;
int vertical_block_length   = 0;

int current_tile_position   = -1;
int motor                   = MOTOR_X;
int _direction              = CLOCKWISE;
int steps                   = 200;

int addr                    = 0;
int operation_status        = FAILED;
String responce             = "";

int motorPin[]={8,9,10,11,44,45,46,47}; // x,y stepper pins
int mode =-1,command=-1,opt1=-1,opt2=-1,opt3=-1;
int uX1value = 0, uX2value = 0, uY1value = 0, uY2value = 0;

volatile int reachedX1 = LOW, reachedX2 = LOW, reachedY1 = LOW, reachedY2 = LOW;

void setup() {
   pinMode (ENCODER_X_PIN_A,INPUT);
   digitalWrite(ENCODER_X_PIN_A, HIGH); 
   pinMode (ENCODER_X_PIN_B,INPUT);
   digitalWrite(ENCODER_X_PIN_B, HIGH); 
   pinMode (ENCODER_Y_PIN_A,INPUT);
   digitalWrite(ENCODER_Y_PIN_A, HIGH); 
   pinMode (ENCODER_Y_PIN_B,INPUT);  
   digitalWrite(ENCODER_Y_PIN_B, HIGH);  
   
   attachInterrupt(2, stopX2, CHANGE);
   attachInterrupt(3, stopX1, CHANGE);
   attachInterrupt(4, stopY2, CHANGE);
   attachInterrupt(5, stopY1, CHANGE);
   
   Serial.begin(9600);
}

void loop()
{
  /***Update X encoder readings***/
   nX = digitalRead(ENCODER_X_PIN_A);
   if ((encoder_x_pin_A_last == LOW) && (nX == HIGH)) {
     if (digitalRead(ENCODER_X_PIN_B) == LOW) {
       encoder_x_position--;
     } else {
       encoder_x_position++;
     }
     Serial.println(encoder_x_position);
   } 
   encoder_x_pin_A_last = nX;  
  
  /***Update Y encoder readings***/
  nY = digitalRead(ENCODER_Y_PIN_A);
   if ((encoder_y_pin_A_last == LOW) && (nY == HIGH)) {
     if (digitalRead(ENCODER_Y_PIN_B) == LOW) {
       encoder_y_position--;
     } else {
       encoder_y_position++;
     }
     Serial.println(encoder_y_position);
   } 
   encoder_y_pin_A_last = nY;
   
 /******Finish encoder reading*******/
  
 if (Serial.available() > 0) {
   mode    = Serial.parseInt();
   command = Serial.parseInt(); 
   opt1    = Serial.parseInt(); 
   opt2    = Serial.parseInt(); 
   opt3    = Serial.parseInt();
   
   if (Serial.read() == '\n') {
     Serial.print(mode, DEC);
     Serial.print(command, DEC);
     Serial.println(opt1, DEC);
   }
 }
 
 switch (mode)
 {
   case MANUAL_MODE:
        switch(command){
           case STOP:
             stopMotors();
             break;
           case IMERGENCY_STOP:
             stopMotors();
             break;
           case MOVE_TO_INITIAL_POSITION:
             break;
           case MOVING_TO_NEXT_TILE:
               moveSteps(motor,_direction,steps);
             break;
           case FOCUS_LENSE:
               doFocus(_direction,steps);
             break;   
        }
     break;
   case AUTOMATIC_MODE:
     switch(command){
       case STOP:
         stopMotors();
         break;
       case IMERGENCY_STOP:
         stopMotors();
         break;
       case MOVE_TO_INITIAL_POSITION:
         //use block number and perform action in a while loop
         break;
       case MOVING_TO_NEXT_TILE:
           moveSteps(motor,_direction,steps);
         break;
       case FOCUS_LENSE:
           doFocus(_direction,steps);
         break;    
     }
     sendResponce();
     operation_status = FAILED;
     break;   
 }
}

void moveSteps(int motor,int _direction,int steps)
{
  switch (motor){
    case MOTOR_X:
      while(steps>0){
        turn(0, _direction);
        steps--;
      }
      stopMotors();
      break;
    case MOTOR_Y:
    while(steps>0){
        turn(4, _direction);
        steps--;
      }
      stopMotors();
      break;
  } 
}

void sendResponce()
{
  responce = operation_status+":";
  responce += command+":";
  Serial.print(responce);
}

void doFocus(int _direction,int steps)
{
  //TODO: program z-axis
}

void turn(int x, int dir){
  byte p[]={1,0,0,0};
  for(int i=0; i <= 3; i++){
    int j=0;
    for(int k = motorPin[x]; k <= motorPin[x+3]; k++){
      digitalWrite(k,p[j]);j++;
    }
    delay(STEPPER_DELAY);
    
   switch(dir){    
    case COUNTER_CLOCKWISE:
          {
            int mem = p[0]; 
            p[0]=p[1];
            p[1]=p[2];
            p[2]=p[3];
            p[3]=mem;
          }
      break;
    case CLOCKWISE:
        {
          int mem = p[3]; 
          p[3]=p[2];
          p[2]=p[1];
          p[1]=p[0];
          p[0]=mem;
        }
     break; 
    }
  }
}

void stopMotors(){
  for(int k = motorPin[0]; k <= motorPin[7]; k++){
     digitalWrite(k,B0);
  }
}

void writeStatusToEEPROM()
{
    EEPROM.write(addr, current_tile_position);
}

int readLimitSwitch(int encoderpin){
  return digitalRead(encoderpin);
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


