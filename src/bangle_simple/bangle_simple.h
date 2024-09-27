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

// Initialise step counting
void bangle_simple_init();

/* Registers a new data point for step counting. Data is expected
 * as 12.5Hz, 8192=1g, and accMagSquared = x*x + y*y + z*z
 *
 * Returns the number of steps counted for this accel interval
 */
int bangle_simple_stepcount(int delta_ms, int accx, int accy, int accz);

#endif