#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <pthread.h>
#include "../break_code_c1.h"
#include "../../utilitaire/utiL.h"
#include "../caracteresCandidatsIndexKey.h"

typedef struct {
    int *tableauIndCour;
    int *tableauIndMax;
} sTableauIndCourantEtMax;

pthread_mutex_t *mutexEcritureClefsFichier;

void *clefsThread(void *arg);
sTableauIndCourantEtMax initialiseTableauxIndiceThreadLIGNE0(int tailleClef, unsigned char **carCand, 
    int indThread, int nbThreads, int tailleSegment);

sTableauIndCourantEtMax initialiseTableauxIndice(int tailleClef, unsigned char **carCand);
sTableauIndCourantEtMax prochaineClefSelonTableau(sTableauIndCourantEtMax tabs, int tailleClef);
void noThreadClefFinaleIndTAB(char *msgCode, unsigned long tailleMsgCode, int len_key, unsigned long *nbClefs, char *nameFileOut);


typedef struct {
    sTableauIndCourantEtMax tabs;
    unsigned char **carCandParIndice;
    int len_key;
    void *userData;
    FunctorC1 f;
} threadInfoClefVolee;

// Threads juste sur la premiere ligne
threadInfoClefVolee creeInfoThread(sTableauIndCourantEtMax tabs, 
    unsigned char **carCandParIndice, FunctorC1 f, int len_key, void *userData) {
    threadInfoClefVolee ti;
    ti.tabs = tabs;
    ti.carCandParIndice = carCandParIndice;
    ti.f = f;
    ti.len_key = len_key;
    ti.userData = userData;

    return ti;
}

/*
    Retourne le prod carthesien des caracteresCandidatsParIndice
        — clef [0] ∈ [′5′,′ 6′,′ 8′]
        — clef [1] ∈ [′1′]
        — clef [2] ∈ [′0′,′ 2′]

        renvoie clef ∈ 
        [”510”, 
        ”512”, 
        ”610”, 
        ”612”, 
        ”810”, 
        ”812”]
*/
void clefsFinalesL0(char *msgCode, off_t tailleMsgCode, int len_key, unsigned long *nbClefs, char *nameFileOut) {
    mutexEcritureClefsFichier = malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init(mutexEcritureClefsFichier, NULL);

    unsigned char **carCandParIndice = caracteresCandidatsParIndice(msgCode, tailleMsgCode, len_key);
    *nbClefs = nbClefsTotal(carCandParIndice, len_key);
    printf("nb clefs : %lu\n", *nbClefs);

    //int tailleSegment = 1;
    //int tailleSegment = ceil((double) strlen((const char *) carCandParIndice[0]) / 8);
    int tailleSegment = ceil((double) sqrt(strlen((const char *) carCandParIndice[0])));
    int nbSegment = strlen((const char *) carCandParIndice[0]) / tailleSegment;
    /*
    int fileOutDescriptor = open(nameFileOut, O_WRONLY | O_CREAT, 0644);
    if (fileOutDescriptor == -1) {
        printf("%s : ", nameFileOut);
        fflush(stdout);
        pError(NULL, "Erreur ouverture fichier", 1);
    }
    */

    FunctorC1 f = doNothing;
    void *userData = (void *) &nameFileOut;
    sTableauIndCourantEtMax tabs[nbSegment];
    pthread_t thid[nbSegment];
    threadInfoClefVolee infoThreads[nbSegment];
    for (int i = 0 ; i < nbSegment ; ++i) {
        tabs[i] = initialiseTableauxIndiceThreadLIGNE0(len_key, carCandParIndice, i, nbSegment, tailleSegment);
        infoThreads[i] = creeInfoThread(tabs[i], carCandParIndice, f, len_key, userData);
        pthread_create(&thid[i], NULL, clefsThread, &(infoThreads[i]));
    }
    for (int i = 0 ; i < nbSegment ; ++i) {
        pthread_join(thid[i], NULL);
    }

    pthread_mutex_destroy(mutexEcritureClefsFichier);
    return;
}


void *clefsThread(void *arg) {
    threadInfoClefVolee ti = *((threadInfoClefVolee *) arg);
    unsigned char *curKey;

    while ((ti.tabs).tableauIndCour != NULL) {
        curKey = getKeyFromTab((ti.tabs).tableauIndCour, ti.carCandParIndice, ti.len_key);
        functorOnKey(curKey, ti.f, ti.userData);
        free(curKey);

        ti.tabs = prochaineClefSelonTableau(ti.tabs, ti.len_key);
    }

    pthread_exit(NULL);
}

/*
    chaque thread s'occupera d'un segment de la premiere ligne de carCandidats
    exemple:
    soit le tableau de caracteres candidats
    [1234,
    56,
    908]
    et la taille d'un segment sqrt()
    thread 1 s'occupe de 12 de la premiere ligne et du reste
    thread 2 s'occupe de 34 et du reste (de toutes les lignes en bas)

    la division des taches se fait selon la premiere ligne
    probleme : premiere ligne petite mais reste hyper grand bah y'aura pas beaucoup
    de threads donc ca prendra une eternité
*/
sTableauIndCourantEtMax initialiseTableauxIndiceThreadLIGNE0(int tailleClef, unsigned char **carCand, int indThread, int nbThreads, int tailleSegment) {
    sTableauIndCourantEtMax tabs;

    tabs.tableauIndCour = malloc(tailleClef * sizeof(int));
    pError(tabs.tableauIndCour, "Erreur creation tableau d'indice", 1);
    tabs.tableauIndMax = malloc(tailleClef * sizeof(int));
    pError(tabs.tableauIndMax, "Erreur creation tableau d'indice max", 1);

    for (int i = 1 ; i < tailleClef ; ++i) {
        tabs.tableauIndCour[i] = 0;
        tabs.tableauIndMax[i] = strlen((const char *) carCand[i]);
    }
    tabs.tableauIndCour[0] = indThread * tailleSegment;
    if (indThread == nbThreads - 1) {
        tabs.tableauIndMax[0] = strlen((const char *) carCand[0]);
    } else {
        tabs.tableauIndMax[0] = (indThread + 1) * tailleSegment;
    }
    
    return tabs;
}


/*
    [1,2]
    [3]
    [4,5]

    [ 
      0
      0
      0
    ]
    renvoit 
    [ 
      0
      0
      1
    ]
    renvoit
    [ 
      1
      0 (pas 1 puisque length([3]) == 1)
      0
    ]
    renvoit
    [ 
      1
      0
      1
    ]
    renvoit NULL
*/

sTableauIndCourantEtMax prochaineClefSelonTableau(sTableauIndCourantEtMax tabs, int tailleClef) {
    int ind = tailleClef - 1;
    int *tableau = tabs.tableauIndCour;
    int *nbCaractereCandidatParIndice = tabs.tableauIndMax;

    if (tableau[ind] < nbCaractereCandidatParIndice[ind]) {
        tableau[ind] += 1;
    }
    while ((tableau[ind] == nbCaractereCandidatParIndice[ind]) && (ind > 0)) {
        tableau[ind - 1] += 1;
        tableau[ind] = 0;
        --ind;
    }
    
    if (ind == 0 && tableau[0] == nbCaractereCandidatParIndice[0]) {
        tabs.tableauIndCour = NULL;
    }
    
    return tabs;
}

/*
    initialise le tableau d'indice courant et le tableau d'indice a ne pas depasser

    pour faire avec des threads il faudrait implanter une taille de segment
*/
sTableauIndCourantEtMax initialiseTableauxIndice(int tailleClef, unsigned char **carCand) {
    sTableauIndCourantEtMax tabs;

    tabs.tableauIndCour = malloc(tailleClef * sizeof(int));
    pError(tabs.tableauIndCour, "Erreur creation tableau d'indice", 1);
    tabs.tableauIndMax = malloc(tailleClef * sizeof(int));
    pError(tabs.tableauIndMax, "Erreur creation tableau d'indice max", 1);

    for (int i = 0 ; i < tailleClef ; ++i) {
        tabs.tableauIndCour[i] = 0;
        tabs.tableauIndMax[i] = strlen((const char *) carCand[i]);
    }

    return tabs;
}

void noThreadClefFinaleIndTAB(char *msgCode, unsigned long tailleMsgCode, int len_key, unsigned long *nbClefs, char *nameFileOut) {
    unsigned char **carCandParIndice = caracteresCandidatsParIndice(msgCode, tailleMsgCode, len_key);
    sTableauIndCourantEtMax tabs = initialiseTableauxIndice(len_key, carCandParIndice);

    unsigned char *curKey;
    FunctorC1 f = clefTrouve;
    void *userData = (void *)nameFileOut;

    while (tabs.tableauIndCour != NULL) {
        curKey = getKeyFromTab(tabs.tableauIndCour, carCandParIndice, len_key);
        functorOnKey(curKey, f, userData);
        free(curKey);

        tabs = prochaineClefSelonTableau(tabs, len_key);
    }
    
    *nbClefs = nbClefsTotal(carCandParIndice, len_key);
    return;
}

