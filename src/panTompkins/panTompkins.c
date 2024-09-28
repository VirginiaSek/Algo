#include "panTompkins.h"
#include <stdio.h>
#include <math.h> // For sqrt()

#define WINDOWSIZE 20 // Window size for the integrator (~150ms).
#define FS 360        // Sampling frequency. Adjust based on your data.
#define BUFFSIZE 600  // Buffer size. Must fit more than 1.66 times a step interval.
#define DELAY 22      // Delay introduced by filters.

// Global variable to track steps
int pt_total_steps = 0;
dataType threshold_i1 = 0.5; // Threshold for step detection

/*
    Main loop for step detection based on modified Pan-Tompkins.
    This function now accepts accelerometer data directly.
*/
void panTompkins(dataType accx, dataType accy, dataType accz)
{
    static dataType signal[BUFFSIZE], dcblock[BUFFSIZE], lowpass[BUFFSIZE], highpass[BUFFSIZE], derivative[BUFFSIZE], squared[BUFFSIZE], integral[BUFFSIZE];
    static int sample = 0;
    int current;

    // Buffer shifting logic
    if (sample >= BUFFSIZE)
    {
        for (int i = 0; i < BUFFSIZE - 1; i++)
        {
            signal[i] = signal[i + 1];
            dcblock[i] = dcblock[i + 1];
            lowpass[i] = lowpass[i + 1];
            highpass[i] = highpass[i + 1];
            derivative[i] = derivative[i + 1];
            squared[i] = squared[i + 1];
            integral[i] = integral[i + 1];
        }
        current = BUFFSIZE - 1;
    }
    else
    {
        current = sample;
    }

    // Compute the magnitude of the accelerometer data (instead of ECG signal)
    signal[current] = sqrt(accx * accx + accy * accy + accz * accz);

    // DC Block filter (optional, remove if unnecessary)
    if (current >= 1)
    {
        dcblock[current] = signal[current] - signal[current - 1] + 0.995 * dcblock[current - 1];
    }
    else
    {
        dcblock[current] = 0;
    }

    // Low Pass filter
    lowpass[current] = dcblock[current];
    if (current >= 1)
        lowpass[current] += 2 * lowpass[current - 1];
    if (current >= 2)
        lowpass[current] -= lowpass[current - 2];
    if (current >= 6)
        lowpass[current] -= 2 * dcblock[current - 6];
    if (current >= 12)
        lowpass[current] += dcblock[current - 12];

    // High Pass filter
    highpass[current] = -lowpass[current];
    if (current >= 1)
        highpass[current] -= highpass[current - 1];
    if (current >= 16)
        highpass[current] += 32 * lowpass[current - 16];
    if (current >= 32)
        highpass[current] += lowpass[current - 32];

    // Derivative filter
    derivative[current] = highpass[current];
    if (current > 0)
        derivative[current] -= highpass[current - 1];

    // Squaring
    squared[current] = derivative[current] * derivative[current];

    // Moving-Window Integration
    integral[current] = 0;
    for (int i = 0; i < WINDOWSIZE; i++)
    {
        if (current >= (dataType)i)
            integral[current] += squared[current - i];
        else
            break;
    }
    integral[current] /= (dataType)WINDOWSIZE;

    // Peak detection logic (step detection)
    if (integral[current] >= threshold_i1)
    {
        pt_total_steps++; // Increment step count
    }

    sample++;
}
