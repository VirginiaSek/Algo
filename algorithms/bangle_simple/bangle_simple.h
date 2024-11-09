/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * ----------------------------------------------------------------------------
 * Simple (original) Step Counter
 * ----------------------------------------------------------------------------
 */

#ifndef BANGLE_SIMPLE_STEP_COUNTER
#define BANGLE_SIMPLE_STEP_COUNTER

#include "../../types.h"

// Initialise step counting
void bangle_simple_init();

/* Registers a new data point for step counting. Data is expected
 * as 12.5Hz, 8192=1g, and accMagSquared = x*x + y*y + z*z
 *
 * Returns the number of steps counted for this accel interval
 */
steps_t bangle_simple_stepcount(time_delta_ms_t delta_ms, accel_t accx, accel_t accy, accel_t accz);

#endif