#ifndef PANTOMPKINS_H
#define PANTOMPKINS_H

typedef float dataType; // Definisci dataType come float se non è definito altrove

// Funzione principale per il rilevamento dei passi che accetta i valori accelerometrici
void panTompkins(dataType accx, dataType accy, dataType accz);

// Variabili globali
extern int pt_total_steps;    // Contatore globale dei passi
extern dataType threshold_i1; // Soglia per il rilevamento dei picchi

#endif // PANTOMPKINS_H
