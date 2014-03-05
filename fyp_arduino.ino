#include <EEPROM.h>

#define MANUAL_MODE              22
#define AUTOMATIC_MODE           23

#define CLOCKWISE                1
#define COUNTER_CLOCKWISE       -1
#define MOTOR_X                  15
#define MOTOR_Y                  16 
#define MOTOR_X_Y                17
#define DIRECTION_PLUS           50
#define DIRECTION_MINUS          51

#define STOP                     0
#define EMERGENCY_STOP           1
#define MOVE_TO_INITIAL_POSITION 2
#define MOVING_TO_NEXT_TILE      3
#define FOCUS_LENSE              4

#define MOTOR_X_PIN_1            22
#define MOTOR_X_PIN_2            23
#define MOTOR_X_PWM_PIN          8

#define MOTOR_X_PWM              255
#define MOTOR_Y_PWM              255

#define MOTOR_Y_PIN_1            24
#define MOTOR_Y_PIN_2            25
#define MOTOR_Y_PWM_PIN          9

#define ENCODER_X_PIN_A          3  
#define ENCODER_X_PIN_B          41
#define ENCODER_Y_PIN_A          2
#define ENCODER_Y_PIN_B          40

#define LIMIT_SWITCH_X1_PIN      20 //int.3
#define LIMIT_SWITCH_X2_PIN      21 //int.2
#define LIMIT_SWITCH_Y1_PIN      18 //int.5
#define LIMIT_SWITCH_Y2_PIN      19 //int.4

/*STATUS CODES*/
#define SUCCESS                  30
#define FAILED                   55
#define PENDING                  40

int current_motor_x_direction       = -1;
int current_motor_y_direction       = -1;

volatile int encoder_x_position      = 0;
volatile int encoder_y_position      = 0;
int encoder_x_max = 100;
int encoder_y_max = 100;

int horizontal_block_length = 0;
int vertical_block_length   = 0;

int current_tile_position   = -1;
int motor                   = MOTOR_X;
int _direction              = CLOCKWISE;
int steps                   = 200;

int addr                    = 0;
int operation_status        = FAILED;
int sequence_no				= 0;
String responce             = "";

int mode =-1,command=-1,opt1=-1,opt2=-1,opt3=-1;

int inByte  = -1;
volatile int reachedX1 = 0, reachedX2 = 0, reachedY1 = 0, reachedY2 = 0;

int initBlockX = 0;
  
int block_2_x1 = 182,block_3_x1 = 364;

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
   encoder_x_max = encoder_x_position; encoder_y_max = encoder_y_position;
   limit_x2 = 1; limit_y2 = 0; 
   Serial.print("X max:"); Serial.println(encoder_x_position);
   Serial.print("Y max:"); Serial.println(encoder_y_position);
   
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
   
   //attachInterrupt(2, stopX2, CHANGE);
   attachInterrupt(3, stopX1, CHANGE);
   attachInterrupt(4, stopY2, RISING);
   attachInterrupt(5, stopY1, RISING);
   encoder_x_position = 0;
   encoder_y_position = 0;
}

void loop()
{ 

  int sequenceNo = 0;
  while(Serial.available() > 0) {
   sequenceNo  = Serial.parseInt();
   mode    = Serial.parseInt();
   command = Serial.parseInt(); 
   opt1    = Serial.parseInt();  // scanning block
   opt2    = Serial.parseInt(); 
   opt3    = Serial.parseInt();
   initBlockX = getInitBlockX(opt1);
 }
 
 if(digitalRead(LIMIT_SWITCH_X2_PIN)==0)
  stopMotors(MOTOR_X);
 
 switch (mode)
 {
   case MANUAL_MODE:   
   
     break;
   case AUTOMATIC_MODE:
     switch(command){
       case STOP:
         stopMotors(0);
         break;
       case EMERGENCY_STOP:
         stopMotors(0);
         break;
       case MOVE_TO_INITIAL_POSITION:
			if(encoder_x_position < initBlockX)
				moveMotor(MOTOR_X,DIRECTION_PLUS);
			else if(encoder_x_position > initBlockX)
				moveMotor(MOTOR_X,DIRECTION_MINUS);
			else
				stopMotors(MOTOR_X);
			
			if(encoder_y_position > 0)
				moveMotor(MOTOR_Y,DIRECTION_MINUS);
			else
				stopMotors(MOTOR_Y);
		
			if(encoder_x_position == initBlockX && encoder_y_position <=0){
				stopMotors(MOTOR_X_Y);
				operation_status = SUCCESS;
				sendResponce();
			}			
			break;
       case MOVING_TO_NEXT_TILE:
           moveMotorSteps(opt1,opt2,opt3);
         break;
      case PENDING:
	     break;
       case FOCUS_LENSE:
           doFocus(_direction,steps);
         break;    
     }
    // sendResponce();
    // operation_status = FAILED;
     break;   
 }
}

int getInitBlockX(int blockNo){
  switch(blockNo)
  {
     case 1:
       return 0;
     case 2:
       return block_2_x1;
     case 3:
       return block_3_x1;
  }
}

void initializeToZero()
{
    moveMotor(MOTOR_X,DIRECTION_MINUS);
    moveMotor(MOTOR_Y,DIRECTION_MINUS);
}

void moveMotorSteps(int motor,int _direction,int steps)
{
 int current_x_position  = encoder_x_position;
 int current_y_position  = encoder_y_position;
  switch (motor){
    case MOTOR_X:     
	  moveMotor(MOTOR_X,_direction); 
	  while(abs(current_x_position+steps - encoder_x_position)> 0)
	  { ; }
	  operation_status = SUCCESS;
      stopMotors(MOTOR_X);
      break;
    case MOTOR_Y:
      moveMotor(MOTOR_Y,_direction);
	  while(abs(current_y_position+steps-encoder_y_position)> 0)
	  { ; }
	  operation_status = SUCCESS;
      stopMotors(MOTOR_Y);
      break;
  } 
  sendResponce();
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
  responce = "";
  responce.concat(sequence_no);
  responce.concat(":"); 
  responce.concat(operation_status);
  responce.concat(":"); 
  responce.concat(command);
  Serial.println(responce);
  command = PENDING;
  operation_status = FAILED;
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
