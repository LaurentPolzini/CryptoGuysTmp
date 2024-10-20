#ifndef __QUEUE_H__
#define __QUEUE_H__

#include <stdbool.h>

typedef struct _Queue Queue;
typedef Queue *ptrQueue;

// crée une queue de taille tailleMax
//  -> la taille reelle est tailleMax + 1
// pour le \0 final
Queue *queueCreate(int tailleMax);

// retourne le nombre d'element dans la file q
// [512\0] est de taille 3
int queueTaille(Queue *q);

// ajoute un element a la queue
// [51\0] -> [512\0]
Queue *queuePush(Queue *q, unsigned char c);

// retire le dernier element ajouté
// [512\0] -> [51\0]
Queue *queuePop(Queue *q);

// delete la queue p
void queueDelete(Queue *q);

// retourne si la queue est vide (0 element)
bool queueEmpty(Queue *q);

// si la queue vaut [510] et valueAt 1
// renvoie 5
unsigned char queueValueAt(Queue *q, unsigned int valueAt);

// retourne les valeurs de la queue
// retourne par exemple [510\0]
unsigned char *queueValue(Queue *q);

// copy la chaine contenue dans q a destination de dest
void queueCopy(Queue *q, unsigned char **dest);

#endif
