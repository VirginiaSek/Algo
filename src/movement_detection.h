/**
 * Commonly used movement detection stage. Works by applying a moving average over the magnitude
 * and then comapring the average with a threshold, determined experimentally.
 */
#ifndef DETECTION_H
#define DETECTION_H

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
int detect_buffer_head = 0;
int detect_buffer_tail = 0;

static bool detection_buffer_is_full()
{
    return ((detect_buffer_head + 1) % DETECTION_BUFFER_LEN == detect_buffer_tail);
}

static unsigned int detection_buffer_len()
{
    if (detect_buffer_head == detect_buffer_tail)
        return 0;
    else if (detect_buffer_head > detect_buffer_tail)
        return detect_buffer_head - detect_buffer_tail;
    else
        return (DETECTION_BUFFER_LEN - detect_buffer_tail) + detect_buffer_head;
}

static bool detection_buffer_put(accel_big_t item)
{
    int next;

    next = detect_buffer_head + 1; // next is where head will point to after this write.
    if (next >= DETECTION_BUFFER_LEN)
        next = 0;

    if (next == detect_buffer_tail) // if the head + 1 == tail, circular buffer is full
    {
        printf("ERR on put: buffer is full");
        return -1;
    }

    detection_stage_buffer[detect_buffer_head] = item; // Load data and then move
    detect_buffer_head = next;                         // head to next data offset.
    return 0;                                          // return success to indicate successful push.
}

static bool detection_buffer_get(accel_big_t *value)
{
    int next;

    if (detect_buffer_head == detect_buffer_tail) // if the head == tail, we don't have any data
    {
        printf("ERR on get: buffer empty");
        return -1;
    }

    next = detect_buffer_tail + 1; // next is where tail will point to after this read.
    if (next >= DETECTION_BUFFER_LEN)
        next = 0;

    *value = detection_stage_buffer[detect_buffer_tail]; // Read data and then move
    detect_buffer_tail = next;                           // tail to next offset.
    return 0;                                            // return success to indicate successful push.
}

/**
 * Initalise and reset the detection stage
 */
void detection_stage_init()
{
    detect_buffer_head = 0;
    detect_buffer_tail = 0;
    detect_mean = 0;
    for (int i = 0; i < DETECTION_BUFFER_LEN; i++)
    {
        detection_stage_buffer[i] = 0;
    }
}

/**
 * Detects movement in the signal
 * delta_ms: time in ms since last sample
 * magnitude: acceleration magntiude
 * returns: 0 if no movement is detected, and 1 if there is movement*
 */
bool detect_movement(time_delta_ms_t delta_ms, accel_big_t magnitude)
{

    // add to the buffer
    detection_buffer_put(magnitude);

    unsigned int len = detection_buffer_len();
    // printf("len (after put) %d\n", len);

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

        // printf("removed %d\n", magn_removed);

        detect_mean += ((float)magnitude - (float)magn_removed) / (DETECTION_BUFFER_LEN - 2);
    }

    // printf("moving avg %.2f\n", detect_mean);

    if (detect_mean > DETECTION_THRESHOLD)
        return true;
    else
        return false;
}

#endif