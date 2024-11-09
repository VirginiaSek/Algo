#ifndef PAN_TOMPKINS
#define PAN_TOMPKINS

#include <stdbool.h>
#include "../../types.h"

void pantompkins_init();

steps_t pantompkins_totalsteps(time_delta_ms_t delta_ms, accel_t accx, accel_t accy, accel_t accz);

#endif
