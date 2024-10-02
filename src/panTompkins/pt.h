#ifndef PT_H
#define PT_H

#include "../types.h"  // Assicurati che il tipo `steps_t`, `time_delta_ms_t`, e `accel_t` siano definiti qui.

// Dichiarazione della funzione di inizializzazione dell'algoritmo Pan-Tompkins
void pantompkins_init();

// Dichiarazione della funzione per processare i dati accelerometrici e restituire il numero di passi
steps_t pantompkins_totalsteps(time_delta_ms_t delta_ms, accel_t accx, accel_t accy, accel_t accz);

#endif  // PT_H

