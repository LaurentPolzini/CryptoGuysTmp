#ifndef __BREAK_CODE_C2_H__
#define __BREAK_CODE_C2_H__

#include "break_code_c2_c3.h"

// renvoie la fréquence des lettres d'un message
float *freq(char *msg, int msgLen);

// retourne l'indice d'une lettre (0 -> 26)
// ou -1 si c'est n'est pas une lettre
int indice_lettre(char lettre);

void ajouteScoreC2(stC2_C3 *st, unsigned char *key);
void traiteMsgClefC2(char *msg, double *distance);

/*
    Renvoie la distance de la fréquence message crypté 
    et de la fréquence théorique de la langue cible

    plus c'est proche de 0 plus on est proche de la fréquence théorique
*/
double distanceFreqs(float *freqLanguage, float *decryptedFreq);

#endif
