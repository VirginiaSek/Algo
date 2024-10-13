
#include <stdio.h>
#include "buffer.h"

void buffer_reset(Buffer *buffer)
{
    for (int i = 0; i < buffer->length; i++)
    {
        buffer->data[i] = 0;
    }
    buffer->head = 0;
    buffer->tail = 0;
}

bool buffer_is_full(Buffer *buffer)
{
    return ((buffer->head + 1) % buffer->length == buffer->tail);
}

unsigned int buffer_len(Buffer *buffer)
{
    if (buffer->head == buffer->tail)
        return 0;
    else if (buffer->head > buffer->tail)
        return buffer->head - buffer->tail;
    else
        return (buffer->length - buffer->tail) + buffer->head;
}

bool buffer_put(Buffer *buffer, accel_big_t item)
{
    int next;

    next = buffer->head + 1; // next is where head will point to after this write.
    if (next >= buffer->length)
        next = 0;

    if (next == buffer->tail) // if the head + 1 == tail, circular buffer is full
    {
        printf("ERR on put: buffer is full");
        return false;
    }

    buffer->data[buffer->head] = item; // Load data and then move
    buffer->head = next;               // head to next data offset.
    return true;                       // return success to indicate successful push.
}

bool buffer_get(Buffer *buffer, accel_big_t *value)
{
    int next;

    if (buffer->head == buffer->tail) // if the head == tail, we don't have any data
    {
        printf("ERR on get: buffer empty");
        return false;
    }

    next = buffer->tail + 1; // next is where tail will point to after this read.
    if (next >= buffer->length)
        next = 0;

    *value = buffer->data[buffer->tail]; // Read data and then move
    buffer->tail = next;                 // tail to next offset.
    return true;                         // return success to indicate successful push.
}

bool buffer_peek(Buffer *buffer, int index, accel_big_t *value)
{
    if (buffer->head == buffer->tail) // if the head == tail, we don't have any data
    {
        printf("ERR on get: buffer empty");
        return 0;
    }

    int readIndex = (buffer->tail + index) % buffer->length;

    *value = buffer->data[readIndex]; // Read data
    return 1;                         // return success to indicate successful push.
}