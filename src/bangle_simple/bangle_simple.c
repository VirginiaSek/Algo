/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 *  Copyright (C) 2021 Gordon Williams <gw@pur3.co.uk>
 *
 * ----------------------------------------------------------------------------
 * Simple (original) Step Counter
 * ----------------------------------------------------------------------------
 */
#include <stdbool.h>

static bool bangle_simple_StepWasLow;

/// How low must acceleration magnitude squared get before we consider the next rise a step?
static int bangle_simple_ThresholdLow = (8192 - 80) * (8192 - 80);
/// How high must acceleration magnitude squared get before we consider it a step?
static int bangle_simple_ThresholdHigh = (8192 + 80) * (8192 + 80);
static int bangle_simple_StepCount = 0;

/// Initialise step counting
void bangle_simple_init()
{
    bangle_simple_StepCount = 0;
    bangle_simple_StepWasLow = 0;
}

/* Registers a new data point for step counting.
 *
 * Returns the number of steps counted for this accel interval
 */
int bangle_simple_stepcount(int delta_ms, int accx, int accy, int accz)
{

    int accMagSquared = accx * accx + accy * accy + accz * accz;

    if (accMagSquared < bangle_simple_ThresholdLow)
        bangle_simple_StepWasLow = true;
    else if ((accMagSquared > bangle_simple_ThresholdHigh) && bangle_simple_StepWasLow)
    {
        bangle_simple_StepWasLow = false;
        bangle_simple_StepCount++;
    }

    return bangle_simple_StepCount;
}