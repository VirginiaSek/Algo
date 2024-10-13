/*
 * ----------------------------------------------------------------------------
 * Dummy Step Counter
 * ----------------------------------------------------------------------------
 */
#include <stdint.h>
#include <stdio.h>
#include <math.h>

#include "../types.h"
#include "../movement_detection/movement_detection.h"
#include "dummyStepCounter.h"

static long long dummy_time_passed = 0;
static const float steps_per_sec = 2;

static accel_big_t dummy_mov_detect_buffdata[DETECTION_BUFFER_LEN];
static Buffer dummy_mov_detect_buffer = {
    .length = DETECTION_BUFFER_LEN,
    .head = 0,
    .tail = 0,
    .data = dummy_mov_detect_buffdata};

/// Initialise step counting
void dummy_stepcount_init()
{
    dummy_time_passed = 0;
    detect_movement_init(&dummy_mov_detect_buffer);
}

// process sample
steps_t dummy_stepcount(time_delta_ms_t delta_ms, accel_t accx, accel_t accy, accel_t accz)
{
    static int index = 0;
    uint16_t magn = sqrt(accx * accx + accy * accy + accz * accz);
    bool ismoving = detect_movement(delta_ms, magn, &dummy_mov_detect_buffer);
    if (!ismoving)
    {
        printf("Not moving at %d\n", index);
    }
    index++;

    if (delta_ms > 0 && ismoving)
    {
        dummy_time_passed += delta_ms;
    }

    // time passed in seconds since the start
    float timepassed_secs = (float)dummy_time_passed / 1000;

    // dummy step counter: simply moltiply time by a step rate
    return (timepassed_secs * steps_per_sec);
}
