#ifndef PAN_TOMPKINS
#define PAN_TOMPKINS

#include <stdint.h>  // Aggiunto per int8_t, int16_t
#include <stdbool.h>
#include "../types.h"
// Definizione del tipo dataType
typedef int dataType;

// Definizione di costanti
//#define WINDOWSIZE 4      // Modifica la dimensione della finestra di integrazione
//#define BUFFSIZE 20      // Dimensione del buffer
#define THRESHOLD 8500     // Soglia adattata
#define REFRACTORY_PERIOD 320  // Periodo refrattario in millisecondi

// Definizione delle funzioni
void pantompkins_init();
// Aggiungere le nuove funzioni per il conteggio dei passi

steps_t pantompkins_totalsteps(time_delta_ms_t delta_ms, accel_t accx, accel_t accy, accel_t accz);

#endif

