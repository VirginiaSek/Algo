/**
 * Commonly used movement detection stage. Works by applying a moving average over the magnitude
 * and then comapring the average with a threshold, determined experimentally.
 */
#ifndef DETECTION_H
#define DETECTION_H

#include <stdbool.h>
#include "../types.h"

/**
 * Initalise and reset the detection stage
 */
void detection_stage_init();

/**
 * Detects movement in the signal
 * delta_ms: time in ms since last sample
 * magnitude: acceleration magntiude
 * returns: 0 if no movement is detected, and 1 if there is movement*
 */
bool detect_movement(time_delta_ms_t delta_ms, accel_big_t magnitude);

#endif