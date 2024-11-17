#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <fcntl.h>
#include <ctype.h>
#include <string.h>
#include <pthread.h>
#include "./crackage.h"
#include "../utilitaire/utiL.h"
#include "./break_code_c2.h"
#include "./break_code_c1.h"
#include "../Partie_1/xor.h"

/*
    msgAndTaille : le msg crypté, décrypté, sa taille
    distance la distance de la clef actuelle
    tabMeilleurScore le tableau des meilleurs scores (0 = excellent)
    nbScoreTab le nombre total de score actuel
    tabKeysScore le tableau des clefs à l'indice 
        correspondant à tabMeilleurScore
*/
typedef struct {
    sMsgAndTaille *msgAndTaille;
    double *distance;
    double *tabMeilleurScore;
    int *nbScoreTab;
    unsigned char **tabKeysScore;
} structC2;

void freeTabs(void **tabs, int nbElems);
void freeStructC2(structC2 **sc2);

int getIndexInsertion(structC2 *st);

void ajouteScore(structC2 *st, unsigned char *key);
void functorC2(unsigned char *key, void *s);
void traiteMsgClefC2(char *msg, void *distance);

extern float stat_thFr[26];
extern float stat_thEn[26];

// le nombre de clef qui vont être sauvegardées
// comme étant les meilleurs clefs
int tailleTabMeilleurScore = 50;

// chaque clef accédera a la meme zone mémoire
// le temps de calculer la distance et eventuellement
// la mettre dans le tableau des meilleurs scores
pthread_mutex_t mutexScore;


int break_code_c2(char *file_in, char *dict_file_in, char *score_out, int keyLength, char *logFile) {
    (void) file_in, (void) dict_file_in, (void) score_out, (void) keyLength, (void) logFile;
    
    off_t tailleMsg = 0;
    char *msgCrypted = ouvreEtLitFichier(file_in, &tailleMsg);
    char *msgDecrypte = encrypt_decrypt_xorMSG(msgCrypted, "Clef1", tailleMsg);
    printf("msg : %s\n", msgDecrypte);
    double dist;
    traiteMsgClefC2(msgDecrypte, &dist);

    printf("score du texte décodé : %f\n", dist);

    if (pthread_mutex_init(&mutexScore, NULL) != 0) {
        pError(NULL, "Erreur creation mutex score break_code_c2", 2);
    }

    structC2 *stC2 = malloc(sizeof(structC2));
    pError(stC2, "Erreur allocation memoire", 2);
    (stC2 -> msgAndTaille) = malloc(sizeof(sMsgAndTaille));
    pError((stC2 -> msgAndTaille), "Erreur allocation memoire", 2);
    (stC2 -> msgAndTaille) -> msg = msgCrypted;
    (stC2 -> msgAndTaille) -> lenMsg = tailleMsg;

    stC2 -> tabMeilleurScore = malloc(sizeof(double) * tailleTabMeilleurScore);
    pError(stC2 -> tabMeilleurScore, "Erreur allocation memoire", 2);
    stC2 -> tabKeysScore = malloc(sizeof(unsigned char *) * tailleTabMeilleurScore);
    pError(stC2 -> tabKeysScore, "Erreur allocation memoire", 2);

    stC2 -> distance = malloc(sizeof(double));
    pError(stC2 -> distance, "Erreur allocation memoire", 2);
    stC2 -> nbScoreTab = malloc(sizeof(int));
    pError(stC2 -> nbScoreTab, "Erreur allocation memoire", 2);
    *(stC2 -> nbScoreTab) = 0;

    appelClefsFinales(file_in, keyLength, (void *) stC2, functorC2, logFile);

    if (pthread_mutex_destroy(&mutexScore) != 0) {
        pError(NULL, "Erreur destruction mutex score break_code_c2", 2);
    }

    printf("Voici les 10 meilleurs clefs, avec leur score (0 = bien) : \n");
    for (int i = 0 ; i < 50 ; ++i) {
        printf("key : %s ; score : %f\n", (stC2 -> tabKeysScore)[i], (stC2 -> tabMeilleurScore)[i]);
    }
    freeStructC2(&stC2);

    return 0;
}

/*
    assigne un poid a une clef
    la conserve eventuellement dans le tableau des meilleures clefs
*/
void functorC2(unsigned char *key, void *s) {
    if (pthread_mutex_lock(&mutexScore) != 0) {
        pError(NULL, "Erreur prise jeton mutex score", 2);
    }

    structC2 *ds = (structC2 *) s;

    // recoit un message codé, le décrypte avec la clef
    translateMsg(key, (void *) (ds -> msgAndTaille));
    // lui assigne une distance
    traiteMsgClefC2((ds -> msgAndTaille) -> msgUncrypted, ds -> distance);

    // l'ajoute au tableau (si nécessaire)
    ajouteScore(ds, key);

    if (pthread_mutex_unlock(&mutexScore) != 0) {
        pError(NULL, "Erreur don jeton mutex score", 2);
    }
}

