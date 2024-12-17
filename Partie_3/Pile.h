#ifndef __PILE_H__
#define __PILE_H__

#include <stdbool.h>

typedef struct _Pile Pile;
typedef Pile *ptrPile;

// elem and userData
typedef void(*FunctorPile)(void *, void *);

// crée une Pile de taille tailleMax
Pile *pileCreate(int tailleMax);

// retourne le nombre d'element dans la file p
// [512] est de taille 3
unsigned int pileTaille(Pile *p);

// ajoute un element a la Pile
// [51] -> [512]
Pile *pilePush(Pile *p, void *e);

// retire le dernier element ajouté
// [512\0] -> [51\0]
Pile *pilePop(Pile *p);

// delete la Pile p
void pileDelete(Pile **p);

// retourne si la Pile est vide (0 element)
bool pileEmpty(Pile *p);

// si la Pile vaut [510] et valueAt 1
// renvoie 5
void *pileValueAt(Pile *p, unsigned int valueAt);

// copy la chaine contenue dans p a destination de dest
void pileCopyValueCHAR(Pile *p, unsigned char **dest);

void pileCopyCHAR(Pile *to, Pile *from);
void pileCopyINT(Pile *to, Pile *from);

Pile *modifyStackValueINT(Pile *p, unsigned int atIndex, int newValue);

void fonctorDelete(void *elem, void *userData);

void fOnStack(Pile *p, FunctorPile f, void *userData);

#endif
