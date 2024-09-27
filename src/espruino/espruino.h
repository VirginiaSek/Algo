#ifndef ESPRUINO
#define ESPRUINO

#include "stepcount.h"

static long espruino_steps_counter = 0;

/// Initialise step counting
void espruino_stepcount_init()
{
    void stepcount_init();
    espruino_steps_counter = 0;
}

// process sample
steps_t espruino_stepcount(time_delta_ms_t delta_ms, accel_t accx, accel_t accy, accel_t accz)
{

    int accMagSquared = accx * accx + accy * accy + accz * accz;
    espruino_steps_counter += stepcount_new(accMagSquared);
    return espruino_steps_counter;
}

#endif