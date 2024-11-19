#ifndef __UTIL_H__
#define __UTIL_H__
#include <stdbool.h>

// pour savoir si il y a -h ou --help dans les arguments
bool argContainsHelp(int argc, char* argv[]);

// si ptr est NULL alors perror(msg) exit(exitStatus)
void pError(void *ptr, char *msg, int exitStatus);

// retourne le contenu de file_in 
// et stocke dans sizeMessage la taille du fichier
char *ouvreEtLitFichier(char *file_in, off_t *sizeMessage);

#endif
