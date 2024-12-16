#ifndef __THREADSEGMENTATIONTABLEAUXINDEX_H__
#define __THREADSEGMENTATIONTABLEAUXINDEX_H__

#include "Pile.h"

// pour chaque indice de clef un nombre de segments
// et de taille de segments
typedef struct {
    int *nbSegment;
    int *tailleSegment;

    int nbElem;
} nbEtTailleSegment;

typedef struct s_sPileIndCourFin sPileIndCourFin;

struct s_sPileIndCourFin {
    Pile *pileOrigin;
    Pile *pileIndCour;
    Pile *pileIndMax;
};

// free the allocated memory of nbSegment and tailleSegment
void freeNBetTailleSeg(nbEtTailleSegment *s);

void freeSPiles(sPileIndCourFin ***piles, int nbPiles);
void freeSPile(sPileIndCourFin **pile);

/*
    On va maintenant mettre des threads sur les premieres lignes

    [123] (segmenté en [12] [3])
    [45] (segmenté en [4] [5])
    [6] pas segmenté

    (2 * 2 * 1 = ) 4 segments : 4 threads
    tableaux en termes d'indice : 
    [0 -> 2]    [0 -> 2]     [2 -> 3]     [2 -> 3]
    [0 -> 1]    [1 -> 2]     [0 -> 1]     [1 -> 2]
    [0 -> 1]    [0 -> 1]     [0 -> 1]     [0 -> 1]

    voici les tableaux d'indice dont s'occupera chacun des threads
*/
sPileIndCourFin **initialisePilesIndiceThreads(int tailleClef, unsigned char **carCand, long *nbThreadsMax, long *nbThreadsReel);

/* renvoit null si la fin
 sinon renvoie le tableau suivant
 [0 -> 2]
 [0 -> 1]
 [0 -> 1]
    revoit 
    [1 -> 2]
    [0 -> 1]
    [0 -> 1]
renvoit null
*/
sPileIndCourFin *prochaineClefSelonPile(sPileIndCourFin *pile, int tailleClef);

unsigned char *clefActuelle(sPileIndCourFin *pile, unsigned char **carCandParIndice, int tailleClef);

/*
    Divise en segment les caracteres candidats d'indice 0 à segmenteUntilI
    met dans nbThreads le nombre de segments crées, donc le nombre de tableau nbEtTailleSegment
    utilisés par les threads
*/
nbEtTailleSegment setNbAndTailleSegment(int tailleClef, unsigned char **carCand, long *nbThreadsMax, long *nbThreadsReel, int *nbLigne_non_traitee);

void ajouteSegmentation(sPileIndCourFin **piles, int *nbPileCur, int nbSeg, int tailleSeg, unsigned char *carCand);

void initialisePilesSuivantes(sPileIndCourFin **piles, int nbPileCur, int nbSeg);

void initialisePilesSuivantes(sPileIndCourFin **piles, int nbPileCur, int nbSeg);

void afficheNbEtTailleSeg(nbEtTailleSegment sg, int len_key);

void affichePiles(sPileIndCourFin **piles, int len_key, int nbPiles);

int *stackIndexToArray(sPileIndCourFin *piles, int len_key);

#endif
