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
#include "break_code_c2_c3.h"

/*
    msgAndTaille : le msg crypté, décrypté, sa taille
    distance la distance de la clef actuelle
    tabMeilleurScore le tableau des meilleurs scores (0 = excellent)
    nbScoreTab le nombre total de score actuel
    tabKeysScore le tableau des clefs à l'indice 
        correspondant à tabMeilleurScore
*/

void freeTabs(void **tabs, int nbElems);

// chaque clef accédera a la meme zone mémoire
// le temps de calculer la distance et eventuellement
// la mettre dans le tableau des meilleurs scores
//pthread_mutex_t mutexScore;

/*sqzwi
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

    stC2_C3 *stC2C3 = initSTc2_c3(msgCrypted, tailleMsg, tailleTabMeilleurScore);

    appelClefsFinales(file_in, keyLength, (void *) stC2C3, functorC2, logFile);

    if (pthread_mutex_destroy(&mutexScore) != 0) {
        pError(NULL, "Erreur destruction mutex score break_code_c2", 2);
    }

    printf("Voici les 10 meilleurs clefs, avec leur score (0 = bien) : \n");
    for (int i = 0 ; i < 50 ; ++i) {
        printf("key : %s ; score : %f\n", (stC2C3 -> tabKeysScore)[i], (stC2C3 -> tabMeilleurScore)[i]);
    }
    destroyStructC2_C3(&stC2C3);

    return 0;
}
*/
/*
    assigne une distance a une clef 
    (par rapport aux fréquences de lettres d'une langue)
*/
double traiteMsgClefC2(char *msg, double *distance, float stats[26]) {
    float *freqMsg = freq(msg, (int) strlen(msg));
    *distance = distanceFreqs(stats, freqMsg);

    return *distance;
}

void ajouteScoreC2(stC2_C3 *st, unsigned char *key, int ind) {
    if (ind < st -> tailleScoreTab) {
        for (int i = *(st -> nbScoreTabs) ; i > ind ; --i) {
            // décaler tout le tableau vers la droite
            // (copier tous les elements pour en inserer un nouveau)
            if (i != st -> tailleScoreTab) { // indice inexistant
                (st -> tabMeilleurScoreC2)[i] = (st -> tabMeilleurScoreC2)[i - 1];
                strcpy((char *) (st -> tabKeysScoreC2)[i], (const char *) (st -> tabKeysScoreC2)[i - 1]);
            }
        }
        // insertion du nouvel element
        (st -> tabMeilleurScoreC2)[ind] = *(st -> distance);
        strcpy((char *) (st -> tabKeysScoreC2)[ind], (const char *) key);
    }
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

int getIndexInsertionC2(stC2_C3 *st) {
    return getIndexInsertionValueC2(st, *(st -> distance));
}

int getIndexInsertionValueC2(stC2_C3 *st, double value) {
    int ind = 0;
    while ((ind < *(st -> nbScoreTabs)) && ((st -> tabMeilleurScoreC2)[ind] < value)) {
        ++ind;
    }

    return ind;
}
