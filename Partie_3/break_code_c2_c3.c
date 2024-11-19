#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "break_code_c2_c3.h"
#include "break_code_c1.h"
#include "break_code_c3.h"
#include "break_code_c2.h"
#include "../utilitaire/utiL.h"

void afficheTab2(double *tab, int nbElems, unsigned char **keys);

void functorC2_C3(unsigned char *key, void *stC2C3VOID);

pthread_mutex_t mutexScore;
int tailleTabScore = 50;

int break_code_c2_c3(char *file_in, char *dict_file_in, char *score_out, int keyLen, char *logFile) {
    (void) score_out;
    if (pthread_mutex_init(&mutexScore, NULL) != 0) {
        pError(NULL, "Erreur creation mutex score break_code_c2", 2);
    }

    off_t tailleMsg = 0;
    char *cryptedMsg = ouvreEtLitFichier(file_in, &tailleMsg);

    stC2_C3 *stC2C3 = initSTc2_c3(cryptedMsg, tailleMsg, tailleTabScore, dict_file_in);

    appelClefsFinales(file_in, keyLen, (void *) stC2C3, functorC2_C3, logFile);

    afficheTab2(stC2C3 -> tabMeilleurScoreC2, *(stC2C3 -> nbScoreTabs), stC2C3 -> tabKeysScoreC2);
    printf("\n");
    afficheTab3(stC2C3 -> tabMeilleurScoreC3, *(stC2C3 -> nbScoreTabs), stC2C3 -> tabKeysScoreC3);


    destroyStructC2_C3(&stC2C3);

    if (pthread_mutex_destroy(&mutexScore) != 0) {
        pError(NULL, "Erreur liberation mutex", 4);
    }

    return 0;
}


void functorC2_C3(unsigned char *key, void *stC2C3VOID) {
    if (pthread_mutex_lock(&mutexScore) != 0) {
        pError(NULL, "Erreur prise jeton mutex score", 4);
    }

    stC2_C3 *varStructC2C3 = (stC2_C3 *) stC2C3VOID;

    // recoit un message codé, le décrypte avec la clef
    translateMsg(key, (void *) (varStructC2C3 -> msgAndTaille));

    // lui assigne une distance avec C2
    traiteMsgClefC2((varStructC2C3 -> msgAndTaille) -> msgUncrypted, varStructC2C3 -> distance);

    // lui assigne le nombre de mots présents : break_code_c3
    traiteMsgClefC3((varStructC2C3 -> msgAndTaille) -> msgUncrypted, varStructC2C3 -> nbMotsPrez, varStructC2C3 -> dicoHash);

    // l'ajoute aux tableaux (si nécessaire)
    ajouteScores(varStructC2C3, key);

    if (pthread_mutex_unlock(&mutexScore) != 0) {
        pError(NULL, "Erreur don jeton mutex score", 4);
    }
}

stC2_C3 *initSTc2_c3(char *msgCrypted, off_t tailleMsgCrypted, int tailleTab, char *dict_file_in) {
    stC2_C3 *sC2_C3 = malloc(sizeof(stC2_C3));
    pError(sC2_C3, "Erreur allocation memoire", 2);

    (sC2_C3 -> msgAndTaille) = malloc(sizeof(sMsgAndTaille));
    pError(sC2_C3 -> msgAndTaille, "Erreur allocation memoire", 4);
    (sC2_C3 -> msgAndTaille) -> msg = msgCrypted;
    (sC2_C3 -> msgAndTaille) -> lenMsg = tailleMsgCrypted;

    sC2_C3 -> tailleScoreTab = tailleTab;

    sC2_C3 -> tabMeilleurScoreC2 = malloc(sizeof(double) * tailleTab);
    pError(sC2_C3 -> tabMeilleurScoreC2, "Erreur allocation memoire", 4);
    sC2_C3 -> tabKeysScoreC2 = malloc(sizeof(unsigned char *) * tailleTab);
    pError(sC2_C3 -> tabKeysScoreC2, "Erreur allocation memoire", 4);

    sC2_C3 -> tabMeilleurScoreC3 = malloc(sizeof(double) * tailleTab);
    pError(sC2_C3 -> tabMeilleurScoreC3, "Erreur allocation memoire", 4);
    sC2_C3 -> tabKeysScoreC3 = malloc(sizeof(unsigned char *) * tailleTab);
    pError(sC2_C3 -> tabKeysScoreC3, "Erreur allocation memoire", 4);

    sC2_C3 -> nbScoreTabs = malloc(sizeof(int));
    pError(sC2_C3 -> nbScoreTabs, "Erreur allocation memoire", 4);
    *(sC2_C3 -> nbScoreTabs) = 0;

    sC2_C3 -> distance = malloc(sizeof(double));
    pError(sC2_C3 -> distance, "Erreur allocation memoire", 4);
    sC2_C3 -> nbMotsPrez = malloc(sizeof(double));
    pError(sC2_C3 -> nbMotsPrez, "Erreur allocation memoire", 4);

    sC2_C3 -> dicoHash = NULL;
    read_and_insert_words(dict_file_in, &(sC2_C3 -> dicoHash));

    return sC2_C3;
}

void destroyStructC2_C3(stC2_C3 **struc2C3) {
    free(((*struc2C3) -> msgAndTaille) -> msg);
    free(((*struc2C3) -> msgAndTaille) -> msgUncrypted);
    freeTabs((void **) ((*struc2C3) -> tabKeysScoreC2), *((*struc2C3) -> nbScoreTabs));
    freeTabs((void **) ((*struc2C3) -> tabKeysScoreC3), *((*struc2C3) -> nbScoreTabs));
    free((*struc2C3) -> nbScoreTabs);
    free((*struc2C3) -> tabMeilleurScoreC2);
    free((*struc2C3) -> tabMeilleurScoreC3);
    clear_table(&((*struc2C3) -> dicoHash));
    free(*struc2C3);
}

void ajouteScores(stC2_C3 *st, unsigned char *key) {
    ajouteScoreC2(st, key);
    ajouteScoreC3(st, key);
    // s'il n'y a pas deja le nombre maximal d'element
    // en ajoute 1
    if (*(st -> nbScoreTabs) != st -> tailleScoreTab) {
        *(st -> nbScoreTabs) += 1;
    }
}

void freeTabs(void **tabs, int nbElems) {
    for (int i = 0 ; i < nbElems ; ++i) {
        free(tabs[i]);
    }
    free(tabs);
}

void afficheTab2(double *tab, int nbElems, unsigned char **keys) {
    printf("scores c2 : \n");
    for (int i = 0 ; i < nbElems ; ++i) {
        printf("%s : %f\n", keys[i], tab[i]);
    }
}

void afficheTab3(int *tab, int nbElems,  unsigned char **keys) {
    printf("scores c3 : \n");
    for (int i = 0 ; i < nbElems ; ++i) {
        printf("%s : %i\n", keys[i], tab[i]);
    }
}
