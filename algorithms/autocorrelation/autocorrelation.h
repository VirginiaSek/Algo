
#include <math.h>
#include "../movement_detection/movement_detection.h"
#include "../../types.h"

void autocorrelation_stepcount_init();

// Wrapper function
steps_t autocorrelation_stepcount_totalsteps(time_delta_ms_t delta_ms, accel_t accx, accel_t accy, accel_t accz);