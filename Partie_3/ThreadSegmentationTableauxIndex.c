#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "ThreadSegmentationTableauxIndex.h"
#include "../utilitaire/utiL.h"
#include "Pile.h"

/*
    on a les caracteres candidats
    1234
    12
    5678

    donne
    [2] [2] 2 segments de taille 2
    [2] [1] 2 segments de taille 1
    [1] [4] 1 segments de taille 4


    produit du nombre de segments : 2 * 2 * 1 = 4
    4 threads qui s'occuperont de :
    [0 -> 2] [0 -> 2] [2 -> 4] [2 -> 4]
    [0 -> 1] [1 -> 2] [0 -> 1] [1 -> 2]
    [0 -> 4] [0 -> 4] [0 -> 4] [0 -> 4]

    le nombre de threads diminue en fonction de segmenteUntilI (1 : jusque ligne 0 ; 5 : jusque ligne 4)
*/
nbEtTailleSegment setNbAndTailleSegment(int tailleClef, unsigned char **carCand, int *nbThreadsMax, int *nbThreadsReel) {
    nbEtTailleSegment infoSeg;
    infoSeg.nbSegment = malloc(sizeof(int) * tailleClef);
    pError(infoSeg.nbSegment, "Erreur création tableau du nombre de segments des caractères candidats", 1);
    infoSeg.tailleSegment = malloc(sizeof(int) * tailleClef);
    pError(infoSeg.tailleSegment, "Erreur création tableau de la taille des segments des caractères candidats", 1);
    int indSeg = 0;

    while (*nbThreadsReel < *nbThreadsMax && indSeg < tailleClef) {
        /*
        int segments = *nbThreadsMax / (1 << indSeg);  // Ajuste le nombre de segments selon les threads disponibles
        infoSeg.nbSegment[indSeg] = (segments > 1) ? segments : 2; // amélioration chatgpt
        */
        infoSeg.nbSegment[indSeg] = 2;
        infoSeg.tailleSegment[indSeg] = strlen((const char *) carCand[indSeg]) / infoSeg.nbSegment[indSeg];
        ++indSeg;
        *nbThreadsReel *= 2;
    }
    for (int i = indSeg ; i < tailleClef ; ++i) {
        infoSeg.nbSegment[i] = 1;
        infoSeg.tailleSegment[i] = strlen((const char *) carCand[i]);
    }
    /*
    *nbThreads = 1;

    for (int i = 0 ; i < segmenteUntilI ; ++i) {
        infoSeg.nbSegment[i] = ceil((double) sqrt(strlen((const char *) carCand[i])));
        infoSeg.tailleSegment[i] = strlen((const char *) carCand[i]) / infoSeg.nbSegment[i];
        
        *nbThreads = *nbThreads * infoSeg.nbSegment[i];
    }
    for (int i = segmenteUntilI ; i < tailleClef ; ++i) {
        infoSeg.nbSegment[i] = 1;
        infoSeg.tailleSegment[i] = strlen((const char *) carCand[i]);
    }
    */
   
    return infoSeg;
}

void freeNBetTailleSeg(nbEtTailleSegment *s) {
    free(s->nbSegment);
    free(s->tailleSegment);
}

void freeSPiles(sPileIndCourFin **sPiles, int nbPiles) {
    for (int i = 0 ; i < nbPiles ; ++i) {
        freeSPile(sPiles[i]);
    }
    free((void *) *sPiles);
}

void freeSPile(sPileIndCourFin *sPiles) {
    pileDelete(sPiles -> pileOrigin);
    pileDelete(sPiles -> pileIndCour);
    pileDelete(sPiles -> pileIndMax);
}


/*
    On va maintenant mettre des threads sur les premieres lignes

    [123] (segmenté en [12] [3])
    [45] (segmenté en [4] [5])
    [6] pas segmenté

    (2 * 2 * 1 = ) 4 segments : 4 threads
    tableaux en termes d'indice : 
    [0 -> 2]    [0 -> 2]    [2 -> 3]    [2 -> 3]
    [0 -> 1]    [1 -> 2]    [0 -> 1]    [1 -> 2]
    [0 -> 1]    [0 -> 1]    [0 -> 1]    [0 -> 1]

    voici les tableaux d'indice dont s'occupera chacun des threads
*/

sPileIndCourFin **initialisePilesIndiceThreads(int tailleClef, unsigned char **carCand, int *nbThreadsMax, int *nbThreadsReel) {
    nbEtTailleSegment nts = setNbAndTailleSegment(tailleClef, carCand, nbThreadsMax, nbThreadsReel);
    // autant de tableaux que de threads
    sPileIndCourFin **piles = malloc(sizeof(sPileIndCourFin) * (*nbThreadsReel));
    pError(piles, "Erreur allocation tableau d'indice pour les threads", 1);
    for (int i = 0 ; i < *nbThreadsReel ; ++i) {
        piles[i] = malloc(sizeof(sPileIndCourFin));
        pError(piles[i], "Erreur allocation memoire piles threads", 1);
    }
    piles[0] -> pileOrigin = pileCreate(tailleClef);
    piles[0] -> pileIndCour = pileCreate(tailleClef);
    piles[0] -> pileIndMax = pileCreate(tailleClef);
    
    int nbPileActuel = 1; // censé finir a nbThreads
    for (int i = tailleClef - 1 ; i >= 0 ; --i) {
        ajouteSegmentation(piles, &nbPileActuel, nts.nbSegment[i], nts.tailleSegment[i], carCand[i]);
    }
    
    freeNBetTailleSeg(&nts);

    return piles;
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
sPileIndCourFin *prochaineClefSelonPile(sPileIndCourFin *piles, int tailleClef) {
    int ind = 0;
    Pile *pileRef = piles -> pileOrigin;
    Pile *pileCur = piles -> pileIndCour;
    Pile *pileMax = piles -> pileIndMax;
    int valueOrigin;
    int valueCur = *((int *) pileValueAt(pileCur, ind + 1));
    int valueMax = *((int *) pileValueAt(pileMax, ind + 1));

    if (valueCur < valueMax) {
        valueCur += 1;
        modifyStackValueINT(pileCur, ind, valueCur);
    }
    // on ne modifie la premiere ligne que grace a la deuxieme ligne
    while ((ind < tailleClef - 1) && (valueCur == valueMax)) {
        // augmente ligne du dessus de 1 (on a fait toutes les clefs
        // avec le caractere courant)
        valueCur = *((int *) pileValueAt(pileCur, ind + 2)) + 1;
        modifyStackValueINT(pileCur, ind + 1, valueCur);
        
        // on remet la ligne courante a 0
        valueOrigin = *((int *) pileValueAt(pileRef, ind + 1));
        modifyStackValueINT(pileCur, ind, valueOrigin);

        ++ind;
        valueCur = *((int *) pileValueAt(pileCur, ind + 1));
        valueMax = *((int *) pileValueAt(pileMax, ind + 1));
    }

    if (ind == tailleClef - 1) {
        valueCur = *((int *) pileValueAt(pileCur, tailleClef)); // premiere valeur
        valueMax = *((int *) pileValueAt(pileMax, tailleClef));
        if (valueCur == valueMax) {
            return NULL;
        }
    }
    
    return piles;
}

unsigned char *clefActuelle(sPileIndCourFin *piles, unsigned char **carCandParIndice, int tailleClef) {
    int *tab = stackIndexToArray(piles, tailleClef);

    unsigned char *key = malloc(sizeof(char) * (tailleClef + 1));
    for (int i = 0 ; i < tailleClef ; ++i) {
        key[i] = carCandParIndice[i][tab[i]];
    }
    key[tailleClef] = '\0';
    free(tab);
    return key;
}


/*
    on a la pile 
    [0 -> 2]
    [0 -> 1]
    et on veut ajouter [4,5,6] donc 2 segments : [45] [6]
    on aura les piles
    [0 -> 2]    [2 -> 3]
    [0 -> 2]    [0 -> 2]
    [0 -> 1]    [0 -> 1]

    besoin de carCand si je suis le dernier segment je ne vais pas a i + 1 * taille segment
    je vais direct a strlen(carCand) : je ne veux ni dépasser ni m'arreter avant
*/
void ajouteSegmentation(sPileIndCourFin **piles, int *nbPileCur, int nbSeg, int tailleSeg, unsigned char *carCand) {
    int indTabs;
    int *origin;
    int *debut;
    int *fin;
    int lenCarCand = strlen((const char *) carCand);

    initialisePilesSuivantes(piles, *nbPileCur, nbSeg);
    for (int i = 0 ; i < nbSeg ; ++i) {
        for (int j = 0 ; j < *nbPileCur ; ++j) {
            indTabs = (i * (*nbPileCur)) + j;
            debut = malloc(sizeof(int));
            pError(debut, "Erreur allocation element pile", 2);
            fin = malloc(sizeof(int));
            pError(fin, "Erreur allocation element pile", 2);
            origin = malloc(sizeof(int));
            pError(origin, "Erreur allocation element pile", 2);
            
            *debut = i * tailleSeg;
            *origin = *debut;
            *fin = (i == nbSeg - 1) ? lenCarCand : ((i + 1) * tailleSeg);
            
            pilePush(piles[indTabs] -> pileOrigin, (void *) origin);
            pilePush(piles[indTabs] -> pileIndCour, (void *) debut);
            pilePush(piles[indTabs] -> pileIndMax, (void *) fin);
        }
    }

    *nbPileCur = *nbPileCur * nbSeg;
    return;
}


/*
    on a la pile 
    [0 -> 2]
    [0 -> 1]
    et on veut ajouter [4,5,6] donc 2 segments : [45] [6]
    on aura les piles
    [0 -> 2]     [0 -> 2]
    [0 -> 1]     [0 -> 1]
    (pour l'intéret complet de cette fonction voir fonction ajouteSegmentation)
*/
void initialisePilesSuivantes(sPileIndCourFin **piles, int nbPileCur, int nbSeg) {
    int ind;
    for (int i = 1 ; i < nbSeg ; ++i) {
        for (int j = 0 ; j < nbPileCur ; ++j) {
            ind = (i * nbPileCur) + j;
            piles[ind] -> pileOrigin = pileCopyINT(piles[j] -> pileOrigin);
            piles[ind] -> pileIndCour = pileCopyINT(piles[j] -> pileIndCour);
            piles[ind] -> pileIndMax = pileCopyINT(piles[j] -> pileIndMax);
        }
    }
}

int *stackIndexToArray(sPileIndCourFin *piles, int len_key) {
    int *tab = malloc(sizeof(int) * len_key);
    pError(tab, "Erreur allocation piles en int *", 1);

    for (int i = len_key ; i > 0 ; --i) {
        tab[len_key - i] = *((int *) pileValueAt(piles->pileIndCour, i));
    }

    return tab;
}

void afficheNbEtTailleSeg(nbEtTailleSegment sg, int len_key) {
    for (int i = 0 ; i < len_key ; ++i) {
        printf("ligne %d : %d segments de taille %d\n", i, (sg.nbSegment)[i], (sg.tailleSegment)[i]);
    }
}

void affichePiles(sPileIndCourFin **piles, int len_key, int nbPiles) {
    int valueCour;
    int valueMax;

    for (int i = 0 ; i < nbPiles ; ++i) {
        printf("Pile %d : \n", i);
        for (int j = len_key ; j > 0 ; --j) {
            valueCour = *((int *) pileValueAt(piles[i] -> pileIndCour, j));
            valueMax = *((int *) pileValueAt(piles[i] -> pileIndMax, j));
            printf("[%d -> %d]\n", valueCour, valueMax);
        }
    }
    return;
}

