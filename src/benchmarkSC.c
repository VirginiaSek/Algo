#include <stdio.h>
#include "detection.h"

int main(int argc, char *argv[])
{
    detection_stage_init();
    for (int i = 0; i < 60; i++)
    {
        accel_big_t accel = 0;
        if (i < 30)
            accel = 10000;
        else
            accel = 1;
        bool ismov = detect_movement(accel);

        printf("iteration %d , added:  %d\n", i, accel);
        // printf("is mov: %d %d\n", i, ismov);
    }
}