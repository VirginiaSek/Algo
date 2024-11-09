#include "panTompkins.h"
#include "stdio.h" //for file operations
#include <math.h>  // Per sqrt

#define FS 12.5 // sampling frequency 12.5 Hz

#define MAGNITUDE_BUFFER_LEN 15
static accel_big_t magnitude_buffer[MAGNITUDE_BUFFER_LEN];
// holds the index of the current sample
static int magnitude_buffer_i = 0;

#define FILTERED_BUFFER_LEN 5
static accel_big_t filtered_buffer[FILTERED_BUFFER_LEN];
static int filtered_buffer_i = 0;

// counter of samples that cannot allow a peak (a step) due to minimum time between steps
// at 12.5Hz sampling rate and 3 steps/s max -> 4 samples (12/3)
#define BLANK_TIME_LEN 4
static int blank_time_cnt = 0;

static accel_big_t peakF = 0;
static accel_big_t peakF_temp = 0;
static accel_big_t best_peak_filtered = 0;
static accel_big_t best_slope = 0;

// #define DUMP_FILE
#ifdef DUMP_FILE
static FILE *magnitudeFile;
static FILE *filteredFile;
#endif

static int buffer_index_plus1(int index, int buffer_len)
{
    index++;
    if (index == buffer_len)
    {
        index = 0;
    }
    return index;
}

static int buffer_index_minus1(int index, int buffer_len)
{
    index--;
    if (index < 0)
    {
        index = buffer_len - 1;
    }
    return index;
}

void pantompkins_init()
{
#ifdef DUMP_FILE
    magnitudeFile = fopen("magnitude.csv", "w+");
    filteredFile = fopen("filtered.csv", "w+");
#endif

    magnitude_buffer_i = 0;

    peakF = 0;
    peakF_temp = 0;
    best_peak_filtered = 0;
    best_slope = 0;
}

steps_t pantompkins_totalsteps(time_delta_ms_t delta_ms, accel_t accx, accel_t accy, accel_t accz)
{
    // used for debugging, needs to be removed laters
    static uint32_t sample_i = 0;
    sample_i++;

    // computes magnitude
    accel_big_t magnitude = sqrt(accx * accx + accy * accy + accz * accz);

#ifdef DUMP_FILE
    if (magnitudeFile)
    {
        fprintf(magnitudeFile, "%d\n", magnitude);
        fflush(magnitudeFile);
    }
#endif

    // advance the index
    magnitude_buffer_i = buffer_index_plus1(magnitude_buffer_i, MAGNITUDE_BUFFER_LEN);
    // add the sample to the buffer
    magnitude_buffer[magnitude_buffer_i] = magnitude;

    // 1. band pass filter
    // TODO: for now it's just a pass-through, but it needs to be implemented for real
    accel_big_t filtered = magnitude;
    // add to the buffer
    filtered_buffer_i = buffer_index_plus1(filtered_buffer_i, FILTERED_BUFFER_LEN);
    filtered_buffer[filtered_buffer_i] = filtered;

#ifdef DUMP_FILE
    if (filteredFile)
    {
        fprintf(filteredFile, "%d\n", filtered);
        fflush(filteredFile);
    }
#endif

    // detect any peak on the filtered signal:
    int pre_val_i = buffer_index_minus1(filtered_buffer_i, FILTERED_BUFFER_LEN);
    accel_big_t pre_val = filtered_buffer[pre_val_i];
    int pre_pre_val_i = buffer_index_minus1(pre_val_i, FILTERED_BUFFER_LEN);
    accel_big_t pre_pre_val = filtered_buffer[pre_pre_val_i];

    // printf("filtered i %d, prev i %d, prev prev i %d\n", filtered_buffer_i, pre_val_i, pre_pre_val_i);
    // printf("filtered %d, prev %d, prev prev %d\n", filtered, pre_val, pre_pre_val);
    // a peak is defined as x[n-2] < x[n-1] > x[n]
    if (filtered <= pre_val && pre_val > pre_pre_val)
    {
        // pre_val is a peak
        // printf("candidate peak at %d, val %d\n", sample_i - 1, pre_val);
        peakF = pre_val;
        // Fiducial mark: peak found at pre_val (previous value than the current) is it really a peaK?
        if (pre_val > best_peak_filtered) // keep track of the highest peak
            best_peak_filtered = pre_val;
    }

    // if no peak was found, increment blank time
    if (!peakF && blank_time_cnt) // no beat, decrement BlankTime
    {
        // if blank time has expired (=0) assign peak temp to peak F
        if (--blank_time_cnt == 0)
        {
            peakF = peakF_temp;
        }
    }

    // if peak was found, and blank time has expired
    // this is maybe a real peak!
    // reset blank time now
    // assign peak temp to peak I, reset peak I

    // if peak was found and blank time has not expired
    // let's see if this peak is bigger than the previously found
    // if bigger, then this is the real peak: reset blank time, assign peak I to peak temp
    // otherwise reduce blank time, if expired the assign peak temp to peak I
    // otherwise just keep decreasing blank time and assign peak I to zero

    // if in learning phase... skip for now

    // compare peak I with threshold 1 and best peak filtered with th 2
    // if bigger .... TBC

    // if peak is not big enough
    // ... TBC

    return 0;
}
