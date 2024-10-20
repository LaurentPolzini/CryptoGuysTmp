#ifndef __NTREE_H__
#define __NTREE_H__

#include <stdbool.h>
#include "./Queue.h"

// définition de structure d'arbre N-aire opaque
typedef struct _NTree TreeNaire;
typedef TreeNaire *ptrTreeNaire;

// des fonctions actant sur un arbre N-aire
// par exemple print t->value
typedef void(*OperateFunctor)(TreeNaire *);

// retourne un arbre sans fils avec la valeur \0
TreeNaire *NTreeCreate(void);

// retourne la valeur de l'arbre t
unsigned char NTreeValue(TreeNaire *t);

// Ajoute une valeur à l'arbre Naire
// je met la valeur dans chaque sous arbre de tree
TreeNaire *NTreeAdd(TreeNaire *tree, unsigned char *value, int tailleValue);

// Detruit l'arbre tree et ses sous arbres
void NTreeDelete(TreeNaire *tree);

// affiche l'arbre
void afficheArbre(TreeNaire *tree);

// parcours l'arbre de maniere prefixe et applique f a chaque noeud
void prefixNTree(TreeNaire *tree, OperateFunctor f);

// parcours l'arbre de maniere postfix (par la droite)
// applique f a chaque noeud
void postfixNTree(TreeNaire *tree, OperateFunctor f);

// crée un arbre avec les fils fils et la valeur value
TreeNaire *NTreeCons(TreeNaire **fils, unsigned char value);

// retourne si l'arbre tree est vide 
//  -> tree NULL ou valeur \0
bool treeEmpty(TreeNaire *tree);

// crée les fils à un arbre sans fils : 
// transforme le tableau de value en noeud individuel
// input: [568]
// crée 3 fils 5, 6 et 8 avec comme pere treeToAttachSons
// chacun 0 fils et leur pere 
void TreeFromValues(TreeNaire *treeToAttachSons, unsigned char *value, int tailleValue);

// retourne le nombre de candidates finales
// -> le produit de chaque nb de fils
int nombreClefsCandidates(TreeNaire *tree);

// profondeur de l'arbre (la taille de chacune des clefs)
int tailleClefs(TreeNaire *tree);

// t l'arbre possédant toutes les clefs
// clef un tableau de chaine de caractères (est modifé)
// input (par arbre): [[568], [1], [23]]
// output (dans clef): [[512], [513], [612], [513], [812], [813]]
void getClefsCandidates(TreeNaire *t, unsigned char **clef);

#endif
