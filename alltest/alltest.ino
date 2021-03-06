#include <EEPROM.h>

#define MANUAL_MODE              100
#define AUTOMATIC_MODE           200

#define CLOCKWISE                1
#define COUNTER_CLOCKWISE       -1
#define MOTOR_X                  15
#define MOTOR_Y                  16 
#define MOTOR_X_Y                17
#define DIRECTION_PLUS           50
#define DIRECTION_MINUS          51

#define STOP                     0
#define IMERGENCY_STOP           1
#define MOVE_TO_INITIAL_POSITION 2
#define MOVING_TO_NEXT_TILE      3
#define FOCUS_LENSE              4

#define MOTOR_X_PIN_1            22
#define MOTOR_X_PIN_2            23
#define MOTOR_X_PWM_PIN          8
#define MOTOR_Y_PWM_PIN          9

#define MOTOR_X_PWM              255
#define MOTOR_Y_PWM              255

#define MOTOR_Y_PIN_1            24
#define MOTOR_Y_PIN_2            25

#define ENCODER_X_PIN_A          3  
#define ENCODER_X_PIN_B          41
#define ENCODER_Y_PIN_A          2
#define ENCODER_Y_PIN_B          40

#define LIMIT_SWITCH_X1_PIN      20 //int.3
#define LIMIT_SWITCH_X2_PIN      21 //int.2
#define LIMIT_SWITCH_Y1_PIN      18 //int.5
#define LIMIT_SWITCH_Y2_PIN      19 //int.4

/*STATUS CODES*/
#define SUCCESS                  300
#define FAILED                   505

int current_motor_x_direction       = -1;
int current_motor_y_direction       = -1;

volatile int encoder_x_position      = 0;
volatile int encoder_y_position      = 0;

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

int inByte  = -1;
volatile int reachedX1 = 0, reachedX2 = 0, reachedY1 = 0, reachedY2 = 0;

int limit_x1 = 1, limit_y1 = 0;
int limit_x2 = 1, limit_y2 = 0;
   
void setup() {
   Serial.begin(9600);
   analogWrite(MOTOR_X_PWM_PIN,MOTOR_X_PWM);
   analogWrite(MOTOR_Y_PWM_PIN,MOTOR_Y_PWM);
   stopMotors(MOTOR_X_Y);
           
   moveMotor(MOTOR_X,DIRECTION_MINUS);
   moveMotor(MOTOR_Y,DIRECTION_MINUS);
      
   while(limit_x1 || !limit_y1){
    if(limit_x1 == 0)
     stopMotors(MOTOR_X);
    if(limit_y1 == 1)
     stopMotors(MOTOR_Y);
    limit_x1 =  digitalRead(LIMIT_SWITCH_X1_PIN);
    limit_y1 =  digitalRead(LIMIT_SWITCH_Y1_PIN);
   }
   limit_x1 = 1; limit_y1 = 0;
   stopMotors(MOTOR_X_Y);
   
   pinMode (ENCODER_X_PIN_A,INPUT);
   digitalWrite(ENCODER_X_PIN_A, HIGH); 
   pinMode (ENCODER_X_PIN_B,INPUT);
   digitalWrite(ENCODER_X_PIN_B, HIGH); 

   attachInterrupt(1, doEncoderX, CHANGE); // x encoder
   
   pinMode (ENCODER_Y_PIN_A,INPUT);
   digitalWrite(ENCODER_Y_PIN_A, HIGH); 
   pinMode (ENCODER_Y_PIN_B,INPUT);  
   digitalWrite(ENCODER_Y_PIN_B, HIGH);  
   
   attachInterrupt(0, doEncoderY, CHANGE); // y encoder
   
   Serial.print("X min:"); Serial.println(encoder_x_position);
   Serial.print("Y min:"); Serial.println(encoder_y_position);
           
   moveMotor(MOTOR_X,DIRECTION_PLUS);
   moveMotor(MOTOR_Y,DIRECTION_PLUS);
      
   while(limit_x2 || !limit_y2){
    if(limit_x2 == 0)
     stopMotors(MOTOR_X);     
    if(limit_y2 == 1)
      stopMotors(MOTOR_Y);
    limit_x2 =  digitalRead(LIMIT_SWITCH_X2_PIN);
    limit_y2 =  digitalRead(LIMIT_SWITCH_Y2_PIN);
   }   
   stopMotors(MOTOR_X_Y);
   limit_x2 = 1; limit_y2 = 0;
   Serial.print("X max:"); Serial.println(encoder_x_position);
   Serial.print("Y max:"); Serial.println(encoder_y_position);
//   
//   //attachInterrupt(2, stopX2, CHANGE);
//   //attachInterrupt(3, stopX1, CHANGE);
//   //attachInterrupt(4, stopY2, RISING);
//   //attachInterrupt(5, stopY1, RISING);
//}
//
}

