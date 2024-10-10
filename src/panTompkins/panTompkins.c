#include "panTompkins.h"
#include <math.h> // Per sqrt

#define FS 12.5         // sampling frequency 12.5 Hz
#define WINDOWSIZE 4    // Finestra di integrazione adattata per 12.5 Hz
#define BUFFSIZE 600    // Dimensione buffer
#define NOSAMPLE -32000 // Indicatore di fine campioni

// Soglie adattive per rilevamento dei picchi (passi)
accel_big_t threshold_i1 = 15000;
accel_big_t threshold_i2 = 10000;
accel_big_t threshold_f1 = 12000;
accel_big_t threshold_f2 = 8000;

// Variabili globali per i filtri e i buffer
accel_big_t signal[BUFFSIZE], dcblock[BUFFSIZE], lowpass[BUFFSIZE], highpass[BUFFSIZE], derivative[BUFFSIZE], squared[BUFFSIZE], integral[BUFFSIZE];
int rr1[8], rr2[8], rravg1 = 0, rravg2 = 0;
int rrlow = 0, rrhigh = 0, rrmiss = 0;
long unsigned int sample = 0, lastQRS = 0, lastSlope = 0, currentSlope = 0;
int current = 0;
accel_big_t peak_i = 0, peak_f = 0, spk_i = 0, spk_f = 0, npk_i = 0, npk_f = 0;
bool qrs = false, regular = true, prevRegular;
int step_counter = 0; // Contatore per i passi

void pantompkins_init()
{
    // Inizializzazione delle variabili e buffer
    for (int i = 0; i < BUFFSIZE; i++)
    {
        signal[i] = 0;
        dcblock[i] = 0;
        lowpass[i] = 0;
        highpass[i] = 0;
        derivative[i] = 0;
        squared[i] = 0;
        integral[i] = 0;
    }
    for (int i = 0; i < 8; i++)
    {
        rr1[i] = 0;
        rr2[i] = 0;
    }
    rravg1 = 0;
    rravg2 = 0;
    step_counter = 0; // Reset del contatore passi
    lastQRS = 0;
    lastSlope = 0;
    currentSlope = 0;
    sample = 0;
}

steps_t pantompkins_totalsteps(time_delta_ms_t delta_ms, accel_t accx, accel_t accy, accel_t accz)
{
    // Calcola la magnitudine dell'accelerazione
    accel_big_t magnitude = sqrt(accx * accx + accy * accy + accz * accz);

    // Shift dei buffer
    if (sample >= BUFFSIZE)
    {
        for (int i = 0; i < BUFFSIZE - 1; i++)
        {
            signal[i] = signal[i + 1];
            dcblock[i] = dcblock[i + 1];
            lowpass[i] = lowpass[i + 1];
            highpass[i] = highpass[i + 1];
            derivative[i] = derivative[i + 1];
            squared[i] = squared[i + 1];
            integral[i] = integral[i + 1];
        }
        current = BUFFSIZE - 1;
    }
    else
    {
        current = sample;
    }

    // Aggiungi la magnitudine al buffer del segnale
    signal[current] = magnitude;

    // Filtro DC Block
    if (current >= 1)
    {
        dcblock[current] = signal[current] - signal[current - 1] + 0.995 * dcblock[current - 1];
    }
    else
    {
        dcblock[current] = 0;
    }

    // Filtro passa-basso
    lowpass[current] = dcblock[current];
    if (current >= 1)
        lowpass[current] += 2 * lowpass[current - 1];
    if (current >= 2)
        lowpass[current] -= lowpass[current - 2];
    if (current >= 6)
        lowpass[current] -= 2 * dcblock[current - 6];
    if (current >= 12)
        lowpass[current] += dcblock[current - 12];

    // Filtro passa-alto
    highpass[current] = -lowpass[current];
    if (current >= 1)
        highpass[current] -= highpass[current - 1];
    if (current >= 16)
        highpass[current] += 32 * lowpass[current - 16];
    if (current >= 32)
        highpass[current] += lowpass[current - 32];

    // Derivata
    derivative[current] = highpass[current];
    if (current > 0)
        derivative[current] -= highpass[current - 1];

    // Quadratura del segnale
    squared[current] = derivative[current] * derivative[current];

    // Integrazione finestrata
    integral[current] = 0;
    for (int i = 0; i < WINDOWSIZE; i++)
    {
        if (current >= i)
            integral[current] += squared[current - i];
        else
            break;
    }
    integral[current] /= WINDOWSIZE;

    // Se sia l'integrale che il segnale sono sopra le soglie, è probabilmente un picco.
    if ((integral[current] >= threshold_i1) && (highpass[current] >= threshold_f1))
    {
        // C'è una latenza di 200ms. Se il nuovo picco rispetta questa condizione, possiamo continuare i test.
        if (sample > lastQRS + FS / 5)
        {
            // Rispetta la latenza di 200ms, ma non quella di 360ms, quindi controlliamo la pendenza.
            if (sample <= lastQRS + (long unsigned int)(0.36 * FS))
            {
                // La derivata quadrata è "a forma di M", quindi dobbiamo controllare campioni vicini per assicurarci di essere al valore massimo.
                currentSlope = 0;
                for (int j = current - 10; j <= current; j++)
                {
                    if (squared[j] > currentSlope)
                    {
                        currentSlope = squared[j];
                    }
                }

                if (currentSlope <= (accel_big_t)(lastSlope / 2))
                {
                    qrs = false;
                }
                else
                {
                    spk_i = 0.125 * peak_i + 0.875 * spk_i;
                    threshold_i1 = npk_i + 0.25 * (spk_i - npk_i);
                    threshold_i2 = 0.5 * threshold_i1;

                    spk_f = 0.125 * peak_f + 0.875 * spk_f;
                    threshold_f1 = npk_f + 0.25 * (spk_f - npk_f);
                    threshold_f2 = 0.5 * threshold_f1;

                    lastSlope = currentSlope;
                    qrs = true;
                }
            }
            else
            {
                currentSlope = 0;
                for (int j = current - 10; j <= current; j++)
                {
                    if (squared[j] > currentSlope)
                    {
                        currentSlope = squared[j];
                    }
                }

                spk_i = 0.125 * peak_i + 0.875 * spk_i;
                threshold_i1 = npk_i + 0.25 * (spk_i - npk_i);
                threshold_i2 = 0.5 * threshold_i1;

                spk_f = 0.125 * peak_f + 0.875 * spk_f;
                threshold_f1 = npk_f + 0.25 * (spk_f - npk_f);
                threshold_f2 = 0.5 * threshold_f1;

                lastSlope = currentSlope;
                qrs = true;
            }
        }
        else
        {
            // Se il nuovo picco non rispetta la latenza di 200ms, è rumore. Aggiorna le soglie e vai al campione successivo.
            peak_i = integral[current];
            npk_i = 0.125 * peak_i + 0.875 * npk_i;
            threshold_i1 = npk_i + 0.25 * (spk_i - npk_i);
            threshold_i2 = 0.5 * threshold_i1;

            peak_f = highpass[current];
            npk_f = 0.125 * peak_f + 0.875 * npk_f;
            threshold_f1 = npk_f + 0.25 * (spk_f - npk_f);
            threshold_f2 = 0.5 * threshold_f1;

            qrs = false;
        }
    }

    // Se è stato rilevato un picco, aggiorna gli intervalli RR.
    if (qrs)
    {
        rravg1 = 0;
        for (int i = 0; i < 7; i++)
        {
            rr1[i] = rr1[i + 1];
            rravg1 += rr1[i];
        }
        rr1[7] = sample - lastQRS;
        lastQRS = sample;
        rravg1 += rr1[7];
        rravg1 *= 0.125;

        // Se l'intervallo RR scoperto è normale, aggiornalo.
        if ((rr1[7] >= rrlow) && (rr1[7] <= rrhigh))
        {
            rravg2 = 0;
            for (int i = 0; i < 7; i++)
            {
                rr2[i] = rr2[i + 1];
                rravg2 += rr2[i];
            }
            rr2[7] = rr1[7];
            rravg2 += rr2[7];
            rravg2 *= 0.125;
            rrlow = 0.92 * rravg2;
            rrhigh = 1.16 * rravg2;
            rrmiss = 1.66 * rravg2;
        }

        prevRegular = regular;
        if (rravg1 == rravg2)
        {
            regular = true;
        }
        else
        {
            regular = false;
            if (prevRegular)
            {
                threshold_i1 /= 2;
                threshold_f1 /= 2;
            }
        }

        // Incrementa il contatore dei passi quando un picco viene rilevato.
        step_counter++;
    }

    // Se non è stato rilevato un picco per troppo tempo, utilizza soglie più leggere e fai una ricerca retroattiva.
    if (!qrs && (sample - lastQRS > (long unsigned int)rrmiss) && (sample > lastQRS + FS / 5))
    {
        for (int i = current - (sample - lastQRS) + FS / 5; i < (long unsigned int)current; i++)
        {
            if ((integral[i] > threshold_i2) && (highpass[i] > threshold_f2))
            {
                currentSlope = 0;
                for (int j = i - 10; j <= i; j++)
                {
                    if (squared[j] > currentSlope)
                    {
                        currentSlope = squared[j];
                    }
                }

                if ((currentSlope < (accel_big_t)(lastSlope / 2)) && (i + sample) < lastQRS + 0.36 * lastQRS)
                {
                    qrs = false;
                }
                else
                {
                    peak_i = integral[i];
                    peak_f = highpass[i];
                    spk_i = 0.25 * peak_i + 0.75 * spk_i;
                    spk_f = 0.25 * peak_f + 0.75 * spk_f;

                    threshold_i1 = npk_i + 0.25 * (spk_i - npk_i);
                    threshold_i2 = 0.5 * threshold_i1;
                    lastSlope = currentSlope;

                    threshold_f1 = npk_f + 0.25 * (spk_f - npk_f);
                    threshold_f2 = 0.5 * threshold_f1;

                    rravg1 = 0;
                    for (int j = 0; j < 7; j++)
                    {
                        rr1[j] = rr1[j + 1];
                        rravg1 += rr1[j];
                    }
                    rr1[7] = sample - (current - i) - lastQRS;
                    lastQRS = sample - (current - i);
                    rravg1 += rr1[7];
                    rravg1 *= 0.125;

                    if ((rr1[7] >= rrlow) && (rr1[7] <= rrhigh))
                    {
                        rravg2 = 0;
                        for (int i = 0; i < 7; i++)
                        {
                            rr2[i] = rr2[i + 1];
                            rravg2 += rr2[i];
                        }
                        rr2[7] = rr1[7];
                        rravg2 += rr2[7];
                        rravg2 *= 0.125;
                        rrlow = 0.92 * rravg2;
                        rrhigh = 1.16 * rravg2;
                        rrmiss = 1.66 * rravg2;
                    }

                    prevRegular = regular;
                    if (rravg1 == rravg2)
                    {
                        regular = true;
                    }
                    else
                    {
                        regular = false;
                        if (prevRegular)
                        {
                            threshold_i1 /= 2;
                            threshold_f1 /= 2;
                        }
                    }

                    break;
                }
            }
        }
    }

    sample++;            // Incrementa il contatore dei campioni
    return step_counter; // Restituisce il conteggio aggiornato dei passi
}
