/*
 ____  _   ____ ___ ____ ___
|  _ \(_) |  _ \_ _/ ___/ _ \
| |_) | | | |_) | | |  | | | |
|  __/| | |  __/| | |__| |_| |
|_|   |_| |_|  |___\____\___/

 __  __       _             ____       _
|  \/  | ___ | |_ ___  _ __|  _ \ _ __(_)_   _____
| |\/| |/ _ \| __/ _ \| '__| | | | '__| \ \ / / _ \
| |  | | (_) | || (_) | |  | |_| | |  | |\ V /  __/
|_|  |_|\___/ \__\___/|_|  |____/|_|  |_| \_/ \___|
*/
#include "StepperDriver.h"
#include <string.h>

int motor_steps = 200; // motor has 200 steps/rev, meaning 1 step is 1.8deg
int step_division = 8; // driver has 1600 pulse/rev, meaning 8 pulse/step

// x axis
int x_dir_pin = 2;
int x_step_pin = 3;
// y axis
int y_dir_pin = 4;
int y_step_pin = 5;
// z axis
int z_dir_pin = 6;
int z_step_pin = 7;

StepperDriver x_stepper(motor_steps, step_division, x_dir_pin, x_step_pin);
StepperDriver y_stepper(motor_steps, step_division, y_dir_pin, y_step_pin);
StepperDriver z_stepper(motor_steps, step_division, z_dir_pin, z_step_pin);

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  // put your setup code here, to run once:
  Serial.begin(115200); //usb serial
  // Serial1.begin(115200); // GPIO serial (GPIO tx-0/rx-1)

  // set speed
  x_stepper.setSpeed(480.0); // rpm
  y_stepper.setSpeed(480.0); // rpm
  z_stepper.setSpeed(480.0); // rpm
}

void loop() {
  if (Serial.available() > 0) {
    // incoming data like x:1000\n
    String incomingData = Serial.readStringUntil('\n');
    incomingData.replace("\n", "");
    char* tmp = strtok((char*)incomingData.c_str(), ":");
    char cmd = tmp[0];
    int val = atoi(strtok(NULL, " "));
    Serial.printf("input %c val %d\n", cmd, val);

    if (cmd == 'x') {
      x_stepper.step(val);
    } else if (cmd == 'y') {
      y_stepper.step(val);
    } else if (cmd == 'z') {
      z_stepper.step(val);
    } else if (cmd == 's') {
      x_stepper.setSpeed((float)val); // rpm
      y_stepper.setSpeed((float)val); // rpm
      z_stepper.setSpeed((float)val); // rpm
    }
  }
}
