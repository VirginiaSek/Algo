#ifndef PANTOMPKINS_H
#define PANTOMPKINS_H
#include "../types.h"
// Definisci il tipo 'dataType' come 'int' perché i tuoi valori sono interi
typedef int dataType;

// Definizioni delle costanti di configurazione
#define WINDOWSIZE 4  // Dimensione della finestra di integrazione
#define BUFFSIZE 600    // Dimensione del buffer per il segnale
#define DELAY 22        // Ritardo introdotto dai filtri

// Dichiarazione dei coefficienti dei filtri
extern const dataType dcblock_coeff;
extern const int lowpass_coeff1;
extern const int lowpass_coeff2;
extern const int lowpass_dcblock_coeff1;
extern const int lowpass_dcblock_coeff2;
extern const int highpass_coeff1;
extern const int highpass_coeff2;

// Dichiarazioni delle variabili globali per il buffer e il segnale
extern dataType signal[BUFFSIZE], dcblock[BUFFSIZE], lowpass[BUFFSIZE], highpass[BUFFSIZE], derivative[BUFFSIZE], squared[BUFFSIZE], integral[BUFFSIZE], outputSignal[BUFFSIZE];

extern int rr1[8], rr2[8], rravg1, rravg2, rrlow, rrhigh, rrmiss;
extern long unsigned int i, j, sample, lastQRS, lastSlope, currentSlope;
extern int current;
extern dataType peak_i, peak_f, threshold_i1, threshold_i2, threshold_f1, threshold_f2, spk_i, spk_f, npk_i, npk_f;
extern int qrs, regular, prevRegular;  // Usando 'int' invece di 'bool'

// Funzioni specifiche dell'algoritmo
void pantompkins_init();
steps_t pantompkins_totalsteps(time_delta_ms_t delta_ms, accel_t accx, accel_t accy, accel_t accz);

#endif  // PANTOMPKINS_H

