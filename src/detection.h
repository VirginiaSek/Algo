#include <stdbool.h>
#include <stdio.h>

#include "../types.h"

/**
 * Lenght of the buffer for the detection stage + 1
 * At 12.5 Hz this corresponds to a little bit more than 1 second
 */
#define DETECTION_BUFFER_LEN 16

/**
 * threshold to identify movement vs non movement
 */
#define DETECTION_THRESHOLD 8500

/**
 * Circula buffer containing the magnitude of the acceleration
 * See https://en.wikipedia.org/wiki/Circular_buffer
 */
static accel_big_t detection_stage_buffer[DETECTION_BUFFER_LEN];
static float detect_mean = 0;
int writeIndx = 0;
int readIndx = 0;

static bool detection_buffer_is_full()
{
    return ((writeIndx + 1) % DETECTION_BUFFER_LEN == readIndx);
}

static unsigned int detection_buffer_len()
{
    if (writeIndx == readIndx)
        return 0;
    else if (writeIndx > readIndx)
        return writeIndx - readIndx;
    else
        return (DETECTION_BUFFER_LEN - readIndx) + writeIndx;
}

static bool detection_buffer_put(accel_big_t item)
{
    if (detection_buffer_is_full())
    {
        // buffer is full, avoid overflow
        return 0;
    }
    detection_stage_buffer[writeIndx] = item;
    writeIndx = (writeIndx + 1) % DETECTION_BUFFER_LEN;
    return 1;
}

static bool detection_buffer_get(accel_big_t *value)
{
    if (readIndx == writeIndx)
    {
        // buffer is empty
        return 0;
    }

    *value = detection_stage_buffer[readIndx];
    readIndx = (readIndx + 1) % DETECTION_BUFFER_LEN;
    return 1;
}

/**
 * Initalise and reset the detection stage
 */
void detection_stage_init()
{
    writeIndx = 0;
    readIndx = 0;
    detect_mean = 0;
    for (int i = 0; i < DETECTION_BUFFER_LEN; i++)
    {
        detection_stage_buffer[i] = 0;
    }
}

/**
 * Detects movement in the signal
 * magnitude: acceleration magntiude
 * returns: 0 if no movement is detected, and 1 if there is movement*
 */
bool detect_movement(accel_big_t magnitude)
{

    // add to the buffer
    detection_buffer_put(magnitude);

    unsigned int len = detection_buffer_len();
    printf("len %d\n", len);

    if (detection_buffer_len() < (DETECTION_BUFFER_LEN - 1))
    {
        float delta = (float)magnitude - detect_mean;
        detect_mean += delta / detection_buffer_len();
    }
    else
    {
        // remove oldest value from buffer
        accel_big_t magn_removed = 0;
        detection_buffer_get(&magn_removed);

        printf("removed %d\n", magn_removed);

        detect_mean += ((float)magnitude - (float)magn_removed) / (DETECTION_BUFFER_LEN - 1);
    }

    printf("moving avg %.2f\n", detect_mean);

    if (detect_mean > DETECTION_THRESHOLD)
        return true;
    else
        return false;
}