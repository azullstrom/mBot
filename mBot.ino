#include <MeAurigaLab.h>
#include "scheduler.h"

int MAX_SPEED = 100;
float ULTRA_SENSE = 0;
int LINE_READ = 3;
int PREV_LINE_READ = 3;

MeLineFollower lineFinder(PORT_9);
MeEncoderOnBoard left(SLOT2);
MeEncoderOnBoard right(SLOT1); // Spegelvänd
MeUltrasonicSensor ultraSensor(PORT_7);

TaskHandle_t lineFollowHandle = NULL, avoidObstacleHandle = NULL;

void TaskLineFollow(void *pvParameters) {
  while (true) {
    LINE_READ = lineFinder.readSensors();

    switch(LINE_READ){
    case 0: // if 0 åk framåt (båda motorerna) BÅDA SVARTA
      left.setMotorPwm(MAX_SPEED);
      right.setMotorPwm(-MAX_SPEED);
      break;
    case 1: // if 1 sväng vänster (gasa med höger motor)
      left.setMotorPwm(0);
      right.setMotorPwm(-MAX_SPEED);
      PREV_LINE_READ = LINE_READ;
      break;
    case 2: // if 2 sväng höger (gasa med vänster motor)
      left.setMotorPwm(MAX_SPEED);
      right.setMotorPwm(0);
      PREV_LINE_READ = LINE_READ;
      break;
    case 3: // if 3 båda vita
      if(PREV_LINE_READ == 2){
        left.setMotorPwm(MAX_SPEED);
        right.setMotorPwm(0);
      }else if(PREV_LINE_READ == 1){
        left.setMotorPwm(0);
        right.setMotorPwm(-MAX_SPEED);
      } 
      break;
    }
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}

void TaskAvoidObstacle(void *pvParameters) {
  while (true) {
    ULTRA_SENSE = ultraSensor.distanceCm();
    
    if (ULTRA_SENSE < 40.00) {
      // Obstacle detected, take avoiding action
      left.setMotorPwm(MAX_SPEED / 2);  // Example: Slow down left motor
      right.setMotorPwm(-MAX_SPEED / 2); // Example: Slow down right motor
    } else {
      // No obstacle, continue avoiding behavior
      left.setMotorPwm(MAX_SPEED);  // Example: Move forward
      right.setMotorPwm(-MAX_SPEED); // Example: Move forward
    }

    // Check if the line is detected, and if so, switch back to line following
    LINE_READ = lineFinder.readSensors();
    if (LINE_READ != 3) {
      vTaskSuspend(avoidObstacleHandle);
      vTaskResume(lineFollowHandle);
    }

    vTaskDelay(pdMS_TO_TICKS(10));
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
