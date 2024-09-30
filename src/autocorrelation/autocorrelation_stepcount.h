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
// Definisci NUM_TUPLES come il numero di tuple contate

uint16_t count_steps(int16_t *data);
 
#endif