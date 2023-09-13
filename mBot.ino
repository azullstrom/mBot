#include <MeAurigaLab.h>
#include "scheduler.h"

int MAX_SPEED = 100;
float ULTRA_SENSE = 0;
int LINE_READ = 3;
int PREV_LINE_READ = 3;

MeLineFollower lineFinder(PORT_9);
MeEncoderOnBoard left(SLOT2);
MeEncoderOnBoard right(SLOT1);  // Spegelvänd
MeUltrasonicSensor ultraSensor(PORT_7);

MeBuzzer buzzer;
TaskHandle_t blipHandle = NULL, motorHandle = NULL, senseHandle = NULL, ultraHandle = NULL;
char c1 = 'a';

void TaskBlip(void *pvParameters) {
  buzzer.tone(400, 100);
  buzzer.tone(500, 100);
  buzzer.tone(600, 100);
  buzzer.tone(700, 100);
}

void TaskMotor(void *pvParameters) {
  Serial.print("Line: ");
  Serial.println(LINE_READ);
  Serial.print("Ultra: ");
  Serial.println(ULTRA_SENSE);
  // LINE_READ = lineFinder.readSensors();

  switch (LINE_READ) {
    case 0:  // if 0 åk framåt (båda motorerna) BÅDA SVARTA
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
    case 3:  // if 3 båda vita ()
      if (PREV_LINE_READ == 2) {
        left.setMotorPwm(MAX_SPEED);
        right.setMotorPwm(0);
      } else if (PREV_LINE_READ == 1) {
        left.setMotorPwm(0);
        right.setMotorPwm(-MAX_SPEED);
      } else {}
      break;
  }
}

void TaskUltra() {
  // ULTRA_SENSE = ultraSensor.distanceCm();
  if (ULTRA_SENSE < 40.00) {
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
  //                                                                                              Processens tidsfas, Processens period, Worst-case tid,    Relativ deadline
  // vSchedulerPeriodicTaskCreate(TaskBlip, "blip", configMINIMAL_STACK_SIZE, &c1, 1, &blipHandle, pdMS_TO_TICKS(0), pdMS_TO_TICKS(400), pdMS_TO_TICKS(100), pdMS_TO_TICKS(400));
  vSchedulerPeriodicTaskCreate(TaskMotor, "motor", configMINIMAL_STACK_SIZE, &c1, 1, &motorHandle, pdMS_TO_TICKS(0), pdMS_TO_TICKS(100), pdMS_TO_TICKS(100), pdMS_TO_TICKS(100));
  vSchedulerPeriodicTaskCreate(TaskSense, "sense", configMINIMAL_STACK_SIZE, &c1, 1, &senseHandle, pdMS_TO_TICKS(0), pdMS_TO_TICKS(10), pdMS_TO_TICKS(100), pdMS_TO_TICKS(10));
  vSchedulerPeriodicTaskCreate(TaskUltra, "ultra", configMINIMAL_STACK_SIZE, &c1, 1, &ultraHandle, pdMS_TO_TICKS(0), pdMS_TO_TICKS(50), pdMS_TO_TICKS(100), pdMS_TO_TICKS(50));
  vSchedulerStart();
}

//vSchedulerPeriodicTaskCreate(Vilken funktion som ska köras,
//                             Vad heter tasken,
//                             Hur stor stack,
//                             Parameter som skickas med funktionen,
//                             Process prioritet,
//                             Pekare till task-objektet,

//                             Processens tidsfas(oftast 0),
//                             Processens period
//                             Worst-case exekveringstid
//                             Relativ Deadline)
void loop() {
}
