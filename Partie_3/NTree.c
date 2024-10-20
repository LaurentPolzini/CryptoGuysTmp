#include <stdio.h>
#include "NTree.h"
#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>
#include "./Queue.h"
#include <string.h>

void clefsCandidates(TreeNaire *t, unsigned char **clef, int *indice, Queue *p);
void FunctorDelete(TreeNaire *tree);
void FunctorPrint(TreeNaire *tree);

// la définition de la structure opaque définie dans NTree.h
struct _NTree {
    TreeNaire *pere;
    int nbFils;
    unsigned char value;
    TreeNaire **fils;
};


// retourne un arbre
TreeNaire *NTreeCreate(void) {
    return NTreeCons(NULL, '\0');
}


// Ajoute une valeur à l'arbre Naire
// étant donné que je cherche toutes les possibilités de clefs
// je met les valeurs dans chaque sous arbre
TreeNaire *NTreeAdd(TreeNaire *tree, unsigned char *value, int tailleValue) {
    if (tree -> nbFils == 0) {
        // TreeFromValues attache les fils crées a tree, pas de retour
        TreeFromValues(tree, value, tailleValue);
        return tree;
    }
    int indFils = 0;

    while (indFils < (tree -> nbFils)) {
        (tree->fils)[indFils] = NTreeAdd((tree -> fils)[indFils], value, tailleValue);
        ++indFils;
    }

    return tree;
}


// Detruit l'arbre tree et ses sous arbres
void NTreeDelete(TreeNaire *tree) {
    postfixNTree(tree, FunctorDelete);
}

// valeur de l'arbre t
unsigned char NTreeValue(TreeNaire *t) {
    assert(t);
    return t -> value;
}

// détruit l'arbre tree
void FunctorDelete(TreeNaire *tree) {
    assert(tree != NULL);
    free((void *) tree);
    tree = NULL;
}

void afficheArbre(TreeNaire *tree) {
    prefixNTree(tree, FunctorPrint);
}

// print l'arbre tree
void FunctorPrint(TreeNaire *tree) {
    printf("%c\n",tree->value);
}

// parcours l'arbre de maniere prefixe (a gauche)
void prefixNTree(TreeNaire *tree, OperateFunctor f) {
    if (!treeEmpty(tree)) {
        f(tree);
    }

    int indFils = 0;
    while (indFils < tree -> nbFils) {
        prefixNTree((tree -> fils)[indFils++], f);
    }
}

// parcours l'arbre de maniere postfix (par la droite)
void postfixNTree(TreeNaire *tree, OperateFunctor f) {    
    int indFils = 0;
    while (indFils < tree -> nbFils) {
        postfixNTree((tree -> fils)[indFils++], f);
    }
    f(tree);
    return;
}


// crée un arbre avec les fils fils et la valeur value
TreeNaire *NTreeCons(TreeNaire **fils, unsigned char value) {
    TreeNaire *tree = malloc(sizeof(TreeNaire));
    if (!tree) {
        perror("Erreur allocation mémoire d'un arbre N-aire pour les clefs");
        exit(1);
    }
    tree -> pere = NULL;
    tree -> value = value;
    tree -> fils = fils;
    int tailleFils = (fils) ? (sizeof(fils) / sizeof(TreeNaire *)) : 0;;
    
    tree -> nbFils = tailleFils;
    if (fils) {
        for (int i = 0 ; i < tree -> nbFils ; ++i) {
            ((tree -> fils)[i]) -> pere = tree;
        }
    }

    return tree;
}

// retourne si l'arbre tree est vide
bool treeEmpty(TreeNaire *tree) {
    return !tree || tree -> value == '\0';
}

// crée les fils à un arbre sans fils : 
// transforme le tableau de value en noeud individuel
void TreeFromValues(TreeNaire *treeToAttachSons, unsigned char *value, int tailleValue) {
    TreeNaire **trees = malloc(tailleValue * sizeof(TreeNaire *));
    if (trees) {
        for (int i = 0 ; i < tailleValue ; ++i) {
            trees[i] = NTreeCons(NULL, value[i]);
            trees[i] -> pere = treeToAttachSons;
        }
        if (treeToAttachSons) {
            treeToAttachSons -> fils = trees;
            treeToAttachSons -> nbFils = tailleValue;
        }
    } else {
        perror("Erreur allocation mémoire pour le tableau de fils");
        exit(1);
    }
}

// retourne le nombre de clefs candidates finales
// -> le produit de chaque nb de fils
int nombreClefsCandidates(TreeNaire *tree) {
    if (tree -> nbFils == 0) {
        return 0;
    }
    int taille = 1;
    while (tree -> nbFils != 0) {
        taille *= tree -> nbFils;
        tree = tree -> fils[0];
    }

    return taille;
}

// profondeur de l'arbre
int tailleClefs(TreeNaire *tree) {
    int taille = 0;
    while (tree -> nbFils != 0) {
        ++taille;
        tree = tree -> fils[0];
    }

    return taille;
}

// t l'arbre possédant toutes les clefs
// clef un pointeur vers un tableau de chaine de caractères (est modifé)
// la taille de clef doit être egal à nombreClefsCandidates * tailleClefs
// queue doit avoir pour taille max tailleClefs
// indice doit être initialisé a 0
// indice est l'indice d'insertion dans *clef[indice] = queue->values
void clefsCandidates(TreeNaire *t, unsigned char **clef, int *indice, Queue *q) {
    if (t -> value != '\0') { // le noeud parent ultime : celui tout en haut
        queuePush(q, t -> value);
    }

    if (t -> nbFils == 0) {
        queueCopy(q, &(clef[*indice]));
        ++(*indice);
    } else {
        int indFils = 0;

        while (indFils < (t -> nbFils)) {
            clefsCandidates((t -> fils)[indFils], clef, indice, q);
            ++indFils;
        }
    }
    if (t -> value != '\0') {
        queuePop(q);
    }
    return;
}

// t l'arbre possédant toutes les clefs
// clef un tableau de chaine de caractères (est modifé)
// input (par arbre): [[568], [1], [23]]
// output (dans clef): [[512], [513], [612], [513], [812], [813]]
void getClefsCandidates(TreeNaire *t, unsigned char **clef) {
    Queue *q = queueCreate(tailleClefs(t));
    int ind = 0;
    clefsCandidates(t, clef, &ind, q);
    queueDelete(q);
}
