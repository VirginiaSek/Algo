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
 
#define DUMP_FILE
#define DUMP_MAGNITUDE_FILE_NAME "magnitude.csv"
#define DUMP_FILTERED_FILE_NAME "filtered.csv"
#define DUMP_REMOVED_MEAN_FILE_NAME "removed_mean.csv"
#define DUMP_AUTOCORRELATION_FILE_NAME "autocorrelation.csv"
#define DUMP_DERIVATIVE_FILE_NAME "derivative.csv"


#define SAMPLING_RATE           12.5                    //20 hz sampling rate
#define NUM_TUPLES 80
// Definisci NUM_TUPLES come il numero di tuple contate
#define WINDOW_LENGTH           NUM_TUPLES/SAMPLING_RATE //window length in seconds
 
uint16_t count_steps(int16_t *data);
 
#endif
