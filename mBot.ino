#include <MeAurigaLab.h>
#include "scheduler.h"

int MAX_SPEED = 80;
float ULTRA_SENSE = 0;
int LINE_READ = 3;
int PREV_LINE_READ = 3;

MeLineFollower lineFinder(PORT_9);
MeEncoderOnBoard left(SLOT2);
MeEncoderOnBoard right(SLOT1); // Spegelvänd
MeUltrasonicSensor ultraSensor(PORT_7);

MeBuzzer buzzer;
TaskHandle_t blipHandle = NULL, motorHandle = NULL, senseHandle = NULL, ultraHandle = NULL;
char c1 = 'a';

void TaskBlip(void *pvParameters) {
  // buzzer.tone(400, 100);
  // buzzer.tone(500, 100);
  // buzzer.tone(600, 100);
  // buzzer.tone(700, 100);
}

void TaskMotor(void *pvParameters) {
  Serial.print("Line: ");
  Serial.println(LINE_READ);
  Serial.print("Ultra: ");
  Serial.println(ULTRA_SENSE);

  switch(LINE_READ){
    case 0: // if 0 åk framåt (båda motorerna) BÅDA SVARTA
      left.setMotorPwm(MAX_SPEED);
      right.setMotorPwm(-MAX_SPEED);
      PREV_LINE_READ = LINE_READ;
      break;
    case 1: // if 1 sväng vänster (gasa med höger motor)
      left.setMotorPwm(0);
      right.setMotorPwm(-(MAX_SPEED + 5));
      PREV_LINE_READ = LINE_READ;
      break;
    case 2: // if 2 sväng höger (gasa med vänster motor)
      left.setMotorPwm(MAX_SPEED + 5);
      right.setMotorPwm(0);
      PREV_LINE_READ = LINE_READ;
      break;
    case 3: // if 3 båda vita ()
      switch(PREV_LINE_READ){
        case 0: // Här måste vi lägga en liten task-janne
          left.setMotorPwm(MAX_SPEED);
          right.setMotorPwm(0);
          PREV_LINE_READ = LINE_READ;
          break;
        case 1:
          left.setMotorPwm(0);
          right.setMotorPwm(-(MAX_SPEED + 5));
          PREV_LINE_READ = LINE_READ;
          break;
        case 2:
          left.setMotorPwm(MAX_SPEED + 5);
          right.setMotorPwm(0);
          PREV_LINE_READ = LINE_READ;
          break;
        case 3:
          left.setMotorPwm(MAX_SPEED);
          right.setMotorPwm(-MAX_SPEED);
          break;
      }
      PREV_LINE_READ = LINE_READ;
  }
}
//   // if 0 åk framåt (båda motorerna)
//   if(LINE_READ == 0) {
//     left.setMotorPwm(MAX_SPEED);
//     right.setMotorPwm(-MAX_SPEED);
//   }

//   // if 1 sväng vänster (gasa med höger motor)
//   if(LINE_READ == 1) {
//     left.setMotorPwm(0);
//     right.setMotorPwm(-(MAX_SPEED + 5));
//   }

//   // if 2 sväng höger (gasa med vänster motor)
//   if(LINE_READ == 2) {
//     left.setMotorPwm(MAX_SPEED + 5);
//     right.setMotorPwm(0);
//   }

//   // if 3
//   if(LINE_READ == 3) {
    
//   }
// }

void TaskUltra(){
  if(ULTRA_SENSE < 25.00) {
    left.setMotorPwm(0);
    right.setMotorPwm(0);
  }
}

void TaskSense() {
  LINE_READ = lineFinder.readSensors();
  ULTRA_SENSE = ultraSensor.distanceCm();
}

void setup() {
  Serial.begin(9600);
  left.setMotionMode(DIRECT_MODE);
  right.setMotionMode(DIRECT_MODE);

  vSchedulerInit();
  // vSchedulerPeriodicTaskCreate(TaskBlip, "blip", configMINIMAL_STACK_SIZE, &c1, 1, &blipHandle, pdMS_TO_TICKS(0), pdMS_TO_TICKS(400), pdMS_TO_TICKS(100), pdMS_TO_TICKS(400));
  vSchedulerPeriodicTaskCreate(TaskMotor, "motor", configMINIMAL_STACK_SIZE, &c1, 1, &motorHandle, pdMS_TO_TICKS(0), pdMS_TO_TICKS(50), pdMS_TO_TICKS(100), pdMS_TO_TICKS(50));
  vSchedulerPeriodicTaskCreate(TaskSense, "sense", configMINIMAL_STACK_SIZE, &c1, 1, &senseHandle, pdMS_TO_TICKS(0), pdMS_TO_TICKS(50), pdMS_TO_TICKS(100), pdMS_TO_TICKS(50));
  vSchedulerPeriodicTaskCreate(TaskUltra, "ultra", configMINIMAL_STACK_SIZE, &c1, 1, &ultraHandle, pdMS_TO_TICKS(0), pdMS_TO_TICKS(50), pdMS_TO_TICKS(100), pdMS_TO_TICKS(50));
  vSchedulerStart();
}

void loop() {
}
