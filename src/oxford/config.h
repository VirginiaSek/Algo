#include <inttypes.h>
#include <math.h>

// magnitude of acceleration
// this should be 1 bit bigger than accel_t
// example: if the accelerometer works at 12 bits, magnitude should be 13 bits, therefore choosing int16_t for both will do
typedef int64_t magnitude_t;

// accumulator of magnitude, should be bigger than magnitude
// the size depends on the length of the filter in the filtering stage
// example: accelerometer works at 12 bits, the filter has 8 taps -> accumulator should be at least 12 + 3 bits -> using int16_t will do
typedef uint64_t accumulator_t;

// difference between min and max acceleration to detect motion
#define MOTION_THRESHOLD 1500

// window size of the scoring phase
#define WINDOW_SIZE 10

// detection threshold whole part and fractional part
#define DETECTION_TRHE_WHOLE 1
#define DETECTION_TRHE_PART 7

// time in ms between which to discard steps
// 3 steps /s is a reasonable maximum
#define TIME_THRE 300

// skip filtering step
#define SKIP_FILTER

// use this to allow dumping each stage on file, useful for debugging
// #define DUMP_FILE
#define DUMP_MAGNITUDE_FILE_NAME "magnitude.csv"
#define DUMP_INTERPOLATED_FILE_NAME "interpolated.csv"
#define DUMP_FILTERED_FILE_NAME "filtered.csv"
#define DUMP_SCORING_FILE_NAME "scoring.csv"
#define DUMP_DETECTION_FILE_NAME "detection.csv"
#define DUMP_POSTPROC_FILE_NAME "postproc.csv"