#include "./Queue.h"
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>

// nbElem le nombre d'element actuellement dans la queue
// nbElemMax la capacité maximale de la queue
// queue la file de caractères
struct _Queue {
    unsigned int nbElem;
    unsigned int nbElemMax;
    unsigned char *queue;
};

// tailleMax la capacité de la queue
Queue *queueCreate(int tailleMax) {
    Queue *q = malloc(sizeof(Queue));
    if (!q) {
        perror("Erreur création queue");
        exit(1);
    }
    q -> nbElem = 0;
    
    // tailleMax + 1 car il faut \0
    unsigned char *elems = malloc(tailleMax + 1);
    if (!elems) {
        perror("Erreur allocation de la taille d'éléments de la queue");
        exit(1);
    }
    elems[0] = '\0';
    q -> queue = elems;
    q -> nbElemMax = tailleMax;

    return q;
}

int queueTaille(Queue *q) {
    assert(q);
    return q -> nbElem;
}

Queue *queuePush(Queue *q, unsigned char c) {
    if ((q -> nbElem) < (q -> nbElemMax)) {
        (q -> queue)[q -> nbElem] = c;
        (q -> queue)[++(q -> nbElem)] = '\0';
    }
    return q;
}

Queue *queuePop(Queue *q) {
    if (!queueEmpty(q)) {
        (q -> queue)[--(q -> nbElem)] = '\0';
    }
    return q;
}

void queueDelete(Queue *q) {
    assert(q != NULL);
    free((void *) q -> queue);
    q -> queue = NULL;
    free((void *) q);
    q = NULL;
}

bool queueEmpty(Queue *q) {
    assert(q != NULL);
    return q -> nbElem == 0;
}

// retourne le caractère d'indice valueAt - 1
unsigned char queueValueAt(Queue *q, unsigned int valueAt) {
    assert(q->nbElem >= valueAt && valueAt >= 1);
    return (q->queue)[valueAt - 1];
}

// retourne les valeurs de la queue
// retourne par exemple [510\0]
unsigned char *queueValue(Queue *q) {
    return q->queue;
}

// copie la clef dans *destination
// queueCopy alloue une zone mémoire mise dans *dest
void queueCopy(Queue *q, unsigned char **dest) {
    unsigned int taille = queueTaille(q) + 1;
    unsigned char *chaineFinale = malloc(taille);

    for (unsigned int i = 0 ; i < taille ; ++i) {
        chaineFinale[i] = queueValue(q)[i];
    }
    *dest = chaineFinale;
}
