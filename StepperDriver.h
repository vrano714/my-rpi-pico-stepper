#ifndef StepperDriver_h
#define StepperDriver_h

#include "Arduino.h"  // needed?

// library interface description
class StepperDriver {
  public:
    // constructors:
    StepperDriver(int number_of_steps, int step_division, int dir_pin, int step_pin, int min_sensor_pin, int max_sensor_pin, char axis);

    void initMinMaxSensors();

    // speed setter method:
    void setSpeed(float rpm);

    // mover method:
    void step(long steps_to_move);

    void cancelStep();

    bool isTimerActive();

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

    bool pin_state;

    bool is_timer_active;
    
    // motor pin numbers:
    int dir_pin;
    int step_pin;

    // sensor pin numbers
    int min_sensor_pin;
    int max_sensor_pin;
};

#endif