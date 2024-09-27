/*
The MIT License (MIT)

Copyright (c) 2020 Anna Brondin and Marcus Nordström

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
#include "preProcessingStage.h"
#include "config.h"

#ifdef DUMP_FILE
#include <stdio.h>
static FILE
    *magnitudeFile;
static FILE *interpolatedFile;
#endif

static ring_buffer_t *inBuff;
static ring_buffer_t *outBuff;
static void (*nextStage)(void);
static uint8_t samplingPeriod = 80;    // in ms, this can be smaller than the actual sampling frequency, but it will result in more computations
static uint16_t timeScalingFactor = 1; // use this for adjusting time to ms, in case the clock has higher precision
static time_delta_ms_t lastSampleTime = -1;

void initPreProcessStage(ring_buffer_t *pInBuff, ring_buffer_t *pOutBuff, void (*pNextStage)(void))
{
    inBuff = pInBuff;
    outBuff = pOutBuff;
    nextStage = pNextStage;

#ifdef DUMP_FILE
    magnitudeFile = fopen(DUMP_MAGNITUDE_FILE_NAME, "w+");
    interpolatedFile = fopen(DUMP_INTERPOLATED_FILE_NAME, "w+");
#endif
}

static void outPutDataPoint(data_point_t dp)
{
    lastSampleTime = dp.time;
    ring_buffer_queue(outBuff, dp);
    (*nextStage)();

#ifdef DUMP_FILE
    if (interpolatedFile)
    {
        if (!fprintf(interpolatedFile, "%lld, %lld\n", dp.time, dp.magnitude))
            puts("error writing file");
        fflush(interpolatedFile);
    }
#endif
}

// Variabile di stato per tracciare il tempo cumulativo
static time_delta_ms_t cumulative_time = 0;

void preProcessSample(time_delta_ms_t delta_ms, accel_t accx, accel_t accy, accel_t accz)
{

    // Aggiornamento del tempo cumulativo
    cumulative_time += delta_ms;

    // Adattamento del tempo in base al fattore di scala
    time_delta_ms_t time = cumulative_time / timeScalingFactor;

    accel_big_t magnitude = (accel_big_t)sqrt((accel_big_t)(accx * accx + accy * accy + accz * accz));
    data_point_t dataPoint;
    dataPoint.time = time;
    dataPoint.magnitude = magnitude;

#ifdef DUMP_FILE
    if (magnitudeFile)
    {
        if (!fprintf(magnitudeFile, "%lld, %lld\n", dataPoint.time, dataPoint.magnitude))
            puts("error writing file");
    }
#endif

    outPutDataPoint(dataPoint);
}

void resetPreProcess(void)
{
    lastSampleTime = -1;

#ifdef DUMP_FILE
    if (magnitudeFile)
    {
        fflush(magnitudeFile);
    }
    if (interpolatedFile)
    {
        fflush(interpolatedFile);
    }
#endif
}