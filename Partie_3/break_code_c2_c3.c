#include <stdio.h>
#include <pthread.h>
#include <fcntl.h>
#include "break_code_c2_c3.h"
#include "break_code_c3.h" // struct_c3
#include "break_code_c2.h" // struct_c2
#include "../Partie_1/chiffrement.h" // pour encrypt_decrypt_xorMSG
#include "../utilitaire/utiL.h" // pError principalement

void init_params(char *score_out, int *fdScoreOut, char *logFileName, FILE **fileLog) {
    *fdScoreOut = -1;
    if (score_out) {
        printf("Score out est fourni, le traitement des clefs prendra plus de temps !\n");
        *fdScoreOut = open(score_out, O_CREAT | O_TRUNC | O_WRONLY, 0644);
        if (*fdScoreOut == -1) {
            pError(NULL, "Erreur ouverture fichier score clefs", 4);
        }
    }
    *fileLog = NULL;
    if (logFileName) {
        *fileLog = fopen(logFileName, "a");
        pError(fileLog, "Erreur ouverture fichier log c3", 4);
    }
}

void destroy_params(int *fdScoreOut, FILE **fileLog) {
    if (fdScoreOut) {
        close(*fdScoreOut);
    }
    if (*fileLog) {
        fclose(*fileLog);
    }
    if (pthread_mutex_destroy(&MUTEX_ECRITURE_SCORE) != 0) {
        pError(NULL, "Erreur destruction mutex", 4);
    }
}

stC2_C3 *init_stC2_C3(struct_c2 *sc2, struct_c3 *sc3) {
    stC2_C3 *sC2C3 = malloc(sizeof(stC2_C3));
    pError(sC2C3, "Erreur allocation mémoire", 4);
    sC2C3 -> s_c2 = sc2;
    sC2C3 -> s_c3 = sc3;
    
    return sC2C3;
}

void destruct_stC2_C3(stC2_C3 **s_c2c3) {
    if ((*s_c2c3) -> s_c2) {
        destruct_struct_c2(&((*s_c2c3) -> s_c2));
    }
    if ((*s_c2c3) -> s_c3) {
        destruct_struct_c3(&((*s_c2c3) -> s_c3));
    }

    free((void *) *s_c2c3);
}

stC2_C3 *copySC2C3(stC2_C3 *toCopy) {
    return init_stC2_C3(copy_s_c2(toCopy -> s_c2), copy_s_c3(toCopy -> s_c3));
}

void ajouteScores(stC2_C3 *st, unsigned char *key) {
    ajouteScoreC2(st -> s_c2, key, getIndexInsertionC2_struc(st -> s_c2));
    ajouteScoreC3(st -> s_c3, key, getIndexInsertionC3_struc(st -> s_c3));
}

// compiles les meilleurs scores de array dans toWhere
void incrusteTab(stC2_C3 *array, stC2_C3 *toWhere) {
    if (array -> s_c2) {
        compile_structs_c2(toWhere -> s_c2, array -> s_c2);
    }
    if (array -> s_c3) {
        compile_structs_c3(toWhere -> s_c3, array -> s_c3);
    }
}

