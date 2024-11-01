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
    unsigned char *value;
};

// tailleMax la capacité de la pile
Pile *pileCreate(int tailleMax) {
    Pile *p = malloc(sizeof(Pile));
    pError((void *) p, "Erreur création pile", 3);

    p -> nbElem = 0;
    
    // tailleMax + 1 car il faut \0
    unsigned char *elems = malloc(tailleMax + 1);
    pError((void *) elems, "Erreur allocation de la taille d'éléments de la pile", 3);
    elems[0] = '\0';
    p -> value = elems;
    p -> nbElemMax = tailleMax;

    return p;
}

int pileTaille(Pile *p) {
    pError((void *) p, "La pile n'existe pas ! (pileTaille)", 3);
    return p -> nbElem;
}

Pile *pilePush(Pile *p, unsigned char c) {
    pError((void *) p, "La pile n'existe pas ! (pilePush)", 3);
    if ((p -> nbElem) < (p -> nbElemMax)) {
        (p -> value)[p -> nbElem + 1] = (p -> value)[p -> nbElem]; // le \0
        (p -> value)[(p -> nbElem)++] = c;
    } else {
        pError(NULL, "La pile est pleine ! (pilePush)", 3);
    }
    return p;
}

Pile *pilePop(Pile *p) {
    pError((void *) p, "La pile n'existe pas ! (pilePop)", 3);
    if (!pileEmpty(p)) {
        (p -> value)[--(p -> nbElem)] = '\0';
    }
    return p;
}

void pileDelete(Pile *p) {
    pError((void *) p, "La pile n'existe pas ! (pileDelete)", 3);

    free((void *) p -> value);
    p -> value = NULL;
    free((void *) p);
    p = NULL;
}

bool pileEmpty(Pile *p) {
    pError((void *) p, "La pile n'existe pas ! (pileEmpty)", 3);
    return p -> nbElem == 0;
}

// retourne le caractère d'indice valueAt - 1
unsigned char pileValueAt(Pile *p, unsigned int valueAt) {
    assert(p->nbElem >= valueAt && valueAt >= 1);
    return (p->value)[valueAt - 1];
}

// retourne les valeurs de la pile
// retourne par exemple [510\0]
unsigned char *pileValue(Pile *p) {
    pError((void *) p, "La pile n'existe pas ! (pileValue)", 3);
    return p->value;
}

// copie la clef dans *destination
// pileCopy alloue une zone mémoire mise dans *dest
void pileCopyValue(Pile *p, unsigned char **dest) {
    pError((void *) p, "La pile n'existe pas ! (pileCopyValue)", 3);

    unsigned int tailleReelle = pileTaille(p) + 1;
    unsigned char *chaineFinale = malloc(tailleReelle);
    pError((void *) chaineFinale, "Erreur creation tableau pour une clef", 3);

    for (unsigned int i = 0 ; i < tailleReelle ; ++i) {
        chaineFinale[i] = pileValue(p)[i];
    }
    *dest = chaineFinale;
}

Pile *pileCopy(Pile *from) {
    pError((void *) from, "La pile de copie est null !", 3);

    Pile *to = pileCreate(from->nbElemMax);
    pError((void *) to, "Erreur création clef", 3);

    to->nbElem = from -> nbElem;
    strcpy((char *) to->value, (const char *) pileValue(from));

    return to;
}
