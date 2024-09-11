#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>

const char directory_name[] = "./tests";

int main(int argc, char *argv[])
{

    DIR *dir;
    struct dirent *entry;
    FILE *fp;
    char line[1024];

    // open the directory with the CSV files
    dir = opendir(directory_name);
    if (dir == NULL)
    {
        perror(strcat("Cannot open directory ", directory_name));
        return 1;
    }

    // read each file
    while ((entry = readdir(dir)) != NULL)
    {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
        {
            // exclude . and ..
            continue;
        }

        // add path to the filename
        char filename[256];
        snprintf(filename, sizeof(filename), "%s/%s", directory_name, entry->d_name);

        // open the file
        fp = fopen(filename, "r");
        if (fp == NULL)
        {
            perror(strcat("Cannot open file", filename));
            continue;
        }

        // counter of the line number
        int lineN = 0;

        while (fgets(line, sizeof(line), fp) != NULL)
        {
            // Process the line here:
            printf("%s", line);

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
                printf("Values: %d, %d, %d, %d\n", ms, accx, accy, accy);
            }
        }

        fclose(fp);
    }

    closedir(dir);
    return 0;
}