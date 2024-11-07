#include "autocorrelation2.h"
#include <math.h>
#include <stdio.h>
#include <strings.h> //for file operations

#include "../lowpass_filter.h"
#include "../types.h"

#define LAST_AUTOCORR_PEAK_LAG 16 // number of lags to calculate for autocorrelation. At a minimum step rate of §1 step /s -> 1000 / sampling_period(ms) = sampling frequency -> add a few more -> 16 at 12.5 Hz
#define FIRST_AUTOCORR_PEAK_LAG 4 // corresponds to the first feasible autocorrelation lag -> at a max step rate of 3 steps /s (running) -> 333ms / sampling_period(ms) = 4 at 12.5 Hz
#define WINDOW_LEN 32             // sliding window length, better if power of 2 (if we want to switch to FFT), 32 samples = 2.5s , 64 samples = 5.12 s
#define WINDOW_STEP 12            // step of the sliding window, kept at roughly the longest step time (1s)

static accel_big_t signal_buffer[WINDOW_LEN] = {0}; // low pass filtered data, on which autocorrelation is computed
static int signal_buffer_next_i;                    // index of the next sample in the buffer

#define AUTOCORRELATION_BUFFER_LEN 18 // number of autocorrelation samples, = LAST_AUTOCORR_PEAK_LAG + 2 = 18
static int64_t autocorr_buff[AUTOCORRELATION_BUFFER_LEN] = {0};
#define AUTOCORR_PEAK_THRE 1e5

static int samples_since_step_count = 0;

// keep track of the minimum and maximum values of the magnitude in the curent window step
static accel_big_t window_step_min, window_step_max;
#define MOVEMENT_DETECTION_THRESHOLD 1500

// total steps counter, use decimals because fractions are added at each WINDOW_STEP
static double total_steps = 0;

// uncomment to debug for ONE FILE ONLY!
// #define DUMP_FILE

#ifdef DUMP_FILE
static int autocorr_passes = 0; // counter of how many times the autocorr has been called
static FILE *autocorrelationFile;
#endif

static int buffer_index_plus(int buffer_next_i, int plus, int max)
{
    return (buffer_next_i + plus) % max;
}

void autocorrelation_2_init()
{
    total_steps = 0;
    samples_since_step_count = 0;
    for (int i = 0; i < WINDOW_LEN; i++)
    {
        signal_buffer[i] = 0;
    }
    signal_buffer_next_i = 0;

    for (int i = 0; i < AUTOCORRELATION_BUFFER_LEN; i++)
    {
        autocorr_buff[i] = 0;
    }

    window_step_min = 66000;
    window_step_max = -66000;
}

steps_t autocorrelation2_stepcount_totalsteps(time_delta_ms_t delta_ms, accel_t accx, accel_t accy, accel_t accz)
{
    // compute magnitude
    uint16_t magn = sqrt(accx * accx + accy * accy + accz * accz);

    // TODO: LPF

    // add to buffer
    signal_buffer[signal_buffer_next_i] = magn;
    signal_buffer_next_i = (signal_buffer_next_i + 1) % WINDOW_LEN;

    samples_since_step_count++;

    if (magn < window_step_min)
    {
        window_step_min = magn;
    }
    if (magn > window_step_max)
    {
        window_step_max = magn;
    }

    if (samples_since_step_count == WINDOW_STEP)
    {
#ifdef DUMP_FILE
        autocorr_passes++;
        char autocorrFileName[100] = "autocorr2_";
        char idxstr[5];
        sprintf(idxstr, "%d", autocorr_passes);
        strcat(autocorrFileName, idxstr);
        strcat(autocorrFileName, ".csv");
        autocorrelationFile = fopen(autocorrFileName, "w+");
#endif
        samples_since_step_count = 0;

        // movement detection

        if (window_step_max - window_step_min > MOVEMENT_DETECTION_THRESHOLD)
        {
            // enough movement, go on computing the presence of new steps!
            // remove mean
            accel_big_t mean = 0;
            for (int i = 0; i < WINDOW_LEN; i++)
            {
                mean += signal_buffer[i] / WINDOW_LEN;
            }

            // compute autocorrelation within the lags of interest
            uint8_t lag;
            uint16_t i;
            int64_t temp_ac;
            for (lag = 0 /*FIRST_AUTOCORR_PEAK_LAG - 2*/; lag < LAST_AUTOCORR_PEAK_LAG + 2; lag++)
            {
                temp_ac = 0;
                for (i = 0; i < WINDOW_LEN - lag; i++)
                {
                    int buffer_i = buffer_index_plus(signal_buffer_next_i, i, WINDOW_LEN);
                    int buffer_i_plus_lag = buffer_index_plus(signal_buffer_next_i, i + lag, WINDOW_LEN);
                    // printf("lag %d, buff_i %d, buff_i+lag %d\n", lag, buffer_i, buffer_i_plus_lag);
                    temp_ac += (int64_t)(signal_buffer[buffer_i] - mean) * (int64_t)(signal_buffer[buffer_i_plus_lag] - mean);
                }

                autocorr_buff[lag] = temp_ac;

#ifdef DUMP_FILE
                if (autocorrelationFile)
                {
                    fprintf(autocorrelationFile, "%lld\n", temp_ac);
                }
#endif
            }

#ifdef DUMP_FILE
            if (autocorrelationFile)
            {
                fflush(autocorrelationFile);
                fclose(autocorrelationFile);
            }
#endif

            // find the peak
            uint8_t autocorr_peak_found = 0;
            for (lag = FIRST_AUTOCORR_PEAK_LAG; lag <= LAST_AUTOCORR_PEAK_LAG; lag++)
            {
                // printf("looking at lag %d for peak\n", lag);
                if (
                    (autocorr_buff[lag] > autocorr_buff[lag - 1]) &&
                    (autocorr_buff[lag] > autocorr_buff[lag + 1]) &&
                    (autocorr_buff[lag - 1] >= autocorr_buff[lag - 2]) &&
                    (autocorr_buff[lag + 1] >= autocorr_buff[lag + 2]))
                {
                    // found a peak, but is it highe neough?
                    if ((autocorr_buff[lag] - autocorr_buff[lag - 2] > AUTOCORR_PEAK_THRE) &&
                        (autocorr_buff[lag] - autocorr_buff[lag + 2] > AUTOCORR_PEAK_THRE))
                    {
                        // peak is high enough, real peak
                        autocorr_peak_found = 1;
                        break;
                    }
                }
            }

            if (autocorr_peak_found)
            {
                // the n of steps walked in the WINDOW_STEP time is the lenght of the window / periodicity (in samples)
                float num_steps = (float)WINDOW_STEP / lag;
                total_steps += num_steps;
            }
        }

        // reset min and max
        window_step_min = 66000;
        window_step_max = -66000;
    }

    return (steps_t)total_steps;
}
