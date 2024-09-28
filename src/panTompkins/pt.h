#ifndef PT_H
#define PT_H

#include "panTompkins.h" // Include panTompkins.h per usare le sue funzioni

// Funzione per inizializzare l'algoritmo Pan-Tompkins
void pantompkins_init(void)
{
    pt_total_steps = 0; // Reimposta il contatore dei passi
}

// Funzione per contare i passi usando l'algoritmo Pan-Tompkins
steps_t pantompkins_totalsteps(time_delta_ms_t delta, accel_t x, accel_t y, accel_t z)
{
    panTompkins(x, y, z); // Richiama la funzione che processa i dati accelerometrici
    return pt_total_steps;
}

#endif
