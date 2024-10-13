/**
 * Commonly used movement detection stage. Works by applying a moving average over the magnitude
 * and then comapring the average with a threshold, determined experimentally.
 */
#include <stdbool.h>

#include "movement_detection.h"
#include "types.h"

/**
 * Threshold to identify movement vs non-movement
 */
#define DETECTION_THRESHOLD 1500

/**
 * Initalise and reset the detection stage
 */
void detect_movement_init(Buffer *buffer)
{
    buffer_reset(buffer);
}

/**
 * Detects movement in the signal
 * delta_ms: time in ms since last sample
 * magnitude: acceleration magntiude
 * returns: false if no movement is detected, and true if there is movement
 */
bool detect_movement(time_delta_ms_t delta_ms, accel_big_t magnitude, Buffer *buffer)
{
    // add to the buffer
    buffer_put(buffer, magnitude);

    // get min and max
    accel_big_t max = -1e6;
    accel_big_t min = 1e6;
    for (int i = 0; i < buffer_len(buffer); i++)
    {
        accel_big_t magn_read = 0;
        buffer_peek(buffer, i, &magn_read);

        if (magn_read > max)
        {
            max = magn_read;
        }
        if (magn_read < min)
        {
            min = magn_read;
        }
    }
    // if the buffer is full, remove the oldest
    if (buffer_len(buffer) >= (buffer->length - 1))
    {
        accel_big_t magn_removed = 0;
        buffer_get(buffer, &magn_removed);
    }

    // movement is detected if max - min is above a threshold
    if ((max - min) > DETECTION_THRESHOLD)
        return true;
    else
        return false;
}