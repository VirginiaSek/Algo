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
#include "../types.h"

// #define DUMP_FILE
#define DUMP_MAGNITUDE_FILE_NAME "magnitude.csv"
#define DUMP_FILTERED_FILE_NAME "filtered.csv"
#define DUMP_REMOVED_MEAN_FILE_NAME "removed_mean.csv"
#define DUMP_AUTOCORRELATION_FILE_NAME "autocorrelation.csv"
#define DUMP_DERIVATIVE_FILE_NAME "derivative.csv"

#define SAMPLING_RATE 12.5                       // 12.5 hz sampling rate
#define NUM_TUPLES 50                            // 4 seconds worth of data
#define WINDOW_LENGTH NUM_TUPLES / SAMPLING_RATE // window length in seconds: 4

steps_t autcorr_count_steps(accel_big_t *data);

#endif