void loop()
{
  limit_x1 = 1; limit_y1 = 0;
  moveMotor(MOTOR_X,DIRECTION_MINUS);
  moveMotor(MOTOR_Y,DIRECTION_MINUS);
  
   while(limit_x1 || !limit_y1){
    if(limit_x1 == 0)
     stopMotors(MOTOR_X);
    if(limit_y1 == 1)
     stopMotors(MOTOR_Y);
    limit_x1 =  digitalRead(LIMIT_SWITCH_X1_PIN);
    limit_y1 =  digitalRead(LIMIT_SWITCH_Y1_PIN);
   }
   stopMotors(MOTOR_X_Y);
      
   Serial.print("X min:"); Serial.println(encoder_x_position);
   Serial.print("Y min:"); Serial.println(encoder_y_position);
   
             
   moveMotor(MOTOR_X,DIRECTION_PLUS);
   moveMotor(MOTOR_Y,DIRECTION_PLUS);
      
   limit_x2 = 1; limit_y2 = 0;
   while(limit_x2 || !limit_y2){
    if(limit_x2 == 0)
     stopMotors(MOTOR_X);     
    if(limit_y2 == 1)
      stopMotors(MOTOR_Y);
    limit_x2 =  digitalRead(LIMIT_SWITCH_X2_PIN);
    limit_y2 =  digitalRead(LIMIT_SWITCH_Y2_PIN);
   }   
   stopMotors(MOTOR_X_Y);
     
   Serial.print("X max:"); Serial.println(encoder_x_position);
   Serial.print("Y max:"); Serial.println(encoder_y_position);
   
  /*if (Serial.available()){
    inByte = Serial.read();
    Serial.println(inByte);  
    if(inByte==49){
      Serial.println("x_plus");
      moveMotor(MOTOR_X,DIRECTION_PLUS);
    }
    else if(inByte==50){
       Serial.println("x_minus");
      moveMotor(MOTOR_X,DIRECTION_MINUS);
    }
    else if(inByte==51){
          Serial.println("y_plus");
      moveMotor(MOTOR_Y,DIRECTION_PLUS);
    }
    else if(inByte==52){
          Serial.println("y_minus");
      moveMotor(MOTOR_Y,DIRECTION_MINUS);
    }
    else
    {
      stopMotors(MOTOR_X_Y);
    }
  }
  
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
             stopMotors(0);
             break;
           case IMERGENCY_STOP:
             stopMotors(0);
             break;
           case MOVE_TO_INITIAL_POSITION:
             if(reachedX1)
               stopMotors(MOTOR_X);
             else
               moveMotor(MOTOR_X,DIRECTION_MINUS);
               
             if(reachedY1)
               stopMotors(MOTOR_Y);
             else
               moveMotor(MOTOR_Y,DIRECTION_MINUS);
               
             if(reachedX1 && reachedY1)
             {
               encoder_x_position = 0;
               encoder_x_position = 0;
               operation_status = SUCCESS;
               command = MOVING_TO_NEXT_TILE;
             }
             break;
           case MOVING_TO_NEXT_TILE:
               moveMotorSteps(motor,_direction,steps);
             break;
           case FOCUS_LENSE:
               doFocus(_direction,steps);
             break;   
        }
     break;
   case AUTOMATIC_MODE:
     switch(command){
       case STOP:
         stopMotors(0);
         break;
       case IMERGENCY_STOP:
         stopMotors(0);
         break;
       case MOVE_TO_INITIAL_POSITION:
         if(reachedX1)
               stopMotors(MOTOR_X);
         if(reachedY1)
           stopMotors(MOTOR_Y);
         if(reachedX1 && reachedY1)
         {
           operation_status = SUCCESS;
           command = MOVE_TO_INITIAL_POSITION;
         }
         break;
       case MOVING_TO_NEXT_TILE:
           moveMotorSteps(motor,_direction,steps);
         break;
       case FOCUS_LENSE:
           doFocus(_direction,steps);
         break;    
     }
     sendResponce();
     operation_status = FAILED;
     break;   
 }*/
}

