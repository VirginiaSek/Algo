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
#include "../types.h"
#include "bangle_simple.h"
#include "../movement_detection/movement_detection.h"
#include <math.h>
 
static bool bangle_simple_StepWasLow;
 
/// How low must acceleration magnitude squared get before we consider the next rise a step?
static accel_big_t bangle_simple_ThresholdLow = (8192 - 80) * (8192 - 80);
/// How high must acceleration magnitude squared get before we consider it a step?
static accel_big_t bangle_simple_ThresholdHigh = (8192 + 80) * (8192 + 80);
static steps_t bangle_simple_StepCount = 0;
 
 
 
static accel_big_t banglesimple_mov_detect_buffdata[DETECTION_BUFFER_LEN];
static Buffer banglesimple_mov_detect_buffer = {
    .length = DETECTION_BUFFER_LEN,
    .head = 0,
    .tail = 0,
    .data = banglesimple_mov_detect_buffdata};
 
 
/// Initialise step counting
void bangle_simple_init()
{
    bangle_simple_StepCount = 0;
    bangle_simple_StepWasLow = 0;
    detect_movement_init(&banglesimple_mov_detect_buffer);
}
 
/* Registers a new data point for step counting.
*
* Returns the number of steps counted for this accel interval
*/
steps_t bangle_simple_stepcount(time_delta_ms_t delta_ms, accel_t accx, accel_t accy, accel_t accz)
{
 
    accel_big_t accMagSquared = accx * accx + accy * accy + accz * accz;
    uint16_t magn = sqrt(accMagSquared);
    bool ismoving = detect_movement(delta_ms, magn, &banglesimple_mov_detect_buffer);
 
    if (delta_ms > 0 && ismoving)
    {
        if (accMagSquared < bangle_simple_ThresholdLow)
            bangle_simple_StepWasLow = true;
        else if ((accMagSquared > bangle_simple_ThresholdHigh) && bangle_simple_StepWasLow)
        {
            bangle_simple_StepWasLow = false;
            bangle_simple_StepCount++;
        }
    }
 
    return bangle_simple_StepCount;
}
