#ifndef __TREE_H__
#define __TREE_H__

#include <stdbool.h>
#include "../Pile.h"

typedef struct _Tree Tree;

typedef struct _Noeud Noeud;
typedef Noeud *ptrNoeud;

Tree *addTree(Tree *t, unsigned char *carCand, int nbCar);

void deleteTree(Tree *t);

Tree *createTree(void);

void clefsCandidates(Noeud *n, Pile *p, unsigned char **clefs, unsigned long *indice);

Noeud *getFirstNoeud(Tree *t);

unsigned long getNbClef(Tree *t);

unsigned long getTailleClef(Tree *t);

bool treeEmpty(Tree *t);

void getClefsCandidates(Tree *t, unsigned char **clefs);

void freeDoubleArray(unsigned char ***arr, unsigned long len);

#endif
