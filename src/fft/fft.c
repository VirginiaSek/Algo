#include <math.h>
#include <stdio.h>
#include <strings.h> // for file operations
#include <stdlib.h>

#include "../lowpass_filter.h"
#include "../types.h"

#define WINDOW_LEN 32             // sliding window length, better if power of 2 (if we want to switch to FFT), 32 samples = 2.5s , 64 samples = 5.12 s
#define WINDOW_STEP 12            // step of the sliding window, kept at roughly the longest step time (1s)
#define SAMPLING_FREQ 12.5    

// Buffers and counters
static accel_big_t signal_buffer[WINDOW_LEN] = {0};
static int signal_buffer_next_i = 0;
static double total_steps = 0;
static int samples_since_step_count = 0;

// For motion detection
static accel_big_t window_step_min, window_step_max;
#define MOVEMENT_DETECTION_THRESHOLD 1500

// Complex number structure
typedef struct {
    double real;
    double imag;
} complex_double;

// Log base 2 function
int my_log2(int N) {
    int k = N, i = 0;
    while(k) {
        k >>= 1;
        i++;
    }
    return i - 1;
}

// Check if a number is a power of 2
int check(int n) {
    return n > 0 && (n & (n - 1)) == 0;
}

// Function to calculate the reverse index based on bit permutation
int reverse(int N, int n) {
    int j, p = 0;
    for(j = 1; j <= my_log2(N); j++) {
        if(n & (1 << (my_log2(N) - j)))
            p |= 1 << (j - 1);
    }
    return p;
}

// Function to reorder the array based on the reverse index
void ordina(complex_double* f1, int N) {
    complex_double f2[WINDOW_LEN];
    for(int i = 0; i < N; i++)
        f2[i] = f1[reverse(N, i)];
    for(int j = 0; j < N; j++)
        f1[j] = f2[j];
}

// FFT transform function
void transform(complex_double* f, int N) {
    ordina(f, N);  // First reorder the array
    complex_double *W;
    W = (complex_double *)malloc(N / 2 * sizeof(complex_double));
    W[1].real = cos(-2. * M_PI / N);
    W[1].imag = sin(-2. * M_PI / N);
    W[0].real = 1;
    W[0].imag = 0;
    for(int i = 2; i < N / 2; i++) {
        W[i].real = cos(-2. * M_PI * i / N);
        W[i].imag = sin(-2. * M_PI * i / N);
    }
    int n = 1;
    int a = N / 2;
    for(int j = 0; j < my_log2(N); j++) {
        for(int i = 0; i < N; i++) {
            if(!(i & n)) {
                complex_double temp = f[i];
                complex_double Temp = W[(i * a) % (n * a)];
                Temp.real *= f[i + n].real - f[i].real;
                Temp.imag *= f[i + n].imag - f[i].imag;
                f[i].real += Temp.real;
                f[i].imag += Temp.imag;
                f[i + n].real = temp.real - Temp.real;
                f[i + n].imag = temp.imag - Temp.imag;
            }
        }
        n *= 2;
        a = a / 2;
    }
    free(W);
}

// FFT function
void FFT(complex_double* f, int N, double d) {
    transform(f, N);
    // Scale the FFT result by multiplying each value by the step size 'd'
    for(int i = 0; i < N; i++) {
        f[i].real *= d;
        f[i].imag *= d;
    }
}

// FFT initialization function
void fft_init() {
    total_steps = 0;
    samples_since_step_count = 0;
    // Initialize the signal buffer to zeros
    for (int i = 0; i < WINDOW_LEN; i++) {
        signal_buffer[i] = 0;
    }
    signal_buffer_next_i = 0;

    window_step_min = 66000;
    window_step_max = -66000;
}

// Function to count total steps
steps_t fft_stepcount_totalsteps(time_delta_ms_t delta_ms, accel_t accx, accel_t accy, accel_t accz) {
    // Calculate the magnitude of the acceleration vector
    uint16_t magn = sqrt(accx * accx + accy * accy + accz * accz);

    // Add the magnitude to the circular buffer
    signal_buffer[signal_buffer_next_i] = magn;
    signal_buffer_next_i = (signal_buffer_next_i + 1) % WINDOW_LEN;

    samples_since_step_count++;

    // Update the window's min and max values
    if (magn < window_step_min) {
        window_step_min = magn;
    }
    if (magn > window_step_max) {
        window_step_max = magn;
    }

    // After WINDOW_STEP samples, check if it's time to perform the analysis
    if (samples_since_step_count == WINDOW_STEP) {
        samples_since_step_count = 0;

        // Check if there is significant movement (difference between max and min)
        if (window_step_max - window_step_min > MOVEMENT_DETECTION_THRESHOLD) {
            // Prepare the data for FFT
            complex_double fft_input[WINDOW_LEN];
            for (int i = 0; i < WINDOW_LEN; i++) {
                fft_input[i].real = (double)signal_buffer[i];
                fft_input[i].imag = 0.0;
            }

            // Perform the FFT
            FFT(fft_input, WINDOW_LEN, 1.0);

            // Find the dominant frequency
            double max_magnitude = 0.0;
            int dominant_freq_index = 0;
            for (int i = 1; i < WINDOW_LEN / 2 + 1; i++) {
                double magnitude = sqrt(fft_input[i].real * fft_input[i].real + fft_input[i].imag * fft_input[i].imag);
                if (magnitude > max_magnitude) {
                    max_magnitude = magnitude;
                    dominant_freq_index = i;
                }
            }

            // Calculate the dominant frequency in Hz
            double dominant_freq = (double)dominant_freq_index * SAMPLING_FREQ / WINDOW_LEN;

            // Calculate the number of steps based on the dominant frequency
            double num_steps = dominant_freq * (WINDOW_STEP / SAMPLING_FREQ);
            total_steps += num_steps;
        }

        // Reset the min and max values for the window
        window_step_min = 66000;
        window_step_max = -66000;
    }

    // Return the total step count
    return (steps_t)total_steps;
}
