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
int pError(void *ptr, char *msg, int exitStatus);

// retourne le contenu de file_in 
// et stocke dans sizeMessage la taille du fichier
char *ouvreEtLitFichier(char *file_in, off_t *sizeMessage);

// ecrit dans le fichier file_in le texte text
void ouvreEtEcritMsg(char *file_in, char *text, off_t len_text, bool cut);

uint32_t true_random(uint32_t value);

void freeTabs(void ***tabs, int nbElems);

char *format_number_with_thousands_separator(unsigned long number);

char *format_seconds_to_string(double timeInSeconds);

//-----------------------------------------------------------------
/*
    Functions' declarations
*/
void read_and_insert_words(const char *filename, dictionnary **dicoHash, int *sizeOfWords);
void add_word(dictionnary **dicoHash, const char *word);
dictionnary *find_word(dictionnary *dicoHash, const char *word);
void clear_table(dictionnary **dicoHash);

#endif
