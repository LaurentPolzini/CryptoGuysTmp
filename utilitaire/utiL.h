#ifndef __UTIL_H__
#define __UTIL_H__
#include <stdbool.h>
#include <sys/types.h>
#include "./uthash.h"

// Hashmap for word dictionnary
typedef struct dictionnary {
    char *word; // value associated with the key
    UT_hash_handle hh;
} dictionnary;

// pour savoir si il y a -h ou --help dans les arguments
bool argContainsHelp(int argc, char* argv[]);

// si ptr est NULL alors perror(msg) exit(exitStatus)
void pError(void *ptr, char *msg, int exitStatus);

// retourne le contenu de file_in 
// et stocke dans sizeMessage la taille du fichier
char *ouvreEtLitFichier(char *file_in, off_t *sizeMessage);

// ecrit dans le fichier file_in le mot word
void ouvreEtEcritMsg(char *file_in, char *word, int lenWord);

uint32_t trueRandom(uint32_t value);

//-----------------------------------------------------------------
/*
    Functions' declarations
*/
void read_and_insert_words(const char *filename, dictionnary **dicoHash, int *sizeOfWords);
void add_word(dictionnary **dicoHash, const char *word);
dictionnary *find_word(dictionnary *dicoHash, const char *word);
void clear_table(dictionnary **dicoHash);

#endif
