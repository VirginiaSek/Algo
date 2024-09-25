#ifndef TYPES_H
#define TYPES_H

#include <inttypes.h>

/*
 * Type for the accelerometer samples, this depends on your hardware.
 * For example, if your accelerometer uses 12bits then an int16_t would suffice.
 * The value is assumed to be integer, but it can be signed, indicating direction of the acceleration.
 */
typedef int16_t accel_t;

/*
 * Type used for something slightly bigger than the acceleration, for example for the magnitude or an accumulator.
 * Here no assumptions are made about the sign.
 */
typedef int32_t accel_big_t;

// type used for time: warning the algorithm is not robust to roll-over of this variable
// example: a year worth of ms needs 35 bits, 32 bits allows you to store about 50 days of ms

/*
 * Type used to express time differences in ms.
 * Distance between samples is not expected to be high, as at leaast 10Hz sampling frequency is needed (100 ms).
 * 16 bits should be more than enough.
 */
typedef int16_t time_delta_ms_t;

/*
 * Type used to represent the total step count.
 * The size of this variable depends on what is the maximum number of steps you are willing to show and store.
 * Example: if the step count is reset every 24h and one walks no more than 20k / day (reasonable) -> 16 bits should be enough.
 */
typedef uint16_t steps_t;

#endif