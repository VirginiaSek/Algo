#include "autocorrelation_stepcount.h"
#include "stdint.h"
#include "stdio.h" //for file operations

#include "../types.h"

#ifdef DUMP_FILE
static FILE *magnitudeFile;
static FILE *filteredFile;
static FILE *removedMeanFile;
static FILE *autocorrelationFile;
static FILE *derivativeFile;
#endif
#define NUM_AUTOCORR_LAGS 50                 // number of lags to calculate for autocorrelation. 50 lags @20Hz corresponds to a step rate of 0.4Hz...its probably not physically possible to walk much slower than this
#define DERIV_FILT_LEN 7                     // length of derivative filter
#define LPF_FILT_LEN 9                       // length of FIR low pass filter
#define AUTOCORR_DELTA_AMPLITUDE_THRESH 1000 // this is the min delta between peak and trough of autocorrelation peak
#define AUTOCORR_MIN_HALF_LEN 3              // this is the min number of points the autocorrelation peak should be on either side of the peak

static int8_t deriv_coeffs[DERIV_FILT_LEN] = {-6, 31, 0, -31, 6};
static int16_t lpf_coeffs[LPF_FILT_LEN] = {-2696, -3734, 11354, 17457, 11354, -3734, -2696};
static int32_t lpf[NUM_TUPLES] = {0};                  // low pass filtered signal
static int64_t autocorr_buff[NUM_AUTOCORR_LAGS] = {0}; // autocorrelation results
static int64_t deriv[NUM_AUTOCORR_LAGS] = {0};         // derivative

// Function to initialize the dump files
static void init_dump_files()
{
#ifdef DUMP_FILE
    magnitudeFile = fopen(DUMP_MAGNITUDE_FILE_NAME, "w+");
    filteredFile = fopen(DUMP_FILTERED_FILE_NAME, "w+");
    removedMeanFile = fopen(DUMP_REMOVED_MEAN_FILE_NAME, "w+");
    autocorrelationFile = fopen(DUMP_AUTOCORRELATION_FILE_NAME, "w+");
    derivativeFile = fopen(DUMP_DERIVATIVE_FILE_NAME, "w+");
#endif
}

// Function to close the dump files
static void close_dump_files()
{
#ifdef DUMP_FILE
    if (magnitudeFile)
        fclose(magnitudeFile);
    if (filteredFile)
        fclose(filteredFile);
    if (removedMeanFile)
        fclose(removedMeanFile);
    if (autocorrelationFile)
        fclose(autocorrelationFile);
    if (derivativeFile)
        fclose(derivativeFile);
#endif
}

// Modified SquareRoot function remains unchanged
static uint32_t SquareRoot(uint32_t a_nInput)
{
    uint32_t op = a_nInput;
    uint32_t res = 0;
    uint32_t one = 1uL << 30;

    while (one > op)
    {
        one >>= 2;
    }

    while (one != 0)
    {
        if (op >= res + one)
        {
            op = op - (res + one);
            res = res + 2 * one;
        }
        res >>= 1;
        one >>= 2;
    }
    return res;
}

// Low pass filter function
static void lowpassfilt(accel_big_t *mag_sqrt, int32_t *lpf)
{
    uint16_t n;
    uint8_t i;
    int32_t temp_lpf;
    for (n = 0; n < NUM_TUPLES; n++)
    {
        temp_lpf = 0;
        for (i = 0; i < LPF_FILT_LEN; i++)
        {
            if (n - i >= 0)
            {
                temp_lpf += (int32_t)lpf_coeffs[i] * (int32_t)mag_sqrt[n - i];
            }
        }
        lpf[n] = temp_lpf;

#ifdef DUMP_FILE
        if (filteredFile)
        {
            fprintf(filteredFile, "%u, %d\n", n, lpf[n]);
        }
#endif
    }
#ifdef DUMP_FILE
    if (filteredFile)
        fflush(filteredFile);
#endif
}

// Remove mean from filtered data
static void remove_mean(accel_big_t *lpf)
{
    accel_big_t sum = 0;
    uint16_t i;
    for (i = 0; i < NUM_TUPLES; i++)
    {
        sum += lpf[i];
    }
    sum = (accel_big_t)sum / (accel_big_t)NUM_TUPLES;

    for (i = 0; i < NUM_TUPLES; i++)
    {
        lpf[i] -= sum;

#ifdef DUMP_FILE
        if (removedMeanFile)
        {
            fprintf(removedMeanFile, "%u, %d\n", i, lpf[i]);
        }
#endif
    }
#ifdef DUMP_FILE
    if (removedMeanFile)
        fflush(removedMeanFile);
#endif
}

