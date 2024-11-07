#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include "./types.h"

#include "./rolling_stats.h"

#include "./dummy/dummyStepCounter.h"
#include "./bangle_simple/bangle_simple.h"
#include "./espruino/espruino.h"
#include "./oxford/oxford.h"
#include "./panTompkins/pt.h"
#include "./autocorrelation/autocorrelation.h"
#include "./autocorrelation2/autocorrelation2.h"
#include "./fft/fft.h"

typedef struct Algo
{
    char *name;
    Stats *stats;
    void (*init)();
    steps_t (*step_count)(time_delta_ms_t delta_ms, accel_t accx, accel_t accy, accel_t accz);
    steps_t counter;
    clock_t total_time;
} Algo;

////////////////////////////////////
// START MODIFY HERE TO ADD NEW ALGO

// all algorithms:
const int algoN = 8; // change this to algo number!
Algo algos[algoN];

void createAlgos()
{
    algos[0] = (Algo){
        .name = "Dummy",
        .stats = malloc(sizeof(Stats)),
        .init = dummy_stepcount_init,
        .step_count = dummy_stepcount,
        .counter = 0,
    };

    algos[1] = (Algo){
        .name = "BangleSimple",
        .stats = malloc(sizeof(Stats)),
        .init = bangle_simple_init,
        .step_count = bangle_simple_stepcount,
        .counter = 0,
    };

    algos[2] = (Algo){
        .name = "Espruino",
        .stats = malloc(sizeof(Stats)),
        .init = espruino_stepcount_init,
        .step_count = espruino_stepcount,
        .counter = 0,
    };

    algos[3] = (Algo){
        .name = "Oxford",
        .stats = malloc(sizeof(Stats)),
        .init = oxford_stepcount_init,
        .step_count = oxford_stepcount_totalsteps,
        .counter = 0,
    };

    algos[4] = (Algo){
        .name = "PanTompkins",
        .stats = malloc(sizeof(Stats)),
        .init = pantompkins_init, // Use the initialization wrapper
        .step_count = pantompkins_totalsteps,
        .counter = 0,
    };

    algos[5] = (Algo){
        .name = "Autocorrelation",
        .stats = malloc(sizeof(Stats)),
        .init = autocorrelation_stepcount_init, // Use the initialization wrapper
        .step_count = autocorrelation_stepcount_totalsteps,
        .counter = 0,
    };

    algos[6] = (Algo){
        .name = "Autocorrelation2",
        .stats = malloc(sizeof(Stats)),
        .init = autocorrelation_2_init, // Use the initialization wrapper
        .step_count = autocorrelation2_stepcount_totalsteps,
        .counter = 0,
    };
     algos[6] = (Algo){
        .name = "fft",
        .stats = malloc(sizeof(Stats)),
        .init = fft_init, // Use the initialization wrapper
        .step_count = fft_stepcount_totalsteps,
        .counter = 0,
    };
}

// END MODIFY HERE TO ADD NEW ALGO
////////////////////////////////////

