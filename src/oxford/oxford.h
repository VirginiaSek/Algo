#ifndef OXFORD
#define OXFORD

#include "./StepCountingAlgo.h"

/// Initialise step counting
void oxford_stepcount_init()
{
    oxford_init();
    oxford_resetSteps();
    oxford_resetAlgo();
}

// process sample
int oxford_stepcount_totalsteps(int delta_ms, int accx, int accy, int accz)
{
    return oxford_stepcount(delta_ms, accx, accy, accz);
}

#endif