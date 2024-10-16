
#include "autocorrelation_stepcount.h"
#include <math.h>
#include "../movement_detection/movement_detection.h"
#include "../types.h"

// #define DUMP_FILE

static steps_t autocorrelation_steps_counter = 0;
static FILE *magnitudeFile = NULL;

static accel_big_t autocorr_buffer[NUM_TUPLES];
static long autocorr_buffer_index = 0;

static accel_big_t autocor_mov_detect_buffdata[DETECTION_BUFFER_LEN];
static Buffer autocor_mov_detect_buffer = {
    .length = DETECTION_BUFFER_LEN,
    .head = 0,
    .tail = 0,
    .data = autocor_mov_detect_buffdata};
static long movement_samples = 0;

void autocorrelation_stepcount_init()
{
    autocorrelation_init();
    detect_movement_init(&autocor_mov_detect_buffer);

    autocorrelation_steps_counter = 0;
    movement_samples = 0;
}

// Wrapper function
steps_t autocorrelation_stepcount_totalsteps(time_delta_ms_t delta_ms, accel_t accx, accel_t accy, accel_t accz)
{
    uint16_t magn = sqrt(accx * accx + accy * accy + accz * accz);
    movement_samples += detect_movement(delta_ms, magn, &autocor_mov_detect_buffer);

    autocorr_buffer[autocorr_buffer_index] = magn;

    autocorr_buffer_index++;

    if (autocorr_buffer_index == NUM_TUPLES)
    {
        if (movement_samples >= (NUM_TUPLES / 4))
        {
            // more than 25% of the samples contain movement
            autocorrelation_steps_counter += autcorr_count_steps(autocorr_buffer);
        }
        // autocorrelation_steps_counter += autcorr_count_steps(autocorr_buffer);
        autocorr_buffer_index = 0;
        movement_samples = 0;
    }

    return autocorrelation_steps_counter; // Restituisci il conteggio totale dei passi
}