int main(int argc, char *argv[])
{
    if (argc < 4)
    {
        printf("Usage: %s <input directory> <reference> <output directory>\n", argv[0]);
        return 1;
    }

    DIR *dir;
    struct dirent *entry;
    // open the input directory
    dir = opendir(argv[1]);
    if (dir == NULL)
    {
        perror("Cannot open directory");
        return 1;
    }

    // input reference file
    FILE *ref_fp;
    char ref_line[1024];
    // open the reference csv file
    ref_fp = fopen(argv[2], "r");
    if (ref_fp == NULL)
    {
        perror("Cannot open reference file");
        printf("%s", argv[2]);
        return 1;
    }

    // Create results file
    FILE *out_fp;
    // add path to the filename
    char out_filename[256];
    snprintf(out_filename, sizeof(out_filename), "%s/results.csv", argv[3]);
    out_fp = fopen(out_filename, "w");
    if (out_fp == NULL)
    {
        perror("Error creating results file\n");
        return 1;
    }

    createAlgos();

    // init all stats
    for (int i = 0; i < algoN; i++)
    {
        rolling_stats_reset(algos[i].stats);
    }

    // input acceleration file
    FILE *accel_fp;
    char line[1024];

    // read each file
    while ((entry = readdir(dir)) != NULL)
    {
        if (strstr(entry->d_name, ".csv") == 0)
        {
            // exclude non csv files
            continue;
        }

        // add path to the filename
        char filename[256];
        snprintf(filename, sizeof(filename), "%s/%s", argv[1], entry->d_name);

        // open the file
        accel_fp = fopen(filename, "r");
        if (accel_fp == NULL)
        {
            perror("Cannot open file");
            printf("%s", filename);
            continue;
        }

        int ref_imu = -1;
        int ref_lineN = 0;

        // search for reference scanning the reference cvs file
        // printf("Searching for %s in reference CSV\n", entry->d_name);
        while (fgets(ref_line, sizeof(ref_line), ref_fp) != NULL)
        {
            // Remove trailing newline
            ref_line[strcspn(ref_line, "\n")] = 0;

            ref_lineN++;
            if (ref_lineN > 1)
            {
                int subj_number = 0;
                char ref_filename[256];
                int bangle_ref = 0;
                int IMU_ref = 0;
                char activity[256];
                if (sscanf(ref_line, "%d;%[a-zA-Z0-9_];%d;%d;%s", &subj_number, ref_filename, &bangle_ref, &IMU_ref, activity) == 5)
                {
                    strcat(ref_filename, ".csv");
                    // printf("%d %s %s\n", subj_number, ref_filename, entry->d_name);
                    if (strcasecmp(ref_filename, entry->d_name) == 0)
                    {
                        ref_imu = IMU_ref;
                        // printf("File found in ref %s \n", entry->d_name);
                    }
                }
            }
        }
        // rewind file pointer so it can be re-read
        rewind(ref_fp);
        if (ref_imu == -1)
        {
            printf("Cannot find file %s in reference CSV\n", entry->d_name);
            continue;
        }

        // initialise the algorithms
        for (int i = 0; i < algoN; i++)
        {
            algos[i].counter = 0;
            algos[i].total_time = 0;
            algos[i].init();
        }

        // open the output file
        // Construct the full path to the file
        char steps_filepath[256];
        snprintf(steps_filepath, sizeof(steps_filepath), "%s/steps_%s.csv", argv[3], entry->d_name);

        // Open the file for writing
        FILE *fps = fopen(steps_filepath, "w");
        if (fps == NULL)
        {
            perror("Error opening steps file");
            return 1;
        }

        // write header of steps output file
        fprintf(fps, "Time,");
        for (int i = 0; i < algoN; i++)
        {
            fprintf(fps, "%s", algos[i].name);
            if (i < algoN - 1)
                fprintf(fps, ",");
        }
        fprintf(fps, "\n");

        // counter of the line number
        unsigned int lineN = 0;
        unsigned int previous_ms = 0;
        unsigned int first_ms = -1;

        while (fgets(line, sizeof(line), accel_fp) != NULL)
        {
            // Process the line here:
            // printf("%s", line);

            lineN++;
            // discard first line, used for header
            if (lineN > 1)
            {
                // Remove trailing newline
                line[strcspn(line, "\n")] = 0;

                int ms, accx, accy, accz;

                // Parse integer values using sscanf
                if (sscanf(line, "%d,%d,%d,%d", &ms, &accx, &accy, &accz) != 4)
                {
                    printf("Error parsing line: %s\n", line);
                    continue;
                }

                if (first_ms == -1)
                {
                    first_ms = ms;
                }

                // Process the extracted integer values
                // printf("Values: %d, %d, %d, %d\n", ms, accx, accy, accy);

                int delta_ms = 0;
                if (lineN > 2)
                    delta_ms = ms - previous_ms;

                fprintf(fps, "%d,", ms);

                // call all algorithms here:
                for (int i = 0; i < algoN; i++)
                {
                    clock_t start_t;
                    start_t = clock();
                    algos[i].counter = algos[i].step_count(delta_ms, accx, accy, accz);
                    algos[i].total_time += clock() - start_t;
                    // write header of steps output file
                    fprintf(fps, "%d", algos[i].counter);
                    if (i < algoN - 1)
                    {
                        fprintf(fps, ",");
                    }
                }
                fprintf(fps, "\n");

                previous_ms = ms;
            }
        }
        fclose(accel_fp);

        // Write the results
        fprintf(out_fp, "%s,%d,", entry->d_name, ref_imu);
        for (int i = 0; i < algoN; i++)
        {
            fprintf(out_fp, "%d,", algos[i].counter);
            fprintf(out_fp, "%lu", algos[i].total_time);

            if (i < algoN - 1)
                fprintf(out_fp, ",");
        }
        fprintf(out_fp, "\n");

        // add error to statistics
        for (int i = 0; i < algoN; i++)
        {
            double error = (double)algos[i].counter - (double)ref_imu;
            double time_ms = ((double)(previous_ms - first_ms) / (1000 * 60));
            double error_minute = error / time_ms;
            rolling_stats_addValue(error_minute, algos[i].stats);
            // printf("Time minutes: %.1f, samples N: %d\n", time_ms, lineN);
        }
    }

    closedir(dir);

    fclose(out_fp);

    // print the stats
    for (int i = 0; i < algoN; i++)
    {
        printf("%s Mean: %.1f Std: %.1f\n", algos[i].name, rolling_stats_get_mean(algos[i].stats), rolling_stats_get_standard_deviation(algos[i].stats, 0));
    }
    return 0;
}