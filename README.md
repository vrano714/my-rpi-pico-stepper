# Motor Driver for 3-axis Linear Actuator using Raspberry Pi Pico

Codes are completely for me, but public if someone may benefit.

Deps: FastLED lib for monitoring status (can be removed)

## To use

- Set motor parameters in sketch
- Set pins for drive direction and drive pulse in sketch
- Set pins for terminal detection sensors

Then compile and burn into Pi Pico.

## Drive motors

Plug Pi Pico to a computer, wait for initialization (when power comes, built-in LED will turn on and turn off after initialization)

Open a serial console with 115200 baud, then control like:

- `x:1000`: rotate X-axis stepper for 1000 steps (`y` and `z` can be controlled by the same way)
- `x:-1000`: same as above, but rotation direction is inverted
- `s:240`: set motor speed to 240 (rpm) (common speed for all motors)
- `p`: cancel movement (all motors will stop)

## Notes

Driver code uses `repeating_timer`. It does not block main thread.
