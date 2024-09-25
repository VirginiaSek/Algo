/*
 * ----------------------------------------------------------------------------
 * Dummy Step Counter
 * ----------------------------------------------------------------------------
 */
#include <stdint.h>
#include "../types.h"

accel_big_t dummy_samples_counter = 0;
const float steps_per_sec = 1.5;

/// Initialise step counting
void dummy_stepcount_init()
{
    dummy_samples_counter = 0;
}

// process sample
steps_t dummy_stepcount(time_delta_ms_t delta_ms, accel_t accx, accel_t accy, accel_t accz)
{
    // keeps track of how many samples have been taken -> we can derive time passed
    dummy_samples_counter++;

    // time passed in seconds since the start
    float timepassed_secs = (float)dummy_samples_counter / 12.5;

    // dummy step counter: simply moltiply time by a step rate
    return (timepassed_secs * steps_per_sec);
}
