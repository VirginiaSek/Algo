
#include "autocorrelation_stepcount.h"
#include <math.h>
#include "../movement_detection.h"
#include "../types.h"

steps_t autocorrelation_steps_counter = 0;
static FILE *magnitudeFile = NULL;

void autocorrelation_stepcount_init()
{
    autocorrelation_steps_counter = 0;

#ifdef DUMP_FILE
    magnitudeFile = fopen(DUMP_MAGNITUDE_FILE_NAME, "w+");
#endif
}

accel_big_t autocorr_buffer[NUM_TUPLES];
long autocorr_buffer_index = 0;

static long movement_samples = 0;

// Wrapper function
steps_t autocorrelation_stepcount_totalsteps(time_delta_ms_t delta_ms, accel_t accx, accel_t accy, accel_t accz)
{
    uint16_t magn = sqrt(accx * accx + accy * accy + accz * accz);
    movement_samples += detect_movement(delta_ms, magn);

    autocorr_buffer[autocorr_buffer_index] = magn;

#ifdef DUMP_FILE
    if (magnitudeFile)
    {

        fprintf(magnitudeFile, "%ld, %d\n", autocorr_buffer_index, magn);
        fflush(magnitudeFile);
    }
#endif

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

void close_magnitude_file()
{
#ifdef DUMP_FILE
    if (magnitudeFile)
        fclose(magnitudeFile);
#endif
}