void initializeToZero()
{
    moveMotor(MOTOR_X,DIRECTION_MINUS);
    moveMotor(MOTOR_Y,DIRECTION_MINUS);
}

void moveMotorSteps(int motor,int _direction,int steps)
{
  switch (motor){
    case MOTOR_X:     
      stopMotors(0);
      break;
    case MOTOR_Y:
      stopMotors(0);
      break;
  } 
}

void moveMotor(int motor,int _direction)
{
    switch (motor){
    case MOTOR_X:     
       switch(_direction){
           case DIRECTION_PLUS : 
             digitalWrite(MOTOR_X_PIN_1,HIGH);
             digitalWrite(MOTOR_X_PIN_2,LOW);
             break;
           case DIRECTION_MINUS:
             digitalWrite(MOTOR_X_PIN_1,LOW);
             digitalWrite(MOTOR_X_PIN_2,HIGH);
             break;
       }       
      break;
    case MOTOR_Y:
      switch(_direction){
           case DIRECTION_PLUS : 
             digitalWrite(MOTOR_Y_PIN_1,LOW);
             digitalWrite(MOTOR_Y_PIN_2,HIGH);
             break;
           case DIRECTION_MINUS:
             digitalWrite(MOTOR_Y_PIN_1,HIGH);
             digitalWrite(MOTOR_Y_PIN_2,LOW);
             break;
       } 
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

void initializeTo(int blockNo)
{
  
}

void stopMotors(int motor){
  switch(motor){
    case MOTOR_X:
      digitalWrite(MOTOR_X_PIN_1,LOW);
      digitalWrite(MOTOR_X_PIN_2,LOW);
      break;
    case MOTOR_Y:
      digitalWrite(MOTOR_Y_PIN_1,LOW);
      digitalWrite(MOTOR_Y_PIN_2,LOW);
      break;
    case MOTOR_X_Y:
    
    default://stop both  motors
      digitalWrite(MOTOR_X_PIN_1,LOW);
      digitalWrite(MOTOR_X_PIN_2,LOW);
      digitalWrite(MOTOR_Y_PIN_1,LOW);
      digitalWrite(MOTOR_Y_PIN_2,LOW);
      break;
  }
}

void doEncoderX()
{
   if (digitalRead(ENCODER_X_PIN_A) == digitalRead(ENCODER_X_PIN_B)) {
    encoder_x_position++;
    current_motor_x_direction = DIRECTION_PLUS;
  } else {
    encoder_x_position--;
    current_motor_x_direction = DIRECTION_MINUS;
  }
}

void doEncoderY(){
 if (digitalRead(ENCODER_Y_PIN_A) == digitalRead(ENCODER_Y_PIN_B)) {
    encoder_y_position++;
    current_motor_y_direction = DIRECTION_PLUS;
  } else {
    encoder_y_position--;
    current_motor_y_direction = DIRECTION_MINUS;
  }
}

void stopX1()
{
  reachedX1 = 1;
  stopMotors(MOTOR_X);
//  Serial.println("x1 changed");
}

void stopX2(){
    reachedX2 = 1;
    stopMotors(MOTOR_X);
   // Serial.println("x2 changed");
}

void stopY1(){
    reachedY1 = 1;
    stopMotors(MOTOR_Y);
   // Serial.println("y1 changed");
}

void stopY2(){
  reachedY2 = 1;
  stopMotors(MOTOR_Y);
 // Serial.println("y2 changed");
}

void writeStatusToEEPROM()
{
    EEPROM.write(addr, current_tile_position);
}
