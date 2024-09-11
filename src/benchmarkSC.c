#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>

#include "./dummy/dummyStepCounter.h"

int main(int argc, char *argv[])
{

    DIR *dir;
    struct dirent *entry;
    // input file
    FILE *fp;
    char line[1024];

    // output file
    FILE *out_fp;

    if (argc < 2)
    {
        printf("Usage: %s <directory> <results>\n", argv[0]);
        return 1;
    }

    if (argc < 3)
    {
        printf("Usage: %s <directory> <results>\n", argv[0]);
        return 1;
    }

    // Create output file
    out_fp = fopen(argv[2], "w");
    if (out_fp == NULL)
    {
        printf("Error creating file\n");
        return 1;
    }
    fprintf(out_fp, "FILENAME,DUMMY\n");

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
        fp = fopen(filename, "r");
        if (fp == NULL)
        {
            perror("Cannot open file");
            printf("%s", filename);
            continue;
        }

        // initialise the algorithms
        dummy_stepcount_init();
        int dummySC = 0;

        // counter of the line number
        unsigned int lineN = 0;
        unsigned int previous_ms = 0;

        while (fgets(line, sizeof(line), fp) != NULL)
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
        fclose(fp);

        // Write the results
        char out_line[1024];
        // concatenate filename and step counts
        snprintf(out_line, sizeof(out_line), "%s,%d\n", entry->d_name, dummySC);

        fputs(out_line, out_fp);
    }

    closedir(dir);

    fclose(out_fp);
    return 0;
}