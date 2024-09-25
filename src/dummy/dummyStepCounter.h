/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * ----------------------------------------------------------------------------
 * Dummy Step Counter
 * ----------------------------------------------------------------------------
 */

#ifndef DUMMY_STEP_COUNTER
#define DUMMY_STEP_COUNTER

#include "../types.h"

/**
 * Initalise and reset the step counter
 */
void dummy_stepcount_init();

/* Registers a new data point for step counting.
 * Each acceleration axis is expected as 12.5Hz, 8192=1g
 *
 * delta_ms: difference in millis between this sample and the previous
 * accx: acceleration on the x axis
 * accy: acceleration on the y axis
 * accz: acceleration on the z axis
 *
 * returns: the number of steps counted so far.
 */
steps_t dummy_stepcount(time_delta_ms_t delta_ms, accel_t accx, accel_t accy, accel_t accz);

#endif
