#ifndef LPFILTER_H_
#define LPFILTER_H_

#include "../../types.h"

/*

FIR filter designed with
 http://t-filter.appspot.com

sampling frequency: 12.5 Hz

fixed point precision: 16 bits

* 0 Hz - 3 Hz
  gain = 1
  desired ripple = 5 dB
  actual ripple = n/a

* 4 Hz - 6.25 Hz
  gain = 0
  desired attenuation = -40 dB
  actual attenuation = n/a

*/

#define LPFILTER_TAP_NUM 15

typedef struct LPFilter
{
  accel_big_t history[LPFILTER_TAP_NUM];
  unsigned int last_index;
} LPFilter;

void LPFilter_init(LPFilter *f);
void LPFilter_put(LPFilter *f, accel_big_t input);
accel_big_t LPFilter_get(LPFilter *f);

#endif