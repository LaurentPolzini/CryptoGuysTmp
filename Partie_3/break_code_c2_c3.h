#ifndef _BREAK_CODE_C2_C3_H_
#define _BREAK_CODE_C2_C3_H_

#include <unistd.h>
#include <stdbool.h>
#include "break_code_c1.h"

struct dictionnary;

typedef struct stC2_C3 {
    // le message crypté et sa taille
    struct sMsgAndTaille *msgAndTaille;
    // immuable : la taille max des tableaux de scores
    int tailleScoreTab;

    // nombre de meilleurs scores présents
    // dans tous les tableaux
    int *nbScoreTabs;

    // tableaux pour c2 : ses meilleurs scores / clefs
    double *tabMeilleurScoreC2;
    unsigned char **tabKeysScoreC2;
    int lenKey;

    // tableaux pour c3 : ses meilleurs scores / clefs
    int *tabMeilleurScoreC3;
    unsigned char **tabKeysScoreC3;
    
    float *stats;
    struct dictionnary *dicoHash;

    // le poid de c2
    double *distance;
    // le poid de c3
    int *nbMotsPrez;

    int fdLogFile;
} stC2_C3;

int break_code_c2_c3(char *file_in, char *dict_file_in, char *score_out, int keyLen, char *logFileName);

// ecrit les clefs et leurs scores dans file (peut etre stdout)
void ecritTab2(double *tab, int nbElems, unsigned char **keys, FILE *file);
void ecritTab3(int *tab, int nbElems,  unsigned char **keys, FILE *file);

stC2_C3 *initSTc2_c3(char *msgCrypted, off_t tailleMsgCrypted, int tailleTab, struct dictionnary *dico, int keyLen, int fdlogFileP, float stat[26]);

void ajouteScores(stC2_C3 *st, unsigned char *key);

void destroyStructC2_C3(stC2_C3 **struc2C3);

void freeTabs(void **tabs, int nbElems);

stC2_C3 *copySC2C3(stC2_C3 *toCopy);

// puts every credible values from array[indexTabFrom]
// to array[indexTabTo]
void incrusteTab(stC2_C3 **array, stC2_C3 *toWhere, int indexTabFrom);

void ecritClefScore(int fdFile, unsigned char *key, int tabPoid);

#endif
