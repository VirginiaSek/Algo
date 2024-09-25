#ifndef OXFORD
#define OXFORD

#include "./oxford_step_counter.h"

// Initialise step counting
void oxford_stepcount_init()
{
    oxford_init();
    oxford_resetSteps();
    oxford_resetAlgo();
}

// process sample
steps_t oxford_stepcount_totalsteps(time_delta_ms_t delta_ms, accel_t accx, accel_t accy, accel_t accz)
{
    return oxford_stepcount(delta_ms, accx, accy, accz);
}

#endif