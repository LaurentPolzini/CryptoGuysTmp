#ifndef __NTREE_H__
#define __NTREE_H__

#include <stdbool.h>
#include "../Pile.h"

// définition de structure d'arbre N-aire opaque
typedef struct _NTree TreeNaire;
typedef TreeNaire *ptrTreeNaire;

// arbre permettant d'ajouter a la fin
// systeme proche d'une sentinelle
typedef struct _Arbre Arbre;
typedef Arbre *ptrArbre;

// des fonctions actant sur un arbre N-aire
// par exemple print t->value
typedef void(*OperateFunctor)(TreeNaire *);

// retourne un arbre sans fils avec la valeur \0
Arbre *NTreeCreate(void);

// retourne la valeur de l'arbre t
unsigned char NTreeValue(TreeNaire *t);

Arbre *ArbreAdd(Arbre *tree, unsigned char *value, int tailleValue);

// Ajoute une valeur à l'arbre Naire
// je met la valeur dans chaque sous arbre de tree
TreeNaire *NTreeAdd(TreeNaire *tree, unsigned char *value, int tailleValue);

// détruit l'arbre t
void arbreDelete(ptrArbre *t);

// affiche l'arbre
void afficheArbre(Arbre *tree);

// parcours l'arbre de maniere prefixe et applique f a chaque noeud
void prefixNTree(TreeNaire *tree, OperateFunctor f);

// parcours l'arbre de maniere postfix (par la droite)
// applique f a chaque noeud
void postfixNTree(TreeNaire *tree, OperateFunctor f);

// crée un arbre avec les fils fils et la valeur value
TreeNaire *NTreeCons(TreeNaire * pere, TreeNaire **fils, unsigned char value);

// retourne si l'arbre tree est vide 
//  -> tree NULL ou pas de fils
bool arbreEmpty(Arbre *tree);

// retourne si l'arbre n-aire est vide
// -> tree NULL ou valuer \0
bool NtreeEmpty(TreeNaire *tree);

// crée les fils à un arbre sans fils : 
// transforme le tableau de value en noeud individuel
// input: [568]
// crée 3 fils 5, 6 et 8 avec comme pere treeToAttachSons
// chacun 0 fils et leur pere 
TreeNaire **TreeFromValues(TreeNaire *treeToAttachSons, unsigned char *value, int tailleValue);

// retourne le nombre de candidates finales
// -> le produit de chaque nb de fils
int nombreClefsCandidates(Arbre *tree);

// profondeur de l'arbre (la taille de chacune des clefs)
int tailleClefs(Arbre *tree);

// t l'arbre possédant toutes les clefs
// clef un tableau de chaine de caractères (est modifé)
// input (par arbre): [[568], [1], [23]]
// output (dans clef): [[512], [513], [612], [513], [812], [813]]

void getClefsCandidatesNT(Arbre *t, unsigned char **clef);

void getClefsCandidatesTHREAD(Arbre *t, unsigned char **clef);

#endif
