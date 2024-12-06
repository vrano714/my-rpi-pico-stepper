#include "StepperDriver.h"


StepperDriver::StepperDriver(int number_of_steps, int step_division, int dir_pin, int step_pin, char axis)
{
  this->number_of_steps = number_of_steps;
  this->step_division = step_division;
  this->step_interval = 10000; // microseconds between pulse?

  this->step_counter = 0;
  this->steps_to_move = 0;
  
  // Arduino pins for the motor control connection:
  this->dir_pin = dir_pin;
  this->step_pin = step_pin;

  this->axis = axis;

  this->is_timer_active = false;

  // setup the pins on the microcontroller:
  pinMode(dir_pin, OUTPUT);
  pinMode(step_pin, OUTPUT);
}

bool StepperDriver::isTimerActive()
{
  return this->is_timer_active;
}

/*
  Sets the speed in revs per minute
*/
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
  if (steps_to_move < 0) {
    digitalWrite(dir_pin, HIGH);
  }
  else {
    digitalWrite(dir_pin, LOW);
  }
}



/*
  Moves the motor steps_to_move steps.  If the number is negative, 
  the motor moves in the reverse direction.
 */
void StepperDriver::step(long steps)
{
    digitalWrite(LED_BUILTIN, LOW);
  if (timer){
    cancel_repeating_timer(timer);
    delete timer;
    Serial.println("clear old timer");
    is_timer_active = false;
  }
  timer = new repeating_timer_t;
  timer->user_data = (void *)this;

  steps *= step_division;
  steps_to_move = steps;
  Serial.printf("total call count: %d (interval %d us)\n", steps_to_move, step_interval);
  setDirection(steps_to_move);

  step_counter = 0;

  add_repeating_timer_us(-1*(long)step_interval, move, (void *)this, timer);
  is_timer_active = true;
}



bool StepperDriver::move(repeating_timer_t *t)
{
  StepperDriver *_this = reinterpret_cast<StepperDriver *>(t->user_data);

  digitalWrite(_this->step_pin, HIGH);
  // delay millisec
  sleep_us((_this->step_interval)>>1); // 1bit right shift- > half the value
  digitalWrite(_this->step_pin, LOW);
  (_this->step_counter)++;
  if (_this->step_counter >= _this->steps_to_move) {
    // cancel timer
    cancel_repeating_timer(t);
    Serial.printf("%c axis - count finished %d\n", _this->axis, _this->step_counter);
    digitalWrite(LED_BUILTIN, HIGH);
    _this->is_timer_active = false;
  }

  return true;
}