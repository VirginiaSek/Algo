/*
 * ----------------------------------------------------------------------------
 * Dummy Step Counter
 * ----------------------------------------------------------------------------
 */
#include <stdint.h>
#include <math.h>

#include "../types.h"
#include "../detection.h"
#include "dummyStepCounter.h"

long long dummy_time_passed = 0;
const float steps_per_sec = 2;

/// Initialise step counting
void dummy_stepcount_init()
{
    dummy_time_passed = 0;

    detection_stage_init();
}

// process sample
steps_t dummy_stepcount(time_delta_ms_t delta_ms, accel_t accx, accel_t accy, accel_t accz)
{
    uint16_t magn = sqrt(accx * accx + accy * accy + accz * accz);
    bool ismoving = detect_movement(magn);

    if (delta_ms > 0 && ismoving)
    {
        dummy_time_passed += delta_ms;
    }

    // time passed in seconds since the start
    float timepassed_secs = (float)dummy_time_passed / 1000;

    // dummy step counter: simply moltiply time by a step rate
    return (timepassed_secs * steps_per_sec);
}
