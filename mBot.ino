#include <MeAurigaLab.h>
#include "scheduler.h"

int MAX_SPEED = 100;
float ULTRA_SENSE = 0;
int LINE_READ = 3;
int PREV_LINE_READ = 3;
int COUNTER = 0;
int TIME_JANNE = 35;

MeLineFollower lineFinder(PORT_9);
MeEncoderOnBoard left(SLOT2);
MeEncoderOnBoard right(SLOT1);  // Spegelvänd
MeUltrasonicSensor ultraSensor(PORT_7);

TaskHandle_t lineFollowHandle = NULL, avoidObstacleHandle = NULL;

void TaskLineFollow(void *pvParameters) {
  LINE_READ = lineFinder.readSensors();

  if(COUNTER == 0) {
    switch(LINE_READ){
    case 0: // if 0 åk framåt (båda motorerna) BÅDA SVARTA
      left.setMotorPwm(MAX_SPEED);
      right.setMotorPwm(-MAX_SPEED);
      break;
    case 1:  // if 1 sväng vänster (gasa med höger motor)
      left.setMotorPwm(0);
      right.setMotorPwm(-MAX_SPEED);
      PREV_LINE_READ = LINE_READ;
      break;
    case 2:  // if 2 sväng höger (gasa med vänster motor)
      left.setMotorPwm(MAX_SPEED);
      right.setMotorPwm(0);
      PREV_LINE_READ = LINE_READ;
      break;
    case 3: // if 3 båda vita
      if(PREV_LINE_READ == 2){
        left.setMotorPwm(MAX_SPEED);
        right.setMotorPwm(0);
      } else if (PREV_LINE_READ == 1) {
        left.setMotorPwm(0);
        right.setMotorPwm(-MAX_SPEED);
      } 
      break;
    }
  }
}

void TaskAvoidObstacle(void *pvParameters) {

  ULTRA_SENSE = ultraSensor.distanceCm();
  
  if (ULTRA_SENSE < 20.00 || COUNTER > 0) {
    COUNTER++;

    if (LINE_READ != 3 && COUNTER > TIME_JANNE) {
      PREV_LINE_READ = 1;
      COUNTER = 0;
    }
    
    if (COUNTER < TIME_JANNE) {
      left.setMotorPwm(-MAX_SPEED);
      right.setMotorPwm(-MAX_SPEED);
    }
    else if (COUNTER < TIME_JANNE*2) {
      left.setMotorPwm(MAX_SPEED);
      right.setMotorPwm(-(MAX_SPEED/1.8));
    }
    Serial.println(COUNTER);
  } 
}

void setup() {
  Serial.begin(9600);
  left.setMotionMode(DIRECT_MODE);
  right.setMotionMode(DIRECT_MODE);
  vSchedulerInit();

  vSchedulerPeriodicTaskCreate(
    TaskLineFollow, // Task function
    "lineFollow",   // Task name
    configMINIMAL_STACK_SIZE, // Stack size
    NULL, // Parameters if any
    0,    // Task priority (only used when scheduling policy is manual)
    &lineFollowHandle, // Pointer to the task handle
    0,    // Task time phase
    pdMS_TO_TICKS(20), // Task period
    pdMS_TO_TICKS(100), // Worst-case time
    pdMS_TO_TICKS(20)   // Relative deadline
  );

  vSchedulerPeriodicTaskCreate(
    TaskAvoidObstacle, // Task function
    "avoidObstacle",   // Task name
    configMINIMAL_STACK_SIZE, // Stack size
    NULL, // Parameters if any
    0,    // Task priority (only used when scheduling policy is manual)
    &avoidObstacleHandle, // Pointer to the task handle
    0,    // Task time phase
    pdMS_TO_TICKS(20), // Task period
    pdMS_TO_TICKS(100), // Worst-case time
    pdMS_TO_TICKS(20)   // Relative deadline
  );

  vSchedulerStart();
}

void loop() {
}
