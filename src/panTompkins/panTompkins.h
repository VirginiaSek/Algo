#ifndef PAN_TOMPKINS
#define PAN_TOMPKINS

#include <stdint.h>  // Aggiunto per int8_t, int16_t
#include <stdbool.h>
#include "../types.h"
// Definizione del tipo dataType
typedef int dataType;

// Definizione di costanti
#define WINDOWSIZE 4      // Modifica la dimensione della finestra di integrazione
#define BUFFSIZE 600       // Dimensione del buffer
#define THRESHOLD 1200     // Soglia adattata
#define REFRACTORY_PERIOD 320  // Periodo refrattario in millisecondi

// Definizione delle funzioni
void panTompkins();
void init(char file_in[], char file_out[]);

// Aggiungere le nuove funzioni per il conteggio dei passi
void pantompkins_init();
steps_t pantompkins_totalsteps(time_delta_ms_t delta_ms, accel_t accx, accel_t accy, accel_t accz);

#endif