// Autocorrelation function
static void autocorr(accel_big_t *lpf, int64_t *autocorr_buff)
{
    uint8_t lag;
    uint16_t i;
    int64_t temp_ac;
    for (lag = 0; lag < NUM_AUTOCORR_LAGS; lag++)
    {
        temp_ac = 0;
        for (i = 0; i < NUM_TUPLES - lag; i++)
        {
            temp_ac += (int64_t)lpf[i] * (int64_t)lpf[i + lag];
        }
        autocorr_buff[lag] = temp_ac;

#ifdef DUMP_FILE
        if (autocorrelationFile)
        {
            fprintf(autocorrelationFile, "%u, %lld\n", lag, autocorr_buff[lag]);
        }
#endif
    }
#ifdef DUMP_FILE
    if (autocorrelationFile)
        fflush(autocorrelationFile);
#endif
}

// Derivative calculation
static void derivative(int64_t *autocorr_buff, int64_t *deriv)
{
    uint8_t n = 0;
    uint8_t i = 0;
    int64_t temp_deriv = 0;
    //     for (n = 0; n < NUM_AUTOCORR_LAGS; n++)
    //     {
    //         temp_deriv = 0;
    //         for (i = 0; i < DERIV_FILT_LEN; i++)
    //         {
    //             if (n - i >= 0)
    //             {
    //                 temp_deriv += deriv_coeffs[i] * autocorr_buff[n - i];
    //             }
    //         }
    //         deriv[n] = temp_deriv;

    // #ifdef DUMP_FILE
    //         if (derivativeFile)
    //         {
    //             fprintf(derivativeFile, "%u, %lld\n", n, deriv[n]);
    //         }
    // #endif
    //     }

    for (n = 0; n < NUM_AUTOCORR_LAGS; n++)
    {
        if (n > 0)
        {
            deriv[n] = autocorr_buff[n] - autocorr_buff[n - 1];
        }
        else
            deriv[0] = 0;

#ifdef DUMP_FILE
        if (derivativeFile)
        {
            fprintf(derivativeFile, "%u, %lld\n", n, deriv[n]);
        }
#endif
    }
#ifdef DUMP_FILE
    if (derivativeFile)
        fflush(derivativeFile);
#endif
}

// Algorithm to count steps
steps_t autcorr_count_steps(accel_big_t *mag_sqrt)
{
    init_dump_files();

    // Step 2: Apply low pass filter
    // lowpassfilt(mag_sqrt, lpf);

    // Step 3: Remove the mean
    remove_mean(mag_sqrt);

    // Step 4: Calculate autocorrelation
    autocorr(mag_sqrt, autocorr_buff);

    // Step 5: Calculate derivative
    derivative(autocorr_buff, deriv);

    // look for first zero crossing where derivative goes from positive to negative. that
    // corresponds to the first positive peak in the autocorrelation. look at two samples
    // instead of just one to maybe reduce the chances of getting tricked by noise
    uint8_t peak_ind = 0;
    // start index is set to 8 lags, which corresponds to a walking rate of 2.5Hz @20Hz sampling rate. its probably
    // running if its faster than this
    uint16_t i;
    for (i = 8; i < NUM_AUTOCORR_LAGS; i++)
    {
        if ((deriv[i] > 0) && (deriv[i - 1] > 0) && (deriv[i - 2] < 0) && (deriv[i - 3] < 0))
        {
            peak_ind = i - 1;
            break;
        }
    }

    // hone in on the exact peak index
    peak_ind = get_precise_peakind(autocorr_buff, peak_ind);
    // printf("peak ind: %i\n", peak_ind);

    // get autocorrelation peak stats
    uint8_t neg_slope_count = 0;
    int64_t delta_amplitude_right = 0;
    uint8_t pos_slope_count = 0;
    int64_t delta_amplitude_left = 0;
    get_autocorr_peak_stats(autocorr_buff, &neg_slope_count, &delta_amplitude_right, &pos_slope_count, &delta_amplitude_left, peak_ind);

    // now check the conditions to see if it was a real peak or not, and if so, calculate the number of steps
    uint8_t num_steps = 0;
    if ((pos_slope_count > AUTOCORR_MIN_HALF_LEN) && (neg_slope_count > AUTOCORR_MIN_HALF_LEN) && (delta_amplitude_right > AUTOCORR_DELTA_AMPLITUDE_THRESH) && (delta_amplitude_left > AUTOCORR_DELTA_AMPLITUDE_THRESH))
    {
        // the period is peak_ind/sampling_rate seconds. that corresponds to a frequency of 1/period
        // with the frequency known, and the number of seconds is 4 seconds, you can then find out the number of steps
        num_steps = (SAMPLING_RATE * WINDOW_LENGTH) / peak_ind;
    }
    else
    {
        // not a valid autocorrelation peak
        num_steps = 0;
    }

    return num_steps;
#ifdef DUMP_FILE
    close_dump_files();
#endif
}
