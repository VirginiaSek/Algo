/**
 * Commonly used movement detection stage. Works by applying a moving average over the magnitude
 * and then comapring the average with a threshold, determined experimentally.
 */
#ifndef DETECTION_H
#define DETECTION_H

#include <stdbool.h>
#include "../../utils/buffer/buffer.h"
#include "../../types.h"

/**
 * Lenght of the buffer for the detection stage
 * At 12.5 Hz this corresponds to a little bit more than 1 second
 */
#define DETECTION_BUFFER_LEN 16

/**
 * Initalise and reset the detection stage
 * the buffer must be pre-allocated
 * the suggested size of the buffer is DETECTION_BUFFER_LEN
 */
void detect_movement_init(Buffer *buffer);

/**
 * Detects movement in the signal
 * delta_ms: time in ms since last sample
 * magnitude: acceleration magntiude
 * buffer: buffer where to store the data
 * returns: false if no movement is detected, and true if there is movement*
 */
bool detect_movement(time_delta_ms_t delta_ms, accel_big_t magnitude, Buffer *buffer);

#endif