#ifndef _BREAK_CODE_C1_H_
#define _BREAK_CODE_C1_H_

#include <stdbool.h>
#include <unistd.h>
#include "break_code_c2_c3.h"

struct stC2_C3;

typedef struct sMsgAndTaille {
    char *msg;
    off_t lenMsg;
    char *msgUncrypted;
} sMsgAndTaille;

typedef void(*FunctorC1)(unsigned char *, void *);

// retourne si le caractere charDechiffre est valide ou non en ASCII
// Les caractères valides sont alphanumérique minuscule/majuscule
// et les caractères spéciaux de ponctuation
bool estCaractereValideASCII(unsigned char charDechiffre);

char *ouvreEtLitFichier(char *file_in, off_t *sizeMessage);

/*
    recoit un message codé du style "s!/ik" dans msgCode
    et une longueur de clef maximale (donc length(clef) <= len_ken)
    retourne quelque chose du type 
    [[512], [612], [312]]
*/
void clefsFinales(char *msgCode, off_t tailleMsgCode, int len_key, unsigned long *nbClefs, char *nameFileOut);

void appelClefsFinales(char *file_in, int keyLength, void *userData, FunctorC1 functor, char *logFile, bool c2c3);

void freeDoubleArray(unsigned char ***arr, unsigned long len);

/*
    Functors
*/
void afficheClef(unsigned char *key, void *userData);
void clefTrouve(unsigned char *curKey, void *actualKey);
void ecritClef(unsigned char *clef, void *fileOutDescriptor);
void doNothing(unsigned char *none, void *userData);
void translateMsg(unsigned char *key, void *msg);


void associeMaxTabs(struct stC2_C3 **array, struct stC2_C3 *toWhere, int nbThreads);


#endif
