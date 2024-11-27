#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <fcntl.h>
#include <unistd.h>
#include "break_code_c2_c3.h"
#include "break_code_c1.h"
#include "break_code_c3.h"
#include "break_code_c2.h"
#include "../Partie_1/xor.h"
#include "../utilitaire/utiL.h"

extern float stat_thFr[26];
extern float stat_thEn[26];


pthread_mutex_t mutexEcritureFichier;

void afficheTab2(double *tab, int nbElems, unsigned char **keys);

void functorC2_C3(unsigned char *key, void *stC2C3VOID);

int tailleTabScore = 50;

int break_code_c2_c3(char *file_in, char *dict_file_in, char *score_out, int keyLen, char *logFile) {
    if (pthread_mutex_init(&mutexEcritureFichier, NULL) != 0) {
        pError(NULL, "Erreur creation mutex", 4);
    }
    int fdScoreOut = -1;
    if (score_out) {
        fdScoreOut = open(score_out, O_CREAT | O_WRONLY | O_TRUNC, 0644);
        if (fdScoreOut == -1) {
            printf("%s : ", score_out);
            fflush(stdout);
            pError(NULL, "Error opening file", 4);
        }
    } 

    off_t tailleMsg = 0;
    char *cryptedMsg = ouvreEtLitFichier(file_in, &tailleMsg);

    dictionnary *dicoHash = NULL;
    read_and_insert_words(dict_file_in, &dicoHash, NULL);

    stC2_C3 *stC2C3 = NULL;
    // je cherche un mot typiquement francais
    // si pas trouvé : anglais
    if (find_word(dicoHash, "abaisser")) {
        stC2C3 = initSTc2_c3(cryptedMsg, tailleMsg, tailleTabScore, dicoHash, keyLen, fdScoreOut, stat_thFr);
    } else {
        stC2C3 = initSTc2_c3(cryptedMsg, tailleMsg, tailleTabScore, dicoHash, keyLen, fdScoreOut, stat_thEn);
    }

    appelClefsFinales(file_in, keyLen, (void *) stC2C3, functorC2_C3, logFile, true);

    char *msgUncrypted = encrypt_decrypt_xorMSG(cryptedMsg, (char *) (stC2C3 -> tabKeysScoreC3)[0], tailleMsg);
    printf("\nMessage décrypté avec la meilleure clef (\"%s\" : score %d) : \n\n%s\n", (stC2C3 -> tabKeysScoreC3)[0], (stC2C3 -> tabMeilleurScoreC3)[0], msgUncrypted);

    destroyStructC2_C3(&stC2C3);
    clear_table(&dicoHash);
    free(cryptedMsg);

    if (pthread_mutex_destroy(&mutexEcritureFichier) != 0) {
        pError(NULL, "Erreur destruction mutex", 4);
    }

    return 0;
}


void functorC2_C3(unsigned char *key, void *stC2C3VOID) {
    stC2_C3 *varStructC2C3 = (stC2_C3 *) stC2C3VOID;

    // recoit un message codé, le décrypte avec la clef
    translateMsg(key, (void *) (varStructC2C3 -> msgAndTaille));

    // lui assigne une distance avec C2
    traiteMsgClefC2((varStructC2C3 -> msgAndTaille) -> msgUncrypted, varStructC2C3 -> distance, varStructC2C3 -> stats);
    
    // lui assigne le nombre de mots présents : break_code_c3
    int motsPrez = traiteMsgClefC3((varStructC2C3 -> msgAndTaille) -> msgUncrypted, varStructC2C3 -> nbMotsPrez, varStructC2C3 -> dicoHash);

    // l'ajoute aux tableaux (si nécessaire)
    ajouteScores(varStructC2C3, key);

    ecritClefScore(varStructC2C3 -> fdLogFile, key, motsPrez);

    free((void *) ((varStructC2C3 -> msgAndTaille) -> msgUncrypted));
}

stC2_C3 *initSTc2_c3(char *msgCrypted, off_t tailleMsgCrypted, int tailleTab, dictionnary *dico, int keyLen, int fdlogFile, float stat[26]) {
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

    sC2_C3 -> fdLogFile = fdlogFile;

    sC2_C3 -> stats = stat;

    return sC2_C3;
}

void destroyStructC2_C3(stC2_C3 **struc2C3) {
    if (*struc2C3) {
        free(((*struc2C3) -> msgAndTaille) -> msg);
        free((void *) ((*struc2C3) -> msgAndTaille));
        freeTabs((void **) ((*struc2C3) -> tabKeysScoreC2), *((*struc2C3) -> nbScoreTabs));
        freeTabs((void **) ((*struc2C3) -> tabKeysScoreC3), *((*struc2C3) -> nbScoreTabs));
        free((*struc2C3) -> nbScoreTabs);
        free((*struc2C3) -> tabMeilleurScoreC2);
        free((*struc2C3) -> tabMeilleurScoreC3);
        free(*struc2C3);

        close((*struc2C3) -> fdLogFile);
    }
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
    return initSTc2_c3((toCopy -> msgAndTaille) -> msg, (toCopy -> msgAndTaille) -> lenMsg, 
        toCopy -> tailleScoreTab, toCopy -> dicoHash, toCopy -> lenKey, 
        toCopy -> fdLogFile, toCopy -> stats);
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


/*
    Ecrit dans un fichier (généralement un log file) 
    une chaine de caracteres de cette forme
    "clef : 1234  nombre de mots présents dans le dictionnaire : 100"
*/
void ecritClefScore(int fdFile, unsigned char *key, int tabPoid) {
    if (fdFile != -1) {
        if (pthread_mutex_lock(&mutexEcritureFichier) != 0) {
            pError(NULL, "Erreur prise token mutex ecriture file", 4);
        }

        int lenKey = strlen((const char *) key);
        // clef : 1234  nombre de mots présents dans le dictionnaire : 100
        char *textToWrite = malloc(sizeof(char) * (2 + lenKey + strlen("clef : nombre de mots présents dans le dictionnaire : 100")));
        pError(textToWrite, "Erreur allocation memoire", 4);

        snprintf(textToWrite, sizeof(textToWrite), "clef : %s\tnombre de mots présents dans le dictionnaire : %d\n", key, tabPoid);
        if (write(fdFile, textToWrite, strlen(textToWrite)) == -1) {
            close(fdFile);
            pError(NULL, "Error writing to file", 4);
        }

        free(textToWrite);

        if (pthread_mutex_unlock(&mutexEcritureFichier) != 0) {
            pError(NULL, "Erreur don token mutex ecriture file", 4);
        }
    }
}
