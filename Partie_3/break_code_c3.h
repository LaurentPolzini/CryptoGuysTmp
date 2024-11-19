#ifndef __BREAK_CODE_C3_H__
#define __BREAK_CODE_C3_H__

#include <unistd.h>
#include "../srcHashMap/uthash.h"
#include "./break_code_c2_c3.h"


// Hashmap for the scrabble's dictionnary
typedef struct dictionnary {
    char word[50]; // value associated with the key
    UT_hash_handle hh;
} dictionnary;

void ajouteScoreC3(stC2_C3 *st, unsigned char *key);
void traiteMsgClefC3(char *msg, int *nbMotsPrez, dictionnary *dico);

char **wordsArrayFromText(char *text, off_t lenText, int *nbMot);

char *nextWord(char *text, off_t lenText, off_t *indText);

void indNextWord(char *text, off_t lenText, off_t *indText);

void read_and_insert_words(const char *filename, dictionnary **dicoHash);
dictionnary *find_word(dictionnary *dicoHash, const char *word);
void clear_table(dictionnary **dicoHash);

#endif
