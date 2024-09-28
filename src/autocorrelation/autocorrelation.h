#ifndef AUTOCORRELATION
#define AUTOCORRELATION
 
#include "autocorrelation_stepcount.h"
#include <stdlib.h> // per malloc e free
#include "../types.h"
 
steps_t autocorrelation_steps_counter=0;
 
void autocorrelation_stepcount_init()
{
    autocorrelation_steps_counter = 0;
}
 
//buffer circolare
#define BUFFER_SIZE 30000 // Dimensione del buffer circolare (ad esempio, per quaranta min di dati a 12.5 Hz)
 
typedef struct {
    int16_t data[BUFFER_SIZE][3]; // Dati di accelerazione (x, y, z)
    size_t head;                   // Indice di scrittura
    size_t count;                  // Numero di elementi nel buffer
} CircularBuffer;
 
// Inizializza il buffer circolare
void circular_buffer_init(CircularBuffer *buffer) {
    buffer->head = 0;
    buffer->count = 0;
}
 
// Aggiunge un campione al buffer circolare
void circular_buffer_add(CircularBuffer *buffer, accel_t accx, accel_t accy, accel_t accz) {
    buffer->data[buffer->head][0] = accx; // Asse x
    buffer->data[buffer->head][1] = accy; // Asse y
    buffer->data[buffer->head][2] = accz; // Asse z
 
    buffer->head = (buffer->head + 1) % BUFFER_SIZE; // Aggiorna l'indice di scrittura
    if (buffer->count < BUFFER_SIZE) {
        buffer->count++; // Incrementa il conteggio se non si è raggiunta la dimensione massima
    }
}
 
// Funzione wrapper
steps_t autocorrelation_stepcount_totalsteps(time_delta_ms_t delta_ms, accel_t accx, accel_t accy, accel_t accz) {
    static CircularBuffer buffer; // Buffer circolare statico
    circular_buffer_init(&buffer); // Inizializzazione del buffer
 
    // Aggiungi i nuovi dati al buffer circolare
    circular_buffer_add(&buffer, accx, accy, accz);
 
    // Passa i dati a count_steps
    // Creiamo un array temporaneo per passare i dati
    int16_t temp_data[buffer.count * 3];
    
    // Copia i dati dal buffer circolare all'array temporaneo
    for (size_t i = 0; i < buffer.count; i++) {
        size_t index = (buffer.head + BUFFER_SIZE - buffer.count + i) % BUFFER_SIZE; // Calcolo dell'indice corretto
        temp_data[i * 3]     = buffer.data[index][0]; // Asse x
        temp_data[i * 3 + 1] = buffer.data[index][1]; // Asse y
        temp_data[i * 3 + 2] = buffer.data[index][2]; // Asse z
    }
 
    // Chiamata alla funzione count_steps e accumulo dei passi
    autocorrelation_steps_counter += count_steps(temp_data);
 
    return autocorrelation_steps_counter; // Restituisci il conteggio totale dei passi
}
#endif
 
