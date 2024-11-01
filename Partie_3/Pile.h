#ifndef __PILE_H__
#define __PILE_H__

#include <stdbool.h>

typedef struct _Pile Pile;
typedef Pile *ptrPile;

// crée une Pile de taille tailleMax
//  -> la taille reelle est tailleMax + 1
// pour le \0 final
Pile *pileCreate(int tailleMax);

// retourne le nombre d'element dans la file p
// [512\0] est de taille 3
int pileTaille(Pile *p);

// ajoute un element a la Pile
// [51\0] -> [512\0]
Pile *pilePush(Pile *p, unsigned char c);

// retire le dernier element ajouté
// [512\0] -> [51\0]
Pile *pilePop(Pile *p);

// delete la Pile p
void pileDelete(Pile *p);

// retourne si la Pile est vide (0 element)
bool pileEmpty(Pile *p);

// si la Pile vaut [510] et valueAt 1
// renvoie 5
unsigned char pileValueAt(Pile *p, unsigned int valueAt);

// retourne les valeurs de la Pile
// retourne par exemple [510\0]
unsigned char *pileValue(Pile *p);

// copy la chaine contenue dans p a destination de dest
void pileCopyValue(Pile *p, unsigned char **dest);

Pile *pileCopy(Pile *from);

#endif
