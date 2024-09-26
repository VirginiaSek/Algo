#include <inttypes.h>
#include <math.h>

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
// #define SKIP_FILTER

// use this to allow dumping each stage on file, useful for debugging
// #define DUMP_FILE
#define DUMP_MAGNITUDE_FILE_NAME "magnitude.csv"
#define DUMP_INTERPOLATED_FILE_NAME "interpolated.csv"
#define DUMP_FILTERED_FILE_NAME "filtered.csv"
#define DUMP_SCORING_FILE_NAME "scoring.csv"
#define DUMP_DETECTION_FILE_NAME "detection.csv"
#define DUMP_POSTPROC_FILE_NAME "postproc.csv"