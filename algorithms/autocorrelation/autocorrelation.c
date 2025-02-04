#include "autocorrelation.h"
#include "stdint.h"
#include "stdio.h"   //for file operations
#include "strings.h" //for file operations

#include "../../utils/lowpass_filter/lowpass_filter.h"

#include "../../types.h"

// #define DUMP_FILE

#define SAMPLING_RATE 12.5                         // 12.5 hz sampling rate
#define NUM_TUPLES 50                              // 4 seconds worth of data
#define WINDOW_LENGTH (NUM_TUPLES / SAMPLING_RATE) // window length in seconds: 4

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

#define NUM_AUTOCORR_LAGS 16      // number of lags to calculate for autocorrelation. At a minimum step rate of 1 step /s -> 1000 / sampling_period(ms) = sampling frequency -> add a few more -> 16 at 12.5 Hz
#define FIRST_AUTOCORR_PEAK_LAG 4 // corresponds to the first feasible autocorrelation lag -> at a max step rate of 3 steps /s (running) -> 333ms / sampling_period(ms) = 4 at 12.5 Hz

#define DERIV_FILT_LEN 7                    // length of derivative filter
#define AUTOCORR_DELTA_AMPLITUDE_THRESH 1e6 // this is the min delta between peak and trough of autocorrelation peak
#define AUTOCORR_MIN_HALF_LEN 2             // this is the min number of points the autocorrelation peak should be on either side of the peak

static int8_t deriv_coeffs[DERIV_FILT_LEN] = {-6, 31, 0, -31, 6};

static accel_big_t filtered_buff[NUM_TUPLES] = {0};    // low pass filtered data
static int64_t autocorr_buff[NUM_AUTOCORR_LAGS] = {0}; // autocorrelation results
static int64_t deriv[NUM_AUTOCORR_LAGS] = {0};         // derivative

static LPFilter autocorr_lpf;

#ifdef DUMP_FILE
#define DUMP_MAGNITUDE_FILE_NAME "magnitude.csv"
#define DUMP_FILTERED_FILE_NAME "filtered.csv"
#define DUMP_REMOVED_MEAN_FILE_NAME "removed_mean.csv"
#define DUMP_AUTOCORRELATION_FILE_NAME "autocorrelation"
#define DUMP_DERIVATIVE_FILE_NAME "derivative"

static int autocorr_passes = 0; // counter of how many times the autocorr has been called

static FILE *magnitudeFile;
static FILE *filteredFile;
static FILE *removedMeanFile;
static FILE *autocorrelationFile;
static FILE *derivativeFile;
#endif

void autocorrelation_stepcount_init()
{
    LPFilter_init(&autocorr_lpf);
    detect_movement_init(&autocor_mov_detect_buffer);

    for (int i = 0; i < NUM_AUTOCORR_LAGS; i++)
    {
        autocorr_buff[i] = 0;
        deriv[i] = 0;
    }
    autocorrelation_steps_counter = 0;
    movement_samples = 0;

#ifdef DUMP_FILE
    magnitudeFile = fopen(DUMP_MAGNITUDE_FILE_NAME, "w");
    filteredFile = fopen(DUMP_FILTERED_FILE_NAME, "w");
    removedMeanFile = fopen(DUMP_REMOVED_MEAN_FILE_NAME, "w");

    autocorr_passes = 0;
#endif
}

// sends the buffer to a low pass filter
static void lowpass_buffer(accel_big_t *input_buffer, accel_big_t *output_buffer)
{
    for (int i = 0; i < NUM_TUPLES; i++)
    {
        // output_buffer[i] = input_buffer[i];
        LPFilter_put(&autocorr_lpf, input_buffer[i]);
        output_buffer[i] = LPFilter_get(&autocorr_lpf);
#ifdef DUMP_FILE
        if (filteredFile)
        {
            fprintf(filteredFile, "%d\n", output_buffer[i]);
            fflush(filteredFile);
        }
#endif
    }
}

