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

int tailleTabScore = 50;

int break_code_c2_c3(char *file_in, char *dict_file_in, char *score_out, int keyLen, char *logFile) {
    (void) score_out, (void) keyLen, (void) logFile;

    off_t tailleMsg = 0;
    char *cryptedMsg = ouvreEtLitFichier(file_in, &tailleMsg);

    dictionnary *dicoHash = NULL;
    read_and_insert_words(dict_file_in, &dicoHash);

    stC2_C3 *stC2C3 = initSTc2_c3(cryptedMsg, tailleMsg, tailleTabScore, dicoHash, keyLen);

    appelClefsFinales(file_in, keyLen, (void *) stC2C3, functorC2_C3, logFile, true);

    afficheTab2(stC2C3 -> tabMeilleurScoreC2, *(stC2C3 -> nbScoreTabs), stC2C3 -> tabKeysScoreC2);
    afficheTab3(stC2C3 -> tabMeilleurScoreC3, *(stC2C3 -> nbScoreTabs), stC2C3 -> tabKeysScoreC3);


    destroyStructC2_C3(&stC2C3);
    clear_table(&dicoHash);
    free(cryptedMsg);

    return 0;
}


void functorC2_C3(unsigned char *key, void *stC2C3VOID) {
    stC2_C3 *varStructC2C3 = (stC2_C3 *) stC2C3VOID;

    // recoit un message codé, le décrypte avec la clef
    translateMsg(key, (void *) (varStructC2C3 -> msgAndTaille));

    // lui assigne une distance avec C2
    traiteMsgClefC2((varStructC2C3 -> msgAndTaille) -> msgUncrypted, varStructC2C3 -> distance);

    // lui assigne le nombre de mots présents : break_code_c3
    traiteMsgClefC3((varStructC2C3 -> msgAndTaille) -> msgUncrypted, varStructC2C3 -> nbMotsPrez, varStructC2C3 -> dicoHash);

    // l'ajoute aux tableaux (si nécessaire)
    ajouteScores(varStructC2C3, key);

    free((void *) ((varStructC2C3 -> msgAndTaille) -> msgUncrypted));
}

stC2_C3 *initSTc2_c3(char *msgCrypted, off_t tailleMsgCrypted, int tailleTab, dictionnary *dico, int keyLen) {
    stC2_C3 *sC2_C3 = malloc(sizeof(stC2_C3));
    pError(sC2_C3, "Erreur allocation memoire", 4);

    (sC2_C3 -> msgAndTaille) = malloc(sizeof(sMsgAndTaille));
    pError(sC2_C3 -> msgAndTaille, "Erreur allocation memoire", 4);
    (sC2_C3 -> msgAndTaille) -> msg = malloc(tailleMsgCrypted);
    pError((sC2_C3 -> msgAndTaille) -> msg, "Erreur allocation memoire", 4);
    memcpy((sC2_C3 -> msgAndTaille) -> msg, msgCrypted, tailleMsgCrypted);
    
    (sC2_C3 -> msgAndTaille) -> lenMsg = tailleMsgCrypted;

    sC2_C3 -> tailleScoreTab = tailleTab;

    sC2_C3 -> tabMeilleurScoreC2 = malloc(sizeof(double) * tailleTab);
    pError(sC2_C3 -> tabMeilleurScoreC2, "Erreur allocation memoire", 4);
    sC2_C3 -> tabKeysScoreC2 = malloc(sizeof(unsigned char *) * tailleTab);
    pError(sC2_C3 -> tabKeysScoreC2, "Erreur allocation memoire", 4);
    for (int i = 0 ; i < tailleTab ; ++i) {
        (sC2_C3 -> tabKeysScoreC2)[i] = malloc(sizeof(unsigned char) * (keyLen + 1));
        pError((sC2_C3 -> tabKeysScoreC2)[i], "Erreur allocation memoire", 4);
    }

    sC2_C3 -> tabMeilleurScoreC3 = malloc(sizeof(double) * tailleTab);
    pError(sC2_C3 -> tabMeilleurScoreC3, "Erreur allocation memoire", 4);
    sC2_C3 -> tabKeysScoreC3 = malloc(sizeof(unsigned char *) * tailleTab);
    pError(sC2_C3 -> tabKeysScoreC3, "Erreur allocation memoire", 4);
    for (int i = 0 ; i < tailleTab ; ++i) {
        (sC2_C3 -> tabKeysScoreC3)[i] = malloc(sizeof(unsigned char) * (keyLen + 1));
        pError((sC2_C3 -> tabKeysScoreC3)[i], "Erreur allocation memoire", 4);
    }

    sC2_C3 -> nbScoreTabs = malloc(sizeof(int));
    pError(sC2_C3 -> nbScoreTabs, "Erreur allocation memoire", 4);
    *(sC2_C3 -> nbScoreTabs) = 0;

    sC2_C3 -> distance = malloc(sizeof(double));
    pError(sC2_C3 -> distance, "Erreur allocation memoire", 4);
    sC2_C3 -> nbMotsPrez = malloc(sizeof(double));
    pError(sC2_C3 -> nbMotsPrez, "Erreur allocation memoire", 4);

    sC2_C3 -> dicoHash = dico;

    sC2_C3 -> lenKey = keyLen;

    return sC2_C3;
}

