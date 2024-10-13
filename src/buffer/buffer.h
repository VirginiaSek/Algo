#ifndef BUFFER_H
#define BUFFER_H

#include <stdbool.h>
#include "../types.h"

typedef struct Buffer
{
    unsigned int length;
    unsigned int head;
    unsigned int tail;
    accel_big_t *data;
} Buffer;

void buffer_reset(Buffer *buffer);

bool buffer_is_full(Buffer *buffer);

unsigned int buffer_len(Buffer *buffer);

bool buffer_put(Buffer *buffer, accel_big_t item);

bool buffer_get(Buffer *buffer, accel_big_t *value);

bool buffer_peek(Buffer *buffer, int index, accel_big_t *value);

#endif