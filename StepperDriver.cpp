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

  this->current_pos = -1;
  this->max_pos = -1;

  this->axis = axis;

  this->pin_state = LOW;

  this->is_timer_active = false;

  // setup the pins on the microcontroller:
  pinMode(dir_pin, OUTPUT);
  pinMode(step_pin, OUTPUT);

  // INPUT ? INPUT_PULLUP?
  pinMode(this->min_sensor_pin, INPUT_PULLUP);
  pinMode(this->max_sensor_pin, INPUT_PULLUP);

  this->ignoreLimit = false;
}



bool StepperDriver::isTimerActive()
{
  return this->is_timer_active;
}



void StepperDriver::toggleIgnoreLimit()
{
  if (ignoreLimit) {
    // Serial.println("Change to KEEP LIMIT");
    ignoreLimit = false;
  } else {
    // Serial.println("Change to IGNORE LIMIT");
    ignoreLimit = true;
  }
}



int StepperDriver::getCurrentPos(bool is_percent)
{
  if (is_percent) {
    // to preserve 2nd order decimal magnify 100x
    // 12.34% -> 1234
    return int(100.0 * 100.0 * current_pos / max_pos);
  } else {
    return current_pos;
  }
}



void StepperDriver::setSpeed(float rpm)
{
  if (rpm < 60){return;} // at least 1 rev/sec needed
  // Serial.print("set speed ");
  // Serial.print(rpm);
  // useconds
  step_interval = 60000000L / (number_of_steps * rpm * step_division);
  // Serial.print(" -> interval of ");
  // Serial.println(step_interval);
}



void StepperDriver::setDirection(long steps_to_move)
{
  if (steps_to_move > 0) {
    digitalWrite(dir_pin, LOW);
    move_dir = true;
  }
  else {
    digitalWrite(dir_pin, HIGH);
    move_dir = false;
  }
}



void StepperDriver::cancelStep()
{
  if (timer){
    cancel_repeating_timer(timer);
    // delete timer; // FIXME needed?
    // Serial.println("clear timer");
    is_timer_active = false;
  }
  digitalWrite(step_pin, LOW); // init pin state
}


// mm is just for recognizing direction (0 for min, 100 for max, 50 for center)
void StepperDriver::calibrate(int mm)
{
  // move to zero
  if (mm == 0){
    step(-100000);
    // current_pos = 0;
  } else if (mm == 100){
    step(100000);
    // max_pos = current_pos;
  } else if (mm == 50) {
    // center pos
    long true_center = max_pos / step_division / 2 / 2;
    long true_current = current_pos / step_division / 2;
    // Serial.printf("true center %d, true current %d\r\n", true_center, true_current);
    step(true_center - true_current);
  }
}



void StepperDriver::step(long steps)
{
  cancelStep();
  // Serial.println("init timer");
  if (timer){delete timer;}
  timer = new repeating_timer_t;
  timer->user_data = (void *)this;
  // Serial.println("init timer DONE");


  steps *= step_division;
  steps *= 2; // timer will trigger 2x (because it uses toggle)
  steps_to_move = steps;
  // Serial.printf("total call count: %d (interval %d us)\r\n", steps_to_move, step_interval);
  setDirection(steps_to_move); // moveDir=true -> PLUS, false -> MINUS
  // read min/max sensor state and decide move or not
  if (move_dir && digitalRead(max_sensor_pin) == LOW && !ignoreLimit) {
    // Serial.println("CANNOT MOVE ABOVE MAX");
    return; // if max sensor is active (motor already at max end), avoid move +
  }
  if (!move_dir && digitalRead(min_sensor_pin) == LOW && !ignoreLimit) {
    current_pos = 0;
    // Serial.println("CANNOT MOVE BELOW MIN");
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

  if (_this->move_dir && digitalRead(_this->max_sensor_pin) == LOW && !(_this->ignoreLimit)) {
    _this->cancelStep();
    _this->max_pos = _this->current_pos;
    // Serial.printf("%c MAX LIMIT HIT! at %d\r\n", _this->axis, _this->step_counter);
    return false;
  }
  if (!(_this->move_dir) && digitalRead(_this->min_sensor_pin) == LOW && !(_this->ignoreLimit)) {
    _this->cancelStep();
    _this->current_pos = 0;
    // Serial.printf("%c MIN LIMIT HIT! at %d\r\n", _this->axis, _this->step_counter);
    return false;
  }

  // toggle output
  digitalWrite(_this->step_pin, !(_this->pin_state));
  _this->pin_state = !(_this->pin_state); // update pin state

  (_this->step_counter)++;
  if (_this->move_dir) {_this->current_pos++;}else{_this->current_pos--;}
  if (_this->step_counter >= abs(_this->steps_to_move)) {
    // Serial.printf("%c axis - count finished %d\r\n", _this->axis, _this->step_counter);
    // TODO replace with cancelStep?
    _this->is_timer_active = false;
    digitalWrite(_this->step_pin, LOW); // reset pin state to low
    return false; // return false -> timer stops
  }

  return true; // otherwise, keep timer running
}