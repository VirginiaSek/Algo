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
#include <stddef.h>
#include "StepCountingAlgo.h"
#include "ringbuffer.h"
#include "preProcessingStage.h"
#include "motionDetectStage.h"
#include "filterStage.h"
#include "scoringStage.h"
#include "detectionStage.h"
#include "postProcessingStage.h"
#include <stdio.h>
// General data
// static steps_t oxford_stepcount;
static steps_t oxford_StepCount;
// Buffers
static ring_buffer_t rawBuf;
static ring_buffer_t ppBuf;
static ring_buffer_t mdBuf;
#ifndef SKIP_FILTER
static ring_buffer_t smoothBuf;
#endif
static ring_buffer_t peakScoreBuf;
static ring_buffer_t peakBuf;

static void increaseStepCallback(void)
{
    // printf("%d",oxford_StepCount);
    oxford_StepCount++;
}

void oxford_init()
{
    oxford_resetSteps();
    // init buffers
    ring_buffer_init(&rawBuf);
    ring_buffer_init(&ppBuf);
    ring_buffer_init(&mdBuf);
#ifndef SKIP_FILTER
    ring_buffer_init(&smoothBuf);
#endif
    ring_buffer_init(&peakScoreBuf);
    ring_buffer_init(&peakBuf);

    initPreProcessStage(&rawBuf, &ppBuf, motionDetectStage);
#ifdef SKIP_FILTER
    initMotionDetectStage(&ppBuf, &mdBuf, scoringStage);
    initScoringStage(&mdBuf, &peakScoreBuf, detectionStage);
#else
    initMotionDetectStage(&ppBuf, &mdBuf, filterStage);
    initFilterStage(&mdBuf, &smoothBuf, scoringStage);
    initScoringStage(&smoothBuf, &peakScoreBuf, detectionStage);
#endif
    initDetectionStage(&peakScoreBuf, &peakBuf, postProcessingStage);
    initPostProcessingStage(&peakBuf, &increaseStepCallback);
    // oxford_resetSteps();
}

void oxford_processSample(time_delta_ms_t delta_ms, accel_t accx, accel_t accy, accel_t accz)
{
    preProcessSample(delta_ms, accx, accy, accz);
}

void oxford_resetSteps(void)
{
    oxford_StepCount = 0;
}

void oxford_resetAlgo(void)
{
    resetPreProcess();
    resetDetection();
    resetPostProcess();
    ring_buffer_init(&rawBuf);
    ring_buffer_init(&ppBuf);
    ring_buffer_init(&mdBuf);
#ifndef SKIP_FILTER
    ring_buffer_init(&smoothBuf);
#endif
    ring_buffer_init(&peakScoreBuf);
    ring_buffer_init(&peakBuf);
}

steps_t oxford_stepcount(int delta_ms, int accx, int accy, int accz)
{

    oxford_processSample(delta_ms, accx, accy, accz);
    return oxford_StepCount;
}