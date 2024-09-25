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
int espruino_stepcount(int delta_ms, int accx, int accy, int accz)
{

    int accMagSquared = accx * accx + accy * accy + accz * accz;
    espruino_steps_counter += stepcount_new(accMagSquared);
    return espruino_steps_counter;
}

#endif