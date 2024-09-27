/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * ----------------------------------------------------------------------------
 * Autocorrelation Step Counter
 * ----------------------------------------------------------------------------
 */

#ifndef autocorrelation_stepcount_h
#define autocorrelation_stepcount_h
#include "stdint.h"
 
#define SAMPLING_RATE           12.5                    //20 hz sampling rate
#define NUM_TUPLES              80                //80 sets of accelerometer readings (so in other words, 80*3 = 240 samples)
#define WINDOW_LENGTH           NUM_TUPLES/SAMPLING_RATE //window length in seconds
 
uint16_t count_steps(int16_t *data);
 
#endif