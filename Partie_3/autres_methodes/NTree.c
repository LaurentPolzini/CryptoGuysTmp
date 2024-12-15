#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <math.h>
#include <pthread.h>
#include "../Pile.h"
#include "NTree.h"
#include "../../utilitaire/utiL.h"

/*
Code erreurs:
    Erreurs liées aux mutex = 3
    Erreurs allocation mem = 1

*/

typedef struct {
    int debut;
    int fin;
    Pile *pile;
    TreeNaire *tree;
    unsigned char **clef;
    int *indice;
} threadClefCandInfo;

typedef struct {
    Arbre *tree;
    int nbValues;
    unsigned char *values;
    int debut;
    int fin;
    TreeNaire **newLastSons;
} threadAddValuesInfo;

pthread_mutex_t mutexEcritureClef;
void *threadClefsCandidatesNT(void *ti);
void *threadAddValues(void *info);

void clefsCandidatesNT(TreeNaire *t, unsigned char **clef, int *indice, Pile *p);
void FunctorDelete(TreeNaire *tree);
void FunctorPrint(TreeNaire *tree);
void NTreeDelete(TreeNaire *tree);

Arbre *treeAddValues(Arbre *tree, unsigned char *value, int tailleValue);
Arbre *treeEmptyAddValues(Arbre *tree, unsigned char *value, int tailleValue);

// la définition de la structure opapue définie dans NTree.h
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
// étant donné pue je cherche toutes les possibilités de clefs
// je met les valeurs dans chapue sous arbre
Arbre *ArbreAdd(Arbre *tree, unsigned char *value, int tailleValue) {
    if (arbreEmpty(tree)) {
        tree = treeEmptyAddValues(tree, value, tailleValue);
    } else {
        tree = treeAddValues(tree, value, tailleValue);
    }

    return tree;
}

// Ajoute une valeur à l'arbre Naire
// étant donné pue je cherche toutes les possibilités de clefs
// je met les valeurs dans chapue sous arbre
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
bool arbreEmpty(Arbre *tree) {
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
        // TODO THREADS ICI
        tabTreesFils[i] = malloc(tailleValue * sizeof(TreeNaire *));
        pError(tabTreesFils[i], "Erreur allocation mémoire pour le tableau de fils", 1);
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
    // ces valeurs se retrouves dans chapue fils
    // 3 * 2 = 6, il y aura 6 fils tout en bas de l'arbre
    tree -> nbClefsTotal *= tailleValue;
    tree -> tailleClefs += 1;

    return tree;
}

Arbre *treeAddValuesTHREAD(Arbre *tree, unsigned char *value, int tailleValue) {
    TreeNaire ***tabTreesFils = malloc(tree -> nbClefsTotal * sizeof(TreeNaire **));
    TreeNaire **tmp = malloc(tailleValue * tree -> nbClefsTotal * sizeof(TreeNaire *));

    if (!tabTreesFils) {
        perror("Erreur allocation mémoire pour le tableau de fils");
        exit(1);
    }
    int nbSegment = (int) sqrt(tree -> nbClefsTotal);
    int tailleSegment = (int) tree -> nbClefsTotal / nbSegment;
    int deb; int fin;

    pthread_t thid[nbSegment];
    threadAddValuesInfo tinfo[nbSegment];

    for (int i = 0 ; i < nbSegment ; ++i) {
        deb = i * tailleSegment;
        if (i == nbSegment - 1) {
            fin = tree -> nbClefsTotal; // TODO verifier que c'est un bon truc
            // l'idee en fait c'est qu'avec le sqrt je suis pas sur que ca fasse tous les fils
            // du coup c'est juste pour tester si on va jusqu'au bout
        } else {
            fin = ((i + 1) * tailleSegment) - 1;
        }
        tinfo[i].debut = deb;
        tinfo[i].fin = fin;
        tinfo[i].nbValues = tailleValue;
        tinfo[i].values = value;
        tinfo[i].newLastSons = tmp;
        tinfo[i].tree = tree;

        pthread_create(&thid[i], NULL, threadAddValues, &tinfo[i]);
    }

    for (int i = 0 ; i < nbSegment ; ++i) {
        pthread_join(thid[i], NULL);
    }

    free(tree -> dernierFils);
    tree -> dernierFils = NULL;
    free(tabTreesFils);
    tabTreesFils = NULL;

    tree -> dernierFils = tmp;
    // j'ai a l'origine 3 fils
    // j'ajoute les valeurs '1' et '2'
    // ces valeurs se retrouves dans chapue fils
    // 3 * 2 = 6, il y aura 6 fils tout en bas de l'arbre
    tree -> nbClefsTotal *= tailleValue;
    tree -> tailleClefs += 1;

    return tree;
}

/*
    Debut - fin : pour chaque fils compris entre debut et fin
    on ajoute un tableau de fils contenant les valeurs info.values
    le pere de ce nouveau tableau est info.tree
*/

void *threadAddValues(void *info) {
    threadAddValuesInfo ti = *((threadAddValuesInfo *) info);
    int tailleSeg = ti.fin - ti.debut;

    // nb de tableau de fils
    // (autant de tableau que l'on traitera de fils)
    TreeNaire ***tabTreesFils = malloc(tailleSeg * sizeof(TreeNaire **));
    int indTabTreeFils = 0; // besoin d'un indice perso

    int indAjoutNewLastSonsTab = ti.debut * ti.nbValues;

    for (int i = ti.debut ; i < ti.fin ; ++i) {
        tabTreesFils[indTabTreeFils] = malloc(ti.nbValues * sizeof(TreeNaire *));
        for (int j = 0 ; j < ti.nbValues ; ++j) {
            indAjoutNewLastSonsTab += j;

            tabTreesFils[indTabTreeFils][j] = NTreeCons((ti.tree -> dernierFils)[i], NULL, ti.values[j]);
            ti.newLastSons[indAjoutNewLastSonsTab] = tabTreesFils[indTabTreeFils][j];
        }
        (ti.tree -> dernierFils)[i] -> fils = tabTreesFils[indTabTreeFils++];
        (ti.tree -> dernierFils)[i] -> nbFils = ti.nbValues;
    }

    pthread_exit(NULL);
}



