#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <math.h>
#include <string.h>
#include <stdbool.h>
#include "Tree.h"
#include "../Pile.h"
#include "../../utilitaire/utiL.h"
#include "../break_code_c1.h"
#include "../caracteresCandidatsIndexKey.h"

/*
    NTree.c ne peut pas fonctionner : 
    pourquoi tout séparer si au final c'est pour stocker les memes choses ?
    -> on ne divise pas les caractères candidats : 
    un noeud = une ligne de caracteres candidats.

    bon bah du coup l'amélioration immédiate qui suit
    c'est pourquoi passer par un arbre alors qu'on a deja les 
    caractères candidats
    => amélioration suivante = technique actuelle
    (l'idée en fait c'est d'avoir 3 tableaux : 
    un pour l'indice d'origine, l'autre pour l'indice courant 
    et un dernier tableau pour les indices de fin)
*/

unsigned char **clefFinalesTree(char *msgCode, unsigned long tailleMsgCode, int len_key, unsigned long *nbClefs) {
    unsigned char **carCandParIndice = caracteresCandidatsParIndice(msgCode, tailleMsgCode, len_key);

    Tree *t = createTree();
    for (int i = 0 ; i < len_key ; ++i) {
        t = addTree(t, carCandParIndice[i], ((int) strlen((const char *) carCandParIndice[i])) );
    }

    *nbClefs = getNbClef(t);
    printf("323 break code c1 Il y a %lu clefs\n", *nbClefs);

    unsigned char **clefs = malloc((*nbClefs) * sizeof(unsigned char *));
    pError(clefs, "Erreur création lot de clefs", 1);

    getClefsCandidates(t, clefs);

    freeDoubleArray(&carCandParIndice, len_key);
    deleteTree(t);

    return clefs;
}

// pour les threads
void *threadClefsCandidates(void *threadInfo);
pthread_mutex_t mutexEcritureClefs;

typedef struct {
    int debut;
    int fin;
    Noeud *node;
    Pile *p;
    unsigned char **clefs;
    unsigned long *indice;
} threadInfo;

typedef struct {
    unsigned char ***arr;
    unsigned long debut, fin;
} threadFreeInfo;

// fonction privée
Noeud *createNoeud(Noeud *pere, unsigned char *carCand, unsigned long nbCar);

// définition de la structure opaque Tree
struct _Tree {
    Noeud *sentinel; // les fils representent la derniere ligne des caracteres candidats
    // le pere represente la premiere ligne des caracteres candidats
    unsigned long nbClefs;
    unsigned long tailleClef;
};

struct _Noeud {
    unsigned char *carCandidats;
    unsigned int nbCar;
    Noeud *pere;
    Noeud *fils;
};

Tree *createTree(void) {
    Tree *t = malloc(sizeof(struct _Tree));
    pError((void *) t, "Erreur creation arbre", 2);

    unsigned char *endSon = malloc(2);
    endSon[0] = '\0';
    Noeud *sentinel = createNoeud(NULL, endSon, 0);
    sentinel -> pere = sentinel -> fils = sentinel;

    t -> sentinel = sentinel;
    t -> tailleClef = 0;
    t -> nbClefs = 1; // pour la multiplication

    return t;
}

void deleteTree(Tree *t) {
    Noeud *curNode = t->sentinel->fils;
    Noeud *nextNode = NULL;

    while (curNode != t -> sentinel) {
        nextNode = curNode -> fils;
        
        free((void *) curNode -> carCandidats);
        free((void *) curNode);

        curNode = nextNode;
    }
    free((void *) t->sentinel);
    free((void *) t);
}

Noeud *getFirstNoeud(Tree *t) {
    return t -> sentinel -> pere;
}

unsigned long getNbClef(Tree *t) {
    return t -> nbClefs;
}

unsigned long getTailleClef(Tree *t) {
    return t -> tailleClef;
}

bool treeEmpty(Tree *t) {
    return t->sentinel->pere == t->sentinel;
}


Tree *addTree(Tree *t, unsigned char *carCand, int nbCar) {
    pError((void *) t, "Il faut creer un arbre !", 2);

    unsigned char *carPourNoeud = malloc((nbCar * ((int) sizeof(unsigned char))) + 1);
    pError((void *) carPourNoeud, "Erreur creation tableau caracteres noeud", 2);
    strcpy((char *) carPourNoeud, (const char *) carCand);

    Noeud *newNode = createNoeud(t -> sentinel -> fils, carPourNoeud, (unsigned long) nbCar);

    if (t -> tailleClef == 0) {
        t -> sentinel -> pere = newNode;
    }
    t -> sentinel -> fils = newNode;

    ++(t -> tailleClef);
    t -> nbClefs *= (unsigned long) nbCar;

    return t;
}