// Remove mean from filtered data
static void remove_mean(accel_big_t *buffer)
{
    accel_big_t sum = 0;
    uint16_t i;
    for (i = 0; i < NUM_TUPLES; i++)
    {
        sum += buffer[i];
    }
    sum = (accel_big_t)sum / (accel_big_t)NUM_TUPLES;

    for (i = 0; i < NUM_TUPLES; i++)
    {
        buffer[i] -= sum;

#ifdef DUMP_FILE
        if (removedMeanFile)
        {
            fprintf(removedMeanFile, "%d\n", buffer[i]);
        }
#endif
    }
#ifdef DUMP_FILE
    if (removedMeanFile)
        fflush(removedMeanFile);
#endif
}

// Autocorrelation function
static void autocorr(accel_big_t *buffer, int64_t *autocorr_buff)
{
#ifdef DUMP_FILE
    autocorr_passes++;
    char autocorrFileName[100] = DUMP_AUTOCORRELATION_FILE_NAME;
    char idxstr[5];
    sprintf(idxstr, "%d", autocorr_passes);
    strcat(autocorrFileName, idxstr);
    strcat(autocorrFileName, ".csv");
    autocorrelationFile = fopen(autocorrFileName, "w+");
#endif

    uint8_t lag;
    uint16_t i;
    int64_t temp_ac;
    for (lag = 0; lag < NUM_AUTOCORR_LAGS; lag++)
    {
        temp_ac = 0;
        for (i = 0; i < NUM_TUPLES - lag; i++)
        {
            temp_ac += (int64_t)buffer[i] * (int64_t)buffer[i + lag];
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
    {
        fflush(autocorrelationFile);
        fclose(autocorrelationFile);
    }
#endif
}

// Derivative calculation
static void derivative(int64_t *autocorr_buff, int64_t *deriv)
{
#ifdef DUMP_FILE
    char derivativeFilename[100] = DUMP_DERIVATIVE_FILE_NAME;
    char idxstr[5];
    sprintf(idxstr, "%d", autocorr_passes);
    strcat(derivativeFilename, idxstr);
    strcat(derivativeFilename, ".csv");
    derivativeFile = fopen(derivativeFilename, "w+");
#endif

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
            fprintf(derivativeFile, "%lld\n", deriv[n]);
        }
#endif
    }
#ifdef DUMP_FILE
    if (derivativeFile)
    {
        fflush(derivativeFile);
        fclose(autocorrelationFile);
    }
#endif
}

// use the original autocorrelation signal to hone in on the
// exact peak index. this corresponds to the point where the points to the
// left and right are less than the current point
static uint8_t get_precise_peakind(int64_t *autocorr_buff, uint8_t peak_ind)
{
    uint8_t loop_limit = 0;
    if ((autocorr_buff[peak_ind] > autocorr_buff[peak_ind - 1]) && (autocorr_buff[peak_ind] > autocorr_buff[peak_ind + 1]))
    {
        // peak_ind is perfectly set at the peak. nothing to do
    }
    else if ((autocorr_buff[peak_ind] > autocorr_buff[peak_ind + 1]) && (autocorr_buff[peak_ind] < autocorr_buff[peak_ind - 1]))
    {
        // peak is to the left. keep moving in that direction
        loop_limit = 0;
        while ((autocorr_buff[peak_ind] > autocorr_buff[peak_ind + 1]) && (autocorr_buff[peak_ind] < autocorr_buff[peak_ind - 1]) && (loop_limit < 10))
        {
            peak_ind = peak_ind - 1;
            loop_limit++;
        }
    }
    else
    {
        // peak is to the right. keep moving in that direction
        loop_limit = 0;
        while ((autocorr_buff[peak_ind] > autocorr_buff[peak_ind - 1]) && (autocorr_buff[peak_ind] < autocorr_buff[peak_ind + 1]) && (loop_limit < 10))
        {
            peak_ind = peak_ind + 1;
            loop_limit++;
        }
    }
    return peak_ind;
}