// crée les fils à un arbre sans fils : 
// transforme le tableau de value en noeud individuel
    
    // sert encore ?

TreeNaire **TreeFromValues(TreeNaire *treeToAttachSons, unsigned char *value, int tailleValue) {
    TreeNaire **trees = malloc(tailleValue * sizeof(TreeNaire *));
    if (trees) {
        for (int i = 0 ; i < tailleValue ; ++i) {
            // TODO THREADS ICI
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
// -> le produit de chapue nb de fils
int nombreClefsCandidates(Arbre *tree) {
    assert(tree);
    return tree -> nbClefsTotal;
}

// profondeur de l'arbre
int tailleClefs(Arbre *tree) {
    assert(tree);
    return tree -> tailleClefs;
}


/*
        TODO
        Je pense pu'ici il faut faire des forks sur chaque moitie de fils
        A
       / \
      B   C
    -> lance un fork sur B et C, et ainsi de suite
    donc avec des boucles puoi 

*/

void clefsCandidatesTHREAD(TreeNaire *t, unsigned char **clef, int *indice, Pile *p) {
    if (t -> value != '\0') { // le noeud parent ultime : celui tout en haut
        pilePush(p, (void *) &(t -> value));
    }

    if (t -> nbFils == 0) {
        if (pthread_mutex_lock(&mutexEcritureClef) != 0) {
            pError(NULL, "Erreur prendre jeton mutex", 3);
        }
        pileCopyValueCHAR(p, &(clef[*indice]));
        ++(*indice);
        if (pthread_mutex_unlock(&mutexEcritureClef) != 0) {
            pError(NULL, "Erreur don jeton mutex", 3);
        }
    } else {
        int nbSegment = (int) (t -> nbFils) / 8; // nb thread
        //int nbSegment = (int) sqrt(t -> nbFils); // nb thread
        int tailleSegment = (int) t -> nbFils / nbSegment;
        int deb; int fin;
        pthread_t thid[nbSegment];
        threadClefCandInfo ti[nbSegment];

        for (int i = 0 ; i < nbSegment ; ++i) {
            deb = i * tailleSegment;
            fin = (i + 1) * tailleSegment; // juste avant le debut de l'autre thread
            ti[i].debut = deb;
            ti[i].fin = fin;
            ti[i].tree = t;
            ti[i].pile = p;
            ti[i].clef = clef;
            ti[i].indice = indice;
            
            pthread_create(&thid[i], NULL, threadClefsCandidatesNT, (void *) &ti[i]);
        }
        for (int i = 0 ; i < nbSegment ; ++i) {
            pthread_join(thid[i], NULL); // TODO ici ??
        }
    }
    if (t -> value != '\0') {
        pilePop(p);
    }
    return;
}


void *threadClefsCandidatesNT(void *ti) {
    threadClefCandInfo tinfo = *((threadClefCandInfo *) ti);
    Pile *newPile = pileCopyCHAR(tinfo.pile);
    for (int i = tinfo.debut ; i < tinfo.fin ; ++i) {
        clefsCandidatesTHREAD((tinfo.tree->fils)[i], tinfo.clef, tinfo.indice, newPile);
    }

    pileDelete(newPile); // TODO ca marche ??
    pthread_exit(NULL);
}

// t l'arbre possédant toutes les clefs
// clef un pointeur vers un tableau de chaine de caractères (est modifé)
// la taille de clef doit être egal à nombreClefsCandidates * tailleClefs
// pile doit avoir pour taille max tailleClefs
// indice doit être initialisé a 0
// indice est l'indice d'insertion dans *clef[indice] = pile->values
void clefsCandidatesNT(TreeNaire *t, unsigned char **clef, int *indice, Pile *p) {
    if (t -> value != '\0') { // le noeud parent ultime : celui tout en haut
        pilePush(p, (void *) &(t -> value));
    }

    if (t -> nbFils == 0) {
        pileCopyValueCHAR(p, &(clef[*indice]));
        ++(*indice);
    } else {
        int indFils = 0;

        while (indFils < (t -> nbFils)) {
            clefsCandidatesNT((t -> fils)[indFils], clef, indice, p);
            ++indFils;
        }
    }
    if (t -> value != '\0') {
        pilePop(p);
    }
    return;
}


void getClefsCandidatesTHREAD(Arbre *t, unsigned char **clef) {
    if (pthread_mutex_init(&mutexEcritureClef, NULL) != 0) {
        pError(NULL, "Erreur creation mutex", 3);
    }

    Pile *p = pileCreate(tailleClefs(t)); // faut la copier mister
    int ind = 0;
    clefsCandidatesTHREAD(t->premiersFils, clef, &ind, p);
    pileDelete(p);

    if (pthread_mutex_destroy(&mutexEcritureClef) != 0) {
        pError(NULL, "Erreur destruction mutex", 3);
    }
}


// t l'arbre possédant toutes les clefs
// clef un tableau de chaine de caractères (est modifé)
// input (par arbre): [[568], [1], [23]]
// output (dans clef): [[512], [513], [612], [513], [812], [813]]
void getClefsCandidatesNT(Arbre *t, unsigned char **clef) {
    Pile *p = pileCreate(tailleClefs(t));
    int ind = 0;
    clefsCandidatesNT(t->premiersFils, clef, &ind, p);
    pileDelete(p);
}

