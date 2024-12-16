#include <pthread.h>
#include "mutex.h"

// Définition de la variable
pthread_mutex_t MUTEX_TEST = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t MUTEX_ECRITURE_SCORE = PTHREAD_MUTEX_INITIALIZER;