Noeud *createNoeud(Noeud *pere, unsigned char *carCand, unsigned long nbCar) {
    Noeud *n = malloc(sizeof(struct _Noeud));
    pError((void *) n, "Erreur creation noeud arbre", 2);

    n -> pere = pere;
    n -> fils = NULL;
    n -> carCandidats = carCand;
    n -> nbCar = nbCar;

    if (pere) {
        n -> fils = pere -> fils;
        pere -> fils = n;
    }

    return n;
}

threadInfo createThreadInfoTree(unsigned char **clefs, unsigned long *indice, Noeud *node, Pile *pile, int debut, int fin) {
    threadInfo inf;
    inf.clefs = clefs;
    inf.debut = debut;
    inf.fin = fin;
    inf.indice = indice;
    inf.node = node;
    inf.p = pile;

    return inf;
}

void clefsCandidates(Noeud *n, Pile *p, unsigned char **clefs, unsigned long *indice) {
    pError((void *) n, "Noeud inexistant", 2);

    if (n -> nbCar == 0) {
        if (pthread_mutex_lock(&mutexEcritureClefs) != 0) {
            pError(NULL, "Erreur prise jeton mutex", 2);
        }
        
        pileCopyValueCHAR(p, &(clefs[*indice]));
        ++(*indice);

        if (pthread_mutex_unlock(&mutexEcritureClefs) != 0) {
            pError(NULL, "Erreur don jeton mutex", 2);
        }
    } else {
        int tailleSegment = (int) (n->nbCar / 8);
        //int tailleSegment = (int) sqrt(n->nbCar);
        int nbSegment = (int) (n->nbCar / tailleSegment);
        threadInfo info[nbSegment];
        pthread_t thid[nbSegment];
        int fin;
        
        for (int i = 0 ; i < nbSegment ; ++i) {
            fin = ((i + 1) * tailleSegment);
            if (i == (nbSegment - 1)) {
                fin = n->nbCar;
            }
            
            info[i] = createThreadInfoTree(clefs, indice, n, p, (i * tailleSegment), fin);

            pthread_create(&(thid[i]), NULL, threadClefsCandidates, (void *) &(info[i]));
        }
        for (int i = 0 ; i < nbSegment ; ++i) {
            pthread_join(thid[i], NULL);
        }
    }
    return;
}

void *threadClefsCandidates(void *inf) {
    threadInfo info = *((threadInfo *) inf);
    Pile *newPile = pileCopyCHAR(info.p);

    for (int i = info.debut ; i < info.fin ; ++i) {
        pilePush(newPile, (void *) &(info.node->carCandidats[i]));
        clefsCandidates(info.node -> fils, newPile, info.clefs, info.indice);
        pilePop(newPile);
    }
    pileDelete(newPile);

    pthread_exit(NULL);
}

void getClefsCandidates(Tree *t, unsigned char **clefs) {
    if (treeEmpty(t)) {
        pError(NULL, "L'arbre est vide !", 2);
    }
    if (pthread_mutex_init(&mutexEcritureClefs, NULL) != 0) {
        pError(NULL, "Erreur creation mutex", 2);
    }

    Pile *p = pileCreate(t->tailleClef);
    unsigned long ind = 0;
    clefsCandidates(t->sentinel->pere, p, clefs, &ind);

    pileDelete(p);

    if (pthread_mutex_destroy(&mutexEcritureClefs) != 0) {
        pError(NULL, "Erreur destruction mutex", 2);
    }
    return;
}

void *threadFreeSegment(void *info) {
    threadFreeInfo ti = *((threadFreeInfo *) info);

    for (unsigned long i = ti.debut ; i < ti.fin ; ++i) {
        if ((*(ti.arr))[i]) {
            free((void *) (*(ti.arr))[i]);
            (*(ti.arr))[i] = NULL;
        }
    }

    pthread_exit(NULL);
}

void freeDoubleArray(unsigned char ***arr, unsigned long len) {
    if (!(arr && *arr)) {
        return;
    }
    unsigned long nbSeg = (unsigned long) sqrt(len);
    unsigned long tailleSeg = len / nbSeg + (len % nbSeg != 0 ? 1 : 0);

    pthread_t *thid = malloc(nbSeg * sizeof(pthread_t));
    pError(thid, "Erreur creation tableau thread free", 1);
    threadFreeInfo *ti = malloc(nbSeg * sizeof(threadFreeInfo));
    pError(ti, "Erreur creation tableau informations pour threads free", 1);
    
    for (unsigned long i = 0 ; i < nbSeg ; ++i) {
        ti[i].arr = arr;
        ti[i].debut = i * tailleSeg;
        ti[i].fin = (i + 1) * tailleSeg > len ? len : (i + 1) * tailleSeg;
        pthread_create(&thid[i], NULL, threadFreeSegment, &ti[i]);
    }
    for (unsigned long i = 0 ; i < nbSeg ; ++i) {
        pthread_join(thid[i], NULL);
    }
    
    free((void *) *arr);
    *arr = NULL;

    free(thid);
    free(ti);
}

