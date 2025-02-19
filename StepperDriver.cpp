#include "StepperDriver.h"



StepperDriver::StepperDriver(int number_of_steps, int step_division, int dir_pin, int step_pin, int min_sensor_pin, int max_sensor_pin, char axis)
{
  this->number_of_steps = number_of_steps;
  this->step_division = step_division;
  this->step_interval = 10000; // microseconds between pulse?

  this->step_counter = 0;
  this->steps_to_move = 0;
  
  // Arduino pins for the motor control connection:
  this->dir_pin = dir_pin;
  this->step_pin = step_pin;

  this->min_sensor_pin = min_sensor_pin;
  this->max_sensor_pin = max_sensor_pin;

  this->axis = axis;

  this->pin_state = LOW;

  this->is_timer_active = false;

  // setup the pins on the microcontroller:
  pinMode(dir_pin, OUTPUT);
  pinMode(step_pin, OUTPUT);
}



bool StepperDriver::isTimerActive()
{
  return this->is_timer_active;
}



void StepperDriver::initMinMaxSensors()
{
  // INPUT ? INPUT_PULLUP?
  // pinMode(this->min_sensor_pin, INPUT);
  // pinMode(this->max_sensor_pin, INPUT);
  pinMode(this->min_sensor_pin, INPUT_PULLUP);
  pinMode(this->max_sensor_pin, INPUT_PULLUP);
}



void StepperDriver::setSpeed(float rpm)
{
  if (rpm < 60){return;} // at least 1 rev/sec needed
  Serial.print("set speed ");
  Serial.print(rpm);
  // useconds
  step_interval = 60000000L / (number_of_steps * rpm * step_division);
  Serial.print(" -> interval of ");
  Serial.println(step_interval);
}



void StepperDriver::setDirection(long steps_to_move)
{
  if (steps_to_move > 0) {
    digitalWrite(dir_pin, HIGH);
    moveDir = true;
  }
  else {
    digitalWrite(dir_pin, LOW);
    moveDir = false;
  }
}



void StepperDriver::cancelStep()
{
  if (timer){
    cancel_repeating_timer(timer);
    delete timer;
    Serial.println("clear timer");
    is_timer_active = false;
  }
  digitalWrite(step_pin, LOW); // init pin state
}



void StepperDriver::step(long steps)
{
  cancelStep();
  timer = new repeating_timer_t;
  timer->user_data = (void *)this;

  steps *= step_division;
  steps *= 2; // timer will trigger 2x (because it uses toggle)
  steps_to_move = steps;
  Serial.printf("total call count: %d (interval %d us)\n", steps_to_move, step_interval);
  setDirection(steps_to_move); // moveDir=true -> PLUS, false -> MINUS
  // read min/max sensor state and decide move or not
  if (moveDir && digitalRead(max_sensor_pin) == LOW) {
    Serial.printf("CANNOT MOVE ABOVE MAX\n");
    return; // if max sensor is active (motor already at max end), avoid move +
  }
  if (!moveDir && digitalRead(min_sensor_pin) == LOW) {
    Serial.printf("CANNOT MOVE BELOW MIN\n");
    return; // if min sensor is active (motor already at min end), avoid move -
  }

  step_counter = 0;

  digitalWrite(step_pin, LOW); // reset pin state
  pin_state = LOW; // set current pin state
  add_repeating_timer_us(-1*(long)(step_interval>>1), move, (void *)this, timer);
  is_timer_active = true;
}



bool StepperDriver::move(repeating_timer_t *t)
{
  StepperDriver *_this = reinterpret_cast<StepperDriver *>(t->user_data);

  if (_this->moveDir && digitalRead(_this->max_sensor_pin) == LOW) {
    _this->cancelStep();
    Serial.printf("MAX LIMIT HIT! at %d\n", _this->step_counter);
  }
  if (!(_this->moveDir) && digitalRead(_this->min_sensor_pin) == LOW) {
    _this->cancelStep();
    Serial.printf("MIN LIMIT HIT! at %d\n", _this->step_counter);
  }

  // toggle output
  digitalWrite(_this->step_pin, !(_this->pin_state));
  _this->pin_state = !(_this->pin_state); // update pin state

  (_this->step_counter)++;
  if (_this->step_counter >= abs(_this->steps_to_move)) {
    Serial.printf("%c axis - count finished %d\n", _this->axis, _this->step_counter);
    // TODO replace with cancelStep?
    _this->is_timer_active = false;
    digitalWrite(_this->step_pin, LOW); // reset pin state to low
    return false; // return false -> timer stops
  }

  return true; // otherwise, keep timer running
}