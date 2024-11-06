#ifndef __BREAK_CODE_C1_H__
#define __BREAK_CODE_C1_H__

#include <stdbool.h>

// retourne si le caractere charDechiffre est valide ou non en ASCII
// Les caractères valides sont alphanumérique minuscule/majuscule
// et les caractères spéciaux de ponctuation
bool estCaractereValideASCII(unsigned char* charDechiffre);

/*
    recoit un message codé du style "s!/ik" dans msgCode
    et une longueur de clef maximale (donc length(clef) <= len_ken)
    retourne quelque chose du type 
    [[512], [612], [312]]
*/
unsigned char **clefsFinales(char *msgCode, int len_key, unsigned long *nbClefs);

void freeDoubleArray(unsigned char ***arr, unsigned long len);


#endif
