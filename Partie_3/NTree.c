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
void NTreeDelete(TreeNaire *tree);

Arbre *treeAddValues(Arbre *tree, unsigned char *value, int tailleValue);
Arbre *treeEmptyAddValues(Arbre *tree, unsigned char *value, int tailleValue);

// la définition de la structure opaque définie dans NTree.h
struct _NTree {
    TreeNaire *pere;
    int nbFils;
    unsigned char value;
    TreeNaire **fils;
};

struct _Arbre {
    TreeNaire *premiersFils;
    TreeNaire **dernierFils;
    int nbClefsTotal; // taille de dernierFils
    int tailleClefs; // la profondeur de l'arbre
};


// retourne un arbre
Arbre *NTreeCreate(void) {
    Arbre *tree = malloc(sizeof(struct _Arbre));
    if (!tree) {
        perror("Erreur allocation mémoire pour l'arbre");
        exit(1);
    }
    tree -> dernierFils = malloc(sizeof(TreeNaire *) * 1024);

    tree -> premiersFils = (tree -> dernierFils)[0] = NTreeCons(NULL, NULL, '\0');
    tree -> nbClefsTotal = tree -> tailleClefs = 0;

    return tree;
}

// Ajoute une valeur à l'arbre Naire
// étant donné que je cherche toutes les possibilités de clefs
// je met les valeurs dans chaque sous arbre
Arbre *NTreeAdd(Arbre *tree, unsigned char *value, int tailleValue) {
    if (treeEmpty(tree)) {
        tree = treeEmptyAddValues(tree, value, tailleValue);
    } else {
        tree = treeAddValues(tree, value, tailleValue);
    }

    return tree;
}
/*
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
*/
// Detruit l'arbre tree et ses sous arbres
void NTreeDelete(TreeNaire *tree) {
    postfixNTree(tree, FunctorDelete);
}

void arbreDelete(ptrArbre *t) {
    NTreeDelete((*t) -> premiersFils);
    free(*t);
    *t = NULL;
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

void afficheArbre(Arbre *tree) {
    prefixNTree(tree -> premiersFils, FunctorPrint);
}

// print l'arbre tree
void FunctorPrint(TreeNaire *tree) {
    printf("%c\n",tree->value);
}

// parcours l'arbre de maniere prefixe (a gauche)
void prefixNTree(TreeNaire *tree, OperateFunctor f) {
    if (!NtreeEmpty(tree)) {
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
TreeNaire *NTreeCons(TreeNaire *pere, TreeNaire **fils, unsigned char value) {
    TreeNaire *tree = malloc(sizeof(TreeNaire));
    if (!tree) {
        perror("Erreur allocation mémoire d'un arbre N-aire pour les clefs");
        exit(1);
    }
    tree -> pere = pere;
    tree -> value = value;
    tree -> fils = fils;
    int tailleFils = (fils) ? (int) (sizeof(fils) / sizeof(TreeNaire *)) : 0;;
    
    tree -> nbFils = tailleFils;
    for (int i = 0 ; i < tailleFils ; ++i) {
        ((tree -> fils)[i]) -> pere = tree;
    }

    return tree;
}

// retourne si l'arbre tree est vide
bool treeEmpty(Arbre *tree) {
    return !tree || tree -> premiersFils -> nbFils == 0;
}

// retourne si l'arbre n-aire tree est vide
bool NtreeEmpty(TreeNaire *tree) {
    return !tree || tree -> value == '\0';
}

Arbre *treeEmptyAddValues(Arbre *tree, unsigned char *value, int tailleValue) {
    TreeNaire **tabTreesFils = malloc(tailleValue * sizeof(TreeNaire *));
    if (!tabTreesFils) {
        perror("Erreur allocation mémoire pour le tableau de fils");
        exit(1);
    }

    for (int i = 0 ; i < tailleValue ; ++i) {
        tabTreesFils[i] = NTreeCons(tree -> premiersFils, NULL, value[i]);
        (tree -> dernierFils)[i] = tabTreesFils[i];
    }

    tree -> premiersFils -> fils = tabTreesFils;
    tree -> premiersFils -> nbFils = tailleValue;
    tree -> nbClefsTotal = tailleValue;
    tree -> tailleClefs = 1;

    return tree;
}

// crée les fils à un arbre sans fils : 
// transforme le tableau de value en noeud individuel
Arbre *treeAddValues(Arbre *tree, unsigned char *value, int tailleValue) {
    TreeNaire ***tabTreesFils = malloc(tree -> nbClefsTotal * sizeof(TreeNaire **));
    TreeNaire **tmp = malloc(tailleValue * tree -> nbClefsTotal * sizeof(TreeNaire *));
    int indTmp = 0;

    if (!tabTreesFils) {
        perror("Erreur allocation mémoire pour le tableau de fils");
        exit(1);
    }

    for (int i = 0 ; i < tree -> nbClefsTotal ; ++i) {
        tabTreesFils[i] = malloc(tailleValue * sizeof(TreeNaire *));
        if (!tabTreesFils[i]) {
            perror("Erreur allocation mémoire pour le tableau de fils");
            exit(1);
        }
        for (int j = 0 ; j < tailleValue ; ++j) {
            tabTreesFils[i][j] = NTreeCons((tree -> dernierFils)[i], NULL, value[j]);
            indTmp = (i * tailleValue) + j;
            tmp[indTmp] = tabTreesFils[i][j]; // change les derniers fils
        }
        (tree -> dernierFils)[i] -> fils = tabTreesFils[i];
        (tree -> dernierFils)[i] -> nbFils = tailleValue;
    }

    free(tree -> dernierFils);
    tree -> dernierFils = NULL;
    free(tabTreesFils);
    tabTreesFils = NULL;

    tree -> dernierFils = tmp;
    // j'ai a l'origine 3 fils
    // j'ajoute les valeurs '1' et '2'
    // ces valeurs se retrouves dans chaque fils
    // 3 * 2 = 6, il y aura 6 fils tout en bas de l'arbre
    tree -> nbClefsTotal *= tailleValue;
    tree -> tailleClefs += 1;

    return tree;
}

// crée les fils à un arbre sans fils : 
// transforme le tableau de value en noeud individuel
TreeNaire **TreeFromValues(TreeNaire *treeToAttachSons, unsigned char *value, int tailleValue) {
    TreeNaire **trees = malloc(tailleValue * sizeof(TreeNaire *));
    if (trees) {
        for (int i = 0 ; i < tailleValue ; ++i) {
            trees[i] = NTreeCons(treeToAttachSons, NULL, value[i]);
        }
        if (treeToAttachSons) {
            treeToAttachSons -> fils = trees;
            treeToAttachSons -> nbFils = tailleValue;
        }
    } else {
        perror("Erreur allocation mémoire pour le tableau de fils");
        exit(1);
    }
    return trees;
}

// retourne le nombre de clefs candidates finales
// -> le produit de chaque nb de fils
int nombreClefsCandidates(Arbre *tree) {
    assert(tree);
    return tree -> nbClefsTotal;
}

// profondeur de l'arbre
int tailleClefs(Arbre *tree) {
    assert(tree);
    return tree -> tailleClefs;
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
void getClefsCandidates(Arbre *t, unsigned char **clef) {
    Queue *q = queueCreate(tailleClefs(t));
    int ind = 0;
    clefsCandidates(t->premiersFils, clef, &ind, q);
    queueDelete(q);
}