/*
    une structure st contenant 2 tableaux :
        un tableau 1 possédant les meilleurs scores (plus proche de 0 mieux c'est)
        un tableau 2 de clefs à l'indice correspondant au tableau 1
*/
void ajouteScore(structC2 *st, unsigned char *key) {
    int lenKey = strlen((const char *) key);
    unsigned char *keyCopy = malloc(lenKey + 1);
    strcpy((char *) keyCopy, (const char *) key);
    keyCopy[lenKey] = '\0';

    int ind = getIndexInsertion(st);
    
    if (ind < tailleTabMeilleurScore) {
        for (int i = *(st -> nbScoreTab) ; i > ind ; --i) {
            // décaler tout le tableau vers la droite
            // (copier tous les elements pour en inserer un nouveau)
            if (i != tailleTabMeilleurScore) {
                (st -> tabMeilleurScore)[i] = (st -> tabMeilleurScore)[i - 1];
                (st -> tabKeysScore)[i] = (st -> tabKeysScore)[i - 1];
            }
        }
        // insertion du nouvel element
        (st -> tabMeilleurScore)[ind] = *(st -> distance);
        (st -> tabKeysScore)[ind] = keyCopy;

        // s'il n'y a pas deja le nombre maximal d'element
        // en ajoute 1
        if (*(st -> nbScoreTab) != tailleTabMeilleurScore) {
            *(st -> nbScoreTab) += 1;
        }
    }
}

int getIndexInsertion(structC2 *st) {
    int ind = 0;
    while ((ind < *(st -> nbScoreTab)) && ((st -> tabMeilleurScore)[ind] < *(st -> distance))) {
        ++ind;
    }
    return ind;
}

/*
    assigne une distance a une clef 
    (par rapport aux fréquences de lettres d'une langue)
*/
void traiteMsgClefC2(char *msg, void *distance) {
    float *freqMsg = freq(msg, (int) strlen(msg));
    *((double *) distance) = distanceFreqs(stat_thFr, freqMsg);
}

/*
    Renvoit un tableau des fréquences des 26 lettres de l'alphabet
    dans le message msg.
*/
float *freq(char *msg, int msgLen) {
    float *freqs = malloc(sizeof(float) * 26);
    pError(freqs, "Erreur allocation tableau fréquence lettre clef actuelle", 2);
    for (int i = 0 ; i < 26 ; ++i) {
        freqs[i] = 0;
    }

    int nbAlpha = 0;
    char readChar;
    for (int i = 0 ; i < msgLen ; ++i) {
        readChar = msg[i];
        if (isalpha(readChar)) {
            readChar = tolower(readChar);
            ++freqs[indice_lettre(readChar)];
            ++nbAlpha;
        }
    }
    for (int i = 0 ; i < 26 ; ++i) {
        freqs[i] = (freqs[i] / nbAlpha) * 100;
    }

    return freqs;
}

/*
    Retourne l'indice d'une lettre (pour le tableau de fréquence)
*/
int indice_lettre(char lettre) {
    if (lettre >= 'a' && lettre <= 'z') {
        return lettre - 'a';
    }
    return -1; // Si la lettre n'est pas entre 'a' et 'z'
}

/*
    Renvoie la distance de la fréquence message crypté 
    et de la fréquence théorique de la langue cible

    plus c'est proche de 0 plus on est proche de la fréquence théorique
*/
double distanceFreqs(float *freqLanguage, float *decryptedFreq) {
    double distance = 0;

    for (int i = 0 ; i < 26 ; ++i) {
        distance += pow((freqLanguage[i] - decryptedFreq[i]), 2);
    }

    return distance;
}

void freeStructC2(structC2 **sc2) {
    free((*sc2) -> distance);
    free(((*sc2) -> msgAndTaille) -> msg);
    free(((*sc2) -> msgAndTaille) -> msgUncrypted);
    freeTabs((void **) ((*sc2) -> tabKeysScore), *((*sc2) -> nbScoreTab));
    free((*sc2) -> nbScoreTab);
    free((*sc2) -> tabMeilleurScore);
}

void freeTabs(void **tabs, int nbElems) {
    for (int i = 0 ; i < nbElems ; ++i) {
        free(tabs[i]);
    }
    free(tabs);
}
