#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "./Pile.h"
#include "../utilitaire/utiL.h"

// nbElem le nombre d'element actuellement dans la pile
// nbElemMax la capacité maximale de la pile
// pile la file de caractères
struct _Pile {
    unsigned int *nbElem;
    unsigned int nbElemMax;
    void **value;
};

// tailleMax la capacité de la pile
Pile *pileCreate(int tailleMax) {
    Pile *p = malloc(sizeof(Pile));
    pError((void *) p, "Erreur création pile", 3);

    p -> nbElem = malloc(sizeof(unsigned int));
    pError(p -> nbElem, "Erreur allocation mémoire", 3);
    *(p -> nbElem) = 0;
    
    p -> value = malloc(sizeof(void *) * tailleMax);
    pError((void *) p -> value, "Erreur allocation tableau élément pile", 3);
    p -> nbElemMax = tailleMax;

    return p;
}

unsigned int pileTaille(Pile *p) {
    pError((void *) p, "La pile n'existe pas ! (pileTaille)", 3);
    return *(p -> nbElem);
}

Pile *pilePush(Pile *p, void *e) {
    pError((void *) p, "La pile n'existe pas ! (pilePush)", 3);
    if (pileTaille(p) < (p -> nbElemMax)) {
        (p -> value)[pileTaille(p)] = e;
        ++(*(p -> nbElem));
    } else {
        pError(NULL, "La pile est pleine ! (pilePush)", 3);
    }
    return p;
}

Pile *pilePop(Pile *p) {
    if (!pileEmpty(p)) {
        --(*(p -> nbElem));
    }
    return p;
}

void pileDelete(Pile **p) {
    pError((void *) *p, "La pile n'existe pas ! (pileDelete)", 3);
    fOnStack(*p, fonctorDelete, NULL);
	
	free((*p) -> value);
	free((*p) -> nbElem);
    free((void *) *p);
    *p = NULL;
}


bool pileEmpty(Pile *p) {
    pError((void *) p, "La pile n'existe pas ! (pileEmpty)", 3);
    return *(p -> nbElem) == 0;
}

// retourne le caractère d'indice valueAt - 1
// honteux sur une pile
void *pileValueAt(Pile *p, unsigned int valueAt) {
    assert(pileTaille(p) >= valueAt && valueAt >= 1);
    return (p->value)[valueAt - 1];
}

// pas censé être possible mais j'en ai besoin pour la segmentation de threads
// je dois modifier l'indice courant
Pile *modifyStackValueINT(Pile *p, unsigned int atIndex, int newValue) {
    assert(pileTaille(p) > atIndex && atIndex >= 0);
    *((int *) ((p -> value)[atIndex])) = newValue; // encore plus honteux
    return p;
}

// copie la clef dans *destination
// pileCopy alloue une zone mémoire mise dans *dest
void pileCopyValueCHAR(Pile *p, unsigned char **dest) {
    pError((void *) p, "La pile n'existe pas ! (pileCopyValueCHAR)", 3);

    unsigned int tailleReelle = pileTaille(p) + 1;
    unsigned char *chaineFinale = malloc(tailleReelle);
    pError((void *) chaineFinale, "Erreur creation tableau pour une clef", 3);

    for (unsigned int i = 0 ; i < pileTaille(p) ; ++i) {
        chaineFinale[i] = *((unsigned char *) pileValueAt(p, i));
    }
    chaineFinale[pileTaille(p)] = '\0';
    *dest = chaineFinale;
}

void pileCopyINT(Pile *to, Pile *from) {
    pError((void *) from, "La pile de copie est null ! (pileCopyINT)", 3);
    pError((void *) to, "La pile de récéption est null (pileCopyINT)", 3);
    if (to -> nbElemMax != from -> nbElemMax) {
        pError(NULL, "Erreur les 2 piles ne font pas la meme taille ! (pileCopyINT)", 1);
    }

    int *elem;

    for (unsigned int i = 1 ; i <= pileTaille(from) ; ++i) {
        elem = malloc(sizeof(int));
        pError(elem, "Erreur allocation nouvel element pile int copie", 3);
		
        *elem = *((int *) pileValueAt(from, i));
        pilePush(to, elem);
    }
}

void pileCopyCHAR(Pile *to, Pile *from) {
    pError((void *) from, "La pile de copie est null ! (pileCopyCHAR)", 3);
    pError((void *) to, "La pile de récéption est null (pileCopyCHAR)", 3);
    if (to -> nbElemMax != from -> nbElemMax) {
        pError(NULL, "Erreur les 2 piles ne font pas la meme taille ! (pileCopyCHAR)", 1);
    }

    char *elem;

    for (unsigned int i = 1 ; i <= pileTaille(from) ; ++i) {
        elem = malloc(sizeof(char));
        pError(elem, "Erreur allocation nouvel element pile char copie", 3);
		
        *elem = *((char *) pileValueAt(from, i));
        pilePush(to, (void *) elem);
    }
}

void fOnStack(Pile *p, FunctorPile f, void *userData) {
    pError((void *) p, "Erreur fonctor : la pile est vide", 1);

    if (pileTaille(p) == 0) {
        return;
    }

    for (unsigned int i = 1; i <= pileTaille(p); ++i) {
        void *elem = pileValueAt(p, i);
        if (!elem) {
            fprintf(stderr, "Warning: Null element at index %u\n", i);
            continue;
        }
        f(elem, userData);
    }
}

/* 
    delete elems
*/
void fonctorDelete(void *elem, void *userData) {
    (void) userData;
    if (elem) {
        free(elem);
    }
}

