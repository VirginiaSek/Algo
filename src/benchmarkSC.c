#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>

#include "./dummy/dummyStepCounter.h"

int main(int argc, char *argv[])
{

    DIR *dir;
    struct dirent *entry;
    // input acceleration file
    FILE *accel_fp;
    char line[1024];

    // input reference file
    FILE *ref_fp;
    char ref_line[1024];

    // output file
    FILE *out_fp;

    if (argc < 4)
    {
        printf("Usage: %s <directory> <reference> <results>\n", argv[0]);
        return 1;
    }

    // open the reference csv file
    ref_fp = fopen(argv[2], "r");
    if (ref_fp == NULL)
    {
        perror("Cannot open reference file");
        printf("%s", argv[2]);
        return 1;
    }

    // Create output file
    out_fp = fopen(argv[3], "w");
    if (out_fp == NULL)
    {
        perror("Error creating results file\n");
        return 1;
    }
    fprintf(out_fp, "FILENAME,REF,DUMMY\n");

    dir = opendir(argv[1]);
    if (dir == NULL)
    {
        perror("Cannot open directory");
        return 1;
    }

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
                if (sscanf(ref_line, "%d,%[a-zA-Z0-9_],%d,%d,%s", &subj_number, ref_filename, &bangle_ref, &IMU_ref, activity) == 5)
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
        dummy_stepcount_init();
        int dummySC = 0;

        // counter of the line number
        unsigned int lineN = 0;
        unsigned int previous_ms = 0;

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

                // Process the extracted integer values
                // printf("Values: %d, %d, %d, %d\n", ms, accx, accy, accy);

                int delta_ms = 0;
                if (lineN > 2)
                    delta_ms = ms - previous_ms;

                // call all algorithms here:
                dummySC = dummy_stepcount(delta_ms, accx, accy, accz);

                previous_ms = ms;
            }
        }
        fclose(accel_fp);

        // Write the results
        char out_line[1024];
        // concatenate filename and step counts
        snprintf(out_line, sizeof(out_line), "%s,%d,%d\n", entry->d_name, ref_imu, dummySC);

        fputs(out_line, out_fp);
    }

    closedir(dir);

    fclose(out_fp);
    return 0;
}