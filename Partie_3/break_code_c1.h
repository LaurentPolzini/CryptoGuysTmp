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

unsigned long nbClefsTotal(unsigned char **carCandParIndice, int len_key);

/*
    recoit un message codé du style "s!/ik" dans msgCode
    et une longueur de clef maximale (donc length(clef) <= len_ken)
    retourne quelque chose du type 
    [[512], [612], [312]]
*/

void appelClefsFinales(char *file_in, int keyLength, void *userData, FunctorC1 functor, char *logFile, bool c2c3, unsigned long *nbOfKeys);

void appelCaracteresCandidats(char *file_in, int keyLength);

/*
    Functors
*/
void functorOnKey(unsigned char *key, FunctorC1 f, void *userData);
// functors possible sur une clef
void afficheClef(unsigned char *key, void *userData);
void clefTrouve(unsigned char *curKey, void *actualKey);
void ecritClef(unsigned char *clef, void *fileOutDescriptor);
void doNothing(unsigned char *none, void *userData);
void translateMsg(unsigned char *key, void *msg);


/*
    a partir d'un tableau de caracteres candidats
    [1,2]
    [3]
    [4,5]
    et d'un tableau d'indice, [1, 0, 1] sort une clef : 235
*/
unsigned char *getKeyFromTab(int *tableauInd, unsigned char **carCandidats, int len_key);


void associeMaxTabs(struct stC2_C3 **array, struct stC2_C3 *toWhere, int nbThreads);


#endif
