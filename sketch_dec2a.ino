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

#include <FastLED.h>
#define NUM_LEDS 30
#define DATA_PIN 8
CRGB leds[NUM_LEDS];

unsigned long ct;
bool status_led;

int motor_steps = 200; // motor has 200 steps/rev, meaning 1 step is 1.8deg
int step_division = 8; // driver has 1600 pulse/rev, meaning 8 pulse/step

// x axis
int x_dir_pin = 2;
int x_step_pin = 3;
// terminal sensor
int x_min_sensor = -1;
int x_max_sensor = -1;

// y axis
int y_dir_pin = 4;
int y_step_pin = 5;
// terminal sensor
int y_min_sensor = -1;
int y_max_sensor = -1;

// z axis
int z_dir_pin = 6;
int z_step_pin = 7;
// terminal sensor
int z_min_sensor = -1;
int z_max_sensor = -1;

StepperDriver x_stepper(motor_steps, step_division, x_dir_pin, x_step_pin, 'x');
StepperDriver y_stepper(motor_steps, step_division, y_dir_pin, y_step_pin, 'y');
StepperDriver z_stepper(motor_steps, step_division, z_dir_pin, z_step_pin, 'z');



void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  delay(10000);
  // put your setup code here, to run once:
  Serial.begin(115200); //usb serial
  // Serial1.begin(115200); // GPIO serial (GPIO tx-0/rx-1)

  // set speed
  x_stepper.setSpeed(480.0); // rpm
  y_stepper.setSpeed(480.0); // rpm
  z_stepper.setSpeed(480.0); // rpm

  delay(500);
  digitalWrite(LED_BUILTIN, LOW);
  Serial.println("done setup");

  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);
  FastLED.setMaxPowerInMilliWatts(2000);
  FastLED.setBrightness(32);
  ct = millis();
  status_led = false;
}

void setup1() {
  // 2nd core
}

void loop1() {
  // 2nd core
}

void loop() {
  if (millis() - ct >= 500) {
    ct = millis();
    if (status_led) {
      leds[4] = CRGB::Black;
      leds[5] = CRGB::Black;
      leds[6] = CRGB::Black;
      status_led = false;
      // Serial.printf("tick\n");
    } else {
      leds[4] = CRGB::Orange;
      leds[5] = CRGB::Orange;
      leds[6] = CRGB::Orange;
      status_led = true;
      // Serial.printf("TOCK\n");
    }
  }
  if (Serial.available() > 0) {
    // incoming data like x:1000\n
    String incomingData = Serial.readStringUntil('\n');
    incomingData.replace("\n", "");
    char* tmp = strtok((char*)incomingData.c_str(), ":");
    char cmd = tmp[0]; // tri-axis (x,y,z) or speed (s) or pause (p)
    int val = atoi(strtok(NULL, " ")); // command value
    Serial.printf("input %c val %d\n", cmd, val);

    if (cmd == 'x') {
      x_stepper.step(val);
      leds[0] = CRGB::Pink;FastLED.show();
    } else if (cmd == 'y') {
      y_stepper.step(val);
      leds[1] = CRGB::Pink;FastLED.show();
    } else if (cmd == 'z') {
      z_stepper.step(val);
      leds[2] = CRGB::Pink;FastLED.show();
    } else if (cmd == 's') { // speed
      x_stepper.setSpeed((float)val); // rpm
      y_stepper.setSpeed((float)val); // rpm
      z_stepper.setSpeed((float)val); // rpm
    } else if (cmd == 'p') { // pause
      x_stepper.cancelStep();
      y_stepper.cancelStep();
      z_stepper.cancelStep();
    }
  }
  // indicate stepper status in LED
  x_stepper.isTimerActive() ? leds[0]=CRGB::Red : leds[0]=CRGB::Black;
  y_stepper.isTimerActive() ? leds[1]=CRGB::Green : leds[1]=CRGB::Black;
  z_stepper.isTimerActive() ? leds[2]=CRGB::Blue : leds[2]=CRGB::Black;
  FastLED.show();
}
