#include "panTompkins.h"
#include <math.h>  // Per sqrt
#include "../types.h"
//#define WINDOWSIZE 4  // Adeguata alla frequenza di campionamento di 12.5 Hz
//const int THRESHOLD = 8500;  // Soglia per rilevare un "passo" sul segnale integrato
//const int REFRACTORY_PERIOD = 320;  // Periodo refrattario in millisecondi
long unsigned int lastStepTime = 0;

// Definizione dei coefficienti dei filtri
const dataType dcblock_coeff = 8192;  // Moltiplicato per evitare l'uso di float
const int lowpass_coeff1 = 2;
const int lowpass_coeff2 = -2;
const int lowpass_dcblock_coeff1 = -2;
const int lowpass_dcblock_coeff2 = 1;
const int highpass_coeff1 = 32;
const int highpass_coeff2 = 1;

// Buffer e variabili globali
dataType signal[BUFFSIZE], dcblock[BUFFSIZE], lowpass[BUFFSIZE], highpass[BUFFSIZE], derivative[BUFFSIZE], squared[BUFFSIZE], integral[BUFFSIZE];
int current = 0;
long unsigned int sample = 0;
int step_count = 0;  // Contatore globale per i passi

void pantompkins_init() {
    // Inizializza tutti i buffer e le variabili globali
    for (int i = 0; i < BUFFSIZE; i++) {
        signal[i] = 0;
        dcblock[i] = 0;
        lowpass[i] = 0;
        highpass[i] = 0;
        derivative[i] = 0;
        squared[i] = 0;
        integral[i] = 0;
    }
    current = 0;
    sample = 0;
    step_count = 0;
    lastStepTime = 0;
}

steps_t pantompkins_totalsteps(time_delta_ms_t delta_ms, accel_t accx, accel_t accy, accel_t accz) {
    // Calcola la magnitude a partire dalle componenti accelerometriche
    dataType magnitude = (int) sqrt(accx * accx + accy * accy + accz * accz);

    // Shift dei buffer per mantenere la struttura Pan-Tompkins originale
    if (sample >= BUFFSIZE) {
        for (int i = 0; i < BUFFSIZE - 1; i++) {
            signal[i] = signal[i+1];
            dcblock[i] = dcblock[i+1];
            lowpass[i] = lowpass[i+1];
            highpass[i] = highpass[i+1];
            derivative[i] = derivative[i+1];
            squared[i] = squared[i+1];
            integral[i] = integral[i+1];
        }
        current = BUFFSIZE - 1;
    } else {
        current = sample;
    }

    // Aggiungi la magnitude al buffer del segnale
    signal[current] = magnitude;

    // Filtraggio - DC Block
    if (current >= 1)
        dcblock[current] = signal[current] - signal[current - 1] + (dcblock_coeff * dcblock[current - 1]) / 1000;
    else
        dcblock[current] = 0;

    /* Filtro passa-basso
    lowpass[current] = dcblock[current];
    if (current >= 1)
        lowpass[current] += lowpass_coeff1 * lowpass[current - 1];
    if (current >= 2)
        lowpass[current] -= lowpass_coeff2 * lowpass[current - 2];
    if (current >= 6)
        lowpass[current] -= lowpass_dcblock_coeff1 * dcblock[current - 6];
    if (current >= 12)
        lowpass[current] += lowpass_dcblock_coeff2 * dcblock[current - 12];
*/
    // Filtro passa-alto
    highpass[current] = -lowpass[current];
    if (current >= 1)
        highpass[current] -= highpass[current - 1];
    if (current >= 16)
        highpass[current] += highpass_coeff1 * lowpass[current - 16];
    if (current >= 32)
        highpass[current] += highpass_coeff2 * lowpass[current - 32];

    // Derivata
    derivative[current] = highpass[current];
    if (current > 0)
        derivative[current] -= highpass[current - 1];

    // Quadratura del segnale
    squared[current] = derivative[current] * derivative[current];

    // Integrazione finestrata
    integral[current] = 0;
    for (int i = 0; i < WINDOWSIZE; i++) {
        if (current >= i)
            integral[current] += squared[current - i];
        else
            break;
    }
    integral[current] /= WINDOWSIZE;

    // Rilevamento di un "passo" con soglia e periodo refrattario
    long unsigned int currentTime = sample * delta_ms;  // Approssima il tempo corrente in millisecondi
    if (integral[current] > THRESHOLD && (currentTime - lastStepTime) > REFRACTORY_PERIOD) {
        step_count++;
        lastStepTime = currentTime;  // Aggiorna l'ultimo passo rilevato
    }

    sample++;  // Incrementa il contatore dei campioni

    return step_count;  // Restituisce il numero totale di passi rilevati
}
