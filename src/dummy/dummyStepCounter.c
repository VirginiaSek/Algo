/*
 * ----------------------------------------------------------------------------
 * Dummy Step Counter
 * ----------------------------------------------------------------------------
 */
#include <stdint.h>

long dummy_samples_counter = 0;
const float steps_per_sec = 1.5;

/// Initialise step counting
void dummy_stepcount_init()
{
    dummy_samples_counter = 0;
}

// process sample
int dummy_stepcount(int delta_ms, int accx, int accy, int accz)
{
    // keeps track of how many samples have been taken -> we can derive time passed
    dummy_samples_counter++;

    // time passed in seconds since the start
    float timepassed_secs = (float)dummy_samples_counter / 12.5;

    // dummy step counter: simply moltiply time by a step rate
    return (timepassed_secs * steps_per_sec);
}