// take a look at the original autocorrelation signal at index i and see if
// it's a real peak or if it's just a fake "noisy" peak corresponding to
// non-walking. Basically just count the number of points of the
// autocorrelation peak to the right and left of the peak. this function gets
// the number of points to the right and left of the peak, as well as the delta amplitude
static void get_autocorr_peak_stats(int64_t *autocorr_buff, uint8_t *neg_slope_count, int64_t *delta_amplitude_right, uint8_t *pos_slope_count, int64_t *delta_amplitude_left, uint8_t peak_ind)
{

    // first look to the right of the peak. walk forward until the slope begins decreasing
    uint8_t neg_slope_ind = peak_ind;
    uint16_t loop_limit = NUM_AUTOCORR_LAGS - 1;
    while ((autocorr_buff[neg_slope_ind + 1] - autocorr_buff[neg_slope_ind] < 0) && (neg_slope_ind < loop_limit))
    {
        *neg_slope_count = *neg_slope_count + 1;
        neg_slope_ind = neg_slope_ind + 1;
    }

    // get the delta amplitude between peak and right trough
    *delta_amplitude_right = autocorr_buff[peak_ind] - autocorr_buff[neg_slope_ind];

    // next look to the left of the peak. walk backward until the slope begins increasing
    uint8_t pos_slope_ind = peak_ind;
    loop_limit = 0;
    while ((autocorr_buff[pos_slope_ind] - autocorr_buff[pos_slope_ind - 1] > 0) && (pos_slope_ind > loop_limit))
    {
        *pos_slope_count = *pos_slope_count + 1;
        pos_slope_ind = pos_slope_ind - 1;
    }

    // get the delta amplitude between the peak and the left trough
    *delta_amplitude_left = autocorr_buff[peak_ind] - autocorr_buff[pos_slope_ind];
}

// Algorithm to count steps on the current buffer
static steps_t autcorr_count_steps(accel_big_t *mag_sqrt_buffer)
{
#ifdef DUMP_FILE
    for (int i = 0; i < NUM_TUPLES; i++)
    {
        if (magnitudeFile)
        {
            fprintf(magnitudeFile, "%u, %d\n", i, mag_sqrt_buffer[i]);
            fflush(magnitudeFile);
        }
    }
#endif

    // Step 1: Apply low pass filter
    lowpass_buffer(mag_sqrt_buffer, filtered_buff);

    // Step 2: Remove the mean
    remove_mean(filtered_buff);

    // Step 3: Calculate autocorrelation
    autocorr(filtered_buff, autocorr_buff);

    // Step 4: Calculate derivative
    derivative(autocorr_buff, deriv);

    // Step 5: find peak
    // look for first zero crossing where derivative goes from positive to negative. that
    // corresponds to the first positive peak in the autocorrelation. look at two samples
    // instead of just one to maybe reduce the chances of getting tricked by noise
    uint8_t peak_ind = 0;
    // start index is set to a rate that is feasible when running
    uint16_t i;
    for (i = FIRST_AUTOCORR_PEAK_LAG; i < NUM_AUTOCORR_LAGS; i++)
    {
        if ((deriv[i] < 0) && (deriv[i - 1] < 0) && (deriv[i - 2] > 0) && (deriv[i - 3] > 0))
        {
            peak_ind = i - 1;
            break;
        }
    }

    // hone in on the exact peak index
    peak_ind = get_precise_peakind(autocorr_buff, peak_ind);
    // printf("peak ind: %i\n", peak_ind);

    uint8_t num_steps = 0;

    // now check the conditions to see if it was a real peak or not, and if so, calculate the number of steps
    // get autocorrelation peak stats
    uint8_t neg_slope_count = 0;
    int64_t delta_amplitude_right = 0;
    uint8_t pos_slope_count = 0;
    int64_t delta_amplitude_left = 0;
    get_autocorr_peak_stats(autocorr_buff, &neg_slope_count, &delta_amplitude_right, &pos_slope_count, &delta_amplitude_left, peak_ind);
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

    // printf("num steps: %i\n", num_steps);
    return num_steps;
}

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
