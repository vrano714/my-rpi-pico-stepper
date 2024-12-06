#ifndef StepperDriver_h
#define StepperDriver_h

#include "Arduino.h"  // needed?

// library interface description
class StepperDriver {
  public:
    // constructors:
    StepperDriver(int number_of_steps, int step_division, int dir_pin, int step_pin, char axis);

    // speed setter method:
    void setSpeed(float rpm);

    // mover method:
    void step(long steps_to_move);

  private:
    void setDirection(long steps_to_move);

    static bool move(repeating_timer_t *t);

    int number_of_steps;
    int step_division;
    unsigned long step_interval;
    repeating_timer_t *timer;
    int step_counter;
    long steps_to_move;

    char axis;
    
    // motor pin numbers:
    int dir_pin;
    int step_pin;
          
};

#endif