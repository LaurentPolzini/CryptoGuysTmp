#ifndef __BREAK_CODE_C2_H__
#define __BREAK_CODE_C2_H__

// renvoie la fréquence des lettres d'un message
float *freq(char *msg, int msgLen);

// retourne l'indice d'une lettre (0 -> 26)
// ou -1 si c'est n'est pas une lettre
int indice_lettre(char lettre);

/*
    Renvoie la distance de la fréquence message crypté 
    et de la fréquence théorique de la langue cible

    plus c'est proche de 0 plus on est proche de la fréquence théorique
*/
double distanceFreqs(float *freqLanguage, float *decryptedFreq);

#endif
