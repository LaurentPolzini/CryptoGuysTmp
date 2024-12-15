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
    unsigned int nbElem;
    unsigned int nbElemMax;
    void **value;
};

// tailleMax la capacité de la pile
Pile *pileCreate(int tailleMax) {
    Pile *p = malloc(sizeof(Pile) + sizeof(void *) * tailleMax);
    pError((void *) p, "Erreur création pile", 3);

    p -> nbElem = 0;
    
    p -> value = (void **)(p + 1);
    p -> nbElemMax = tailleMax;

    return p;
}

int pileTaille(Pile *p) {
    pError((void *) p, "La pile n'existe pas ! (pileTaille)", 3);
    return p -> nbElem;
}

Pile *pilePush(Pile *p, void *e) {
    pError((void *) p, "La pile n'existe pas ! (pilePush)", 3);
    if ((p -> nbElem) < (p -> nbElemMax)) {
        (p -> value)[(p -> nbElem)++] = e;
    } else {
        pError(NULL, "La pile est pleine ! (pilePush)", 3);
    }
    return p;
}

Pile *pilePop(Pile *p) {
    if (!pileEmpty(p)) {
        --(p -> nbElem);
    }
    return p;
}

void pileDelete(Pile *p) {
    pError((void *) p, "La pile n'existe pas ! (pileDelete)", 3);
    fOnStack(p, fonctorDelete, NULL);
    free((void *) p);
    p = NULL;
}

bool pileEmpty(Pile *p) {
    pError((void *) p, "La pile n'existe pas ! (pileEmpty)", 3);
    return p -> nbElem == 0;
}

// retourne le caractère d'indice valueAt - 1
void *pileValueAt(Pile *p, unsigned int valueAt) {
    assert(p->nbElem >= valueAt && valueAt >= 1);
    return (p->value)[valueAt - 1];
}

// pas censé être possible mais j'en ai besoin pour la segmentation de threads
// je dois modifier l'indice courant
Pile *modifyStackValueINT(Pile *p, unsigned int atIndex, int newValue) {
    assert(p->nbElem > atIndex && atIndex >= 0);
    *((int *) ((p -> value)[atIndex])) = newValue;
    return p;
}

// copie la clef dans *destination
// pileCopy alloue une zone mémoire mise dans *dest
void pileCopyValueCHAR(Pile *p, unsigned char **dest) {
    pError((void *) p, "La pile n'existe pas ! (pileCopyValueCHAR)", 3);

    unsigned int tailleReelle = pileTaille(p) + 1;
    unsigned char *chaineFinale = malloc(tailleReelle);
    pError((void *) chaineFinale, "Erreur creation tableau pour une clef", 3);

    for (int i = 0 ; i < pileTaille(p) ; ++i) {
        chaineFinale[i] = *((unsigned char *) pileValueAt(p, i));
    }
    chaineFinale[pileTaille(p)] = '\0';
    *dest = chaineFinale;
}

Pile *pileCopyINT(Pile *from) {
    pError((void *) from, "La pile de copie est null ! (pileCopyINT)", 3);

    Pile *to = pileCreate(from->nbElemMax);
    pError((void *) to, "Erreur création clef", 3);

    to->nbElem = from -> nbElem;
    int *elem = malloc(sizeof(int));
    pError(elem, "Erreur allocation nouvel element pile copie", 3);
    for (unsigned int i = 1 ; i <= from -> nbElem ; ++i) {
        *elem = *((int *) pileValueAt(from, i));
        (to -> value)[i - 1] = (void *) elem;
        elem = malloc(sizeof(int));
        pError(elem, "Erreur allocation nouvel element pile copie", 3);
    }

    return to;
}

Pile *pileCopyCHAR(Pile *from) {
    pError((void *) from, "La pile de copie est null !", 3);

    Pile *to = pileCreate(from->nbElemMax);
    pError((void *) to, "Erreur création clef", 3);

    to->nbElem = from -> nbElem;
    char *elem = malloc(sizeof(char));
    pError(elem, "Erreur allocation nouvel element pile", 3);
    for (unsigned int i = 1 ; i <= from -> nbElem ; ++i) {
        *elem = *((char *) pileValueAt(from, i));
        (to -> value)[i - 1] = elem;
    }

    return to;
}

void fOnStack(Pile *p, FunctorPile f, void *userData) {
    for (unsigned int i = 1 ; i <= p -> nbElem ; ++i) {
        f(pileValueAt(p, i), userData);
    }
}

/* 
    delete elems
*/
void fonctorDelete(void *elem, void *userData) {
    (void) userData;
    free(elem);
}