void destroyStructC2_C3(stC2_C3 **struc2C3) {
    free(((*struc2C3) -> msgAndTaille) -> msg);
    free((void *) ((*struc2C3) -> msgAndTaille));
    freeTabs((void **) ((*struc2C3) -> tabKeysScoreC2), *((*struc2C3) -> nbScoreTabs));
    freeTabs((void **) ((*struc2C3) -> tabKeysScoreC3), *((*struc2C3) -> nbScoreTabs));
    free((*struc2C3) -> nbScoreTabs);
    free((*struc2C3) -> tabMeilleurScoreC2);
    free((*struc2C3) -> tabMeilleurScoreC3);
    free(*struc2C3);
}

void ajouteScores(stC2_C3 *st, unsigned char *key) {
    ajouteScoreC2(st, key, getIndexInsertionC2(st));
    ajouteScoreC3(st, key, getIndexInsertionC3(st));
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
    printf("\nscores c2 : \n");
    for (int i = 0 ; i < nbElems ; ++i) {
        printf("%s : %f  ", keys[i], tab[i]);
        fflush(stdout);
    }
    printf("\n");
}

void afficheTab3(int *tab, int nbElems,  unsigned char **keys) {
    printf("\nscores c3 : \n");
    for (int i = 0 ; i < nbElems ; ++i) {
        printf("%s : %i  ", keys[i], tab[i]);
        fflush(stdout);
    }
    printf("\n");
}

stC2_C3 *copySC2C3(stC2_C3 *toCopy) {
    return initSTc2_c3((toCopy -> msgAndTaille) -> msg, (toCopy -> msgAndTaille) -> lenMsg, toCopy -> tailleScoreTab, toCopy -> dicoHash, toCopy -> lenKey);
}

void incrusteTab(stC2_C3 **array, stC2_C3 *toWhere, int indexTabFrom) {
    int ind;
    // tries to put every values from array[indexTabFrom] to toWhere
    for (int i = 0 ; i < (array[indexTabFrom] -> tailleScoreTab) ; ++i) {
        // initiates the values to insert
        *(toWhere -> distance) = (array[indexTabFrom] -> tabMeilleurScoreC2)[i];
        *(toWhere -> nbMotsPrez) = (array[indexTabFrom] -> tabMeilleurScoreC3)[i];

        // finds where to insert the values from array[indexTabFrom]
        ind = getIndexInsertionValueC2(toWhere, (array[indexTabFrom] -> tabMeilleurScoreC2)[i]);

        // adds all the values contained in array[indexTabFrom] to array[indexTabFrom]
        ajouteScoreC2(toWhere, (array[indexTabFrom] -> tabKeysScoreC2)[i], ind);
    
        // same but for c3
        ind = getIndexInsertionValueC3(toWhere, (array[indexTabFrom] -> tabMeilleurScoreC3)[i]);
        ajouteScoreC3(toWhere, (array[indexTabFrom] -> tabKeysScoreC3)[i], ind);

        if (*(toWhere -> nbScoreTabs) != toWhere -> tailleScoreTab) {
            *(toWhere -> nbScoreTabs) += 1;
        }
    }
}
