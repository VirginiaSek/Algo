#include "lowpass_filter.h"

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

static accel_big_t filter_taps[LPFILTER_TAP_NUM] = {
    -342,
    848,
    2984,
    1529,
    -3087,
    -1143,
    10308,
    17553,
    10308,
    -1143,
    -3087,
    1529,
    2984,
    848,
    -342};

void LPFilter_init(LPFilter *f)
{
    int i;
    for (i = 0; i < LPFILTER_TAP_NUM; ++i)
        f->history[i] = 0;
    f->last_index = 0;
}

void LPFilter_put(LPFilter *f, accel_big_t input)
{
    f->history[f->last_index++] = input;
    if (f->last_index == LPFILTER_TAP_NUM)
        f->last_index = 0;
}

accel_big_t LPFilter_get(LPFilter *f)
{
    long long acc = 0;
    int index = f->last_index, i;
    for (i = 0; i < LPFILTER_TAP_NUM; ++i)
    {
        index = index != 0 ? index - 1 : LPFILTER_TAP_NUM - 1;
        acc += (long long)f->history[index] * filter_taps[i];
    };
    return acc >> 16;
}
