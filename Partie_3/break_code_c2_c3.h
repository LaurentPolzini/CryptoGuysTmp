#ifndef _BREAK_CODE_C2_C3_H_
#define _BREAK_CODE_C2_C3_H_

#include "mutex.h"
#include <sys/types.h> // off_t
#include <stdio.h> // FILE
#include <pthread.h> // mutex
#include "break_code_c2.h" // struct_c2
#include "break_code_c3.h" // struct_c3

#define TAILLE_TAB_SCORE 500

void init_params(char *score_out, int *fdScoreOut, char *logFileName, FILE **fileLog);

void destroy_params(int *fdScoreOut, FILE **fileLog);

/*
    creation / destruction / copie de la structure commune
*/
/* 
    structure qui sert pour c1
    c1 va attribuer a chacun de ses threads
    une structure (chaque threads aura ses meilleures clefs)
    cette structure permet d'appliquer c2 comme c3 comme all
    sans toucher a c1
*/
typedef struct stC2_C3 {
    // immuable : la taille max des tableaux de scores
    struct struct_c2 *s_c2;
    struct struct_c3 *s_c3;
} stC2_C3;

stC2_C3 *init_stC2_C3(struct_c2 *sc2, struct_c3 *sc3);
void destruct_stC2_C3(stC2_C3 **s_c2c3);
stC2_C3 *copySC2C3(stC2_C3 *toCopy);

/*
    Gestion des scores (appelle c2 c3)
*/
void ajouteScores(stC2_C3 *st, unsigned char *key);

// puts every credible values from array[indexTabFrom]
// to array[indexTabTo]
void incrusteTab(stC2_C3 *array, stC2_C3 *toWhere);


#endif
