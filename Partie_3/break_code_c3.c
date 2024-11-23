#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdbool.h>
#include <pthread.h>
#include "./crackage.h"
#include "../utilitaire/utiL.h"
#include "../srcHashMap/uthash.h"
#include "./break_code_c3.h"
#include "./break_code_c2.h"
#include "./break_code_c1.h"
#include "break_code_c2_c3.h"

int nbMotsPresents(char **mots, int nbMots, dictionnary *dico);
int getIndexInsertionC3(stC2_C3 *st);

//pthread_mutex_t *mutexScore;

//-----------------------------------------------------------------
/*
    Functions' declarations
*/
void read_and_insert_words(const char *filename, dictionnary **dicoHash);
void add_word(dictionnary **dicoHash, const char *word);
dictionnary *find_word(dictionnary *dicoHash, const char *word);
void clear_table(dictionnary **dicoHash);


//-----------------------------------------------------------------
/*
    main
*/
/*
int break_code_c3(char *file_in, char *dict_file_in, char *score_out, int keyLen, char *logFile) {
    (void) file_in, (void) dict_file_in, (void) score_out;

    dictionnary *dicoHash = NULL;

    if (pthread_mutex_init(&mutexScore, NULL) != 0) {
        pError(NULL, "Erreur creation mutex score break_code_c2", 2);
    }

    // Read words from the Scrabble dictionary and insert them into the hash table
    read_and_insert_words(dict_file_in, &dicoHash);

    off_t tailleMsg = 0;
    char *cryptedMsg = ouvreEtLitFichier(file_in, &tailleMsg);

    stC2_C3 *stC2C3 = initSTc2_c3(cryptedMsg, tailleMsg, tailleTabScore);

    appelClefsFinales(file_in, keyLen, (void *) stC2C3, functorC3, logFile);


    destroyStructC2_C3(&stC2C3);
    clear_table(&dicoHash);

    return 0;
}
*/

int traiteMsgClefC3(char *msg, int *nbMotsPrez, dictionnary *dico) {
    int nbMots = 0;
    char **mots = wordsArrayFromText(msg, strlen(msg), &nbMots);
    *nbMotsPrez = nbMotsPresents(mots, nbMots, dico);

    freeTabs((void **) mots, nbMots);

    return *nbMotsPrez;
}

int nbMotsPresents(char **mots, int nbMots, dictionnary *dico) {
    int nbMotsPrez = 0;
    for (int i = 0 ; i < nbMots ; ++i) {
        if (find_word(dico, mots[i])) {
            ++nbMotsPrez;
        }
    }

    return nbMotsPrez;
}

/*
    une structure st contenant 2 tableaux :
        un tableau 1 possédant les meilleurs scores (plus proche de 0 mieux c'est)
        un tableau 2 de clefs à l'indice correspondant au tableau 1
*/
void ajouteScoreC3(stC2_C3 *st, unsigned char *key, int ind) {
    if (ind < st -> tailleScoreTab) {
        for (int i = *(st -> nbScoreTabs) ; i > ind ; --i) {
            // décaler tout le tableau vers la droite
            // (copier tous les elements pour en inserer un nouveau)
            if (i != st -> tailleScoreTab) {
                (st -> tabMeilleurScoreC3)[i] = (st -> tabMeilleurScoreC3)[i - 1];
                strcpy((char *) (st -> tabKeysScoreC3)[i], (const char *) (st -> tabKeysScoreC3)[i - 1]);
            }
        }
        // insertion du nouvel element
        (st -> tabMeilleurScoreC3)[ind] = *(st -> nbMotsPrez);
        strcpy((char *) (st -> tabKeysScoreC3)[ind], (const char *) key);
    }
}

int getIndexInsertionC3(stC2_C3 *st) {
    return getIndexInsertionValueC3(st, *(st -> nbMotsPrez));
}

int getIndexInsertionValueC3(stC2_C3 *st, int value) {
    int ind = 0;
    while ((ind < *(st -> nbScoreTabs)) && ((st -> tabMeilleurScoreC3)[ind] > value)) {
        ++ind;
    }
    return ind;
}

//-----------------------------------------------------------------
/*
    Functions to transform a text to an array of words

    toto (and titi) go ->tothe market

    return [toto, and, titi, go, tothe, market]
*/
char **wordsArrayFromText(char *text, off_t lenText, int *nbMot) {
    int arrayLen = 100; // 50 words at the beginning
    char **wordsArray = malloc(sizeof(char *) * arrayLen);
    pError(wordsArray, "Erreur allocation memoire", 3);

    int indWord = 0;
    off_t indText = 0;
    *nbMot = 0; // il faut réassigner la valeur au cas ou !

    while (indText < lenText) {
        if (*nbMot == arrayLen) {
            arrayLen *= 2;
            wordsArray = realloc(wordsArray, sizeof(char *) * arrayLen);
            pError(wordsArray, "Erreur allocation memoire", 3);
        }
        wordsArray[indWord++] = nextWord(text, lenText, &indText);
        indNextWord(text, lenText, &indText);
        ++(*nbMot);
    }

    return wordsArray;
}

/*
    Retourne le prochain mot à partir du curseur indText
    Un mot est composé uniquement de caractères alphabétiques

    Return the next word from indText to the last alpha character
    indText should begin from an alpha character
*/
char *nextWord(char *text, off_t lenText, off_t *indText) {
    int lenWord = 50;
    char *word = malloc(sizeof(char) * lenWord);
    pError(word, "Erreur allocation mémoire", 3);
    int indWord = 0;
    char readChar;

    while (((*indText) < lenText) && isalpha(text[(*indText)])) {
        readChar = tolower(text[(*indText)]);
        if (indWord == lenWord) {
            lenWord *= 2;
            word = realloc(word, sizeof(char) * lenWord);
            pError(word, "Erreur allocation mémoire", 3);
        }
        word[indWord++] = readChar;
        ++(*indText);
    }
    if (indWord == lenWord) {
        word = realloc(word, sizeof(char) * (lenWord + 1)); // need the \0
        pError(word, "Erreur allocation mémoire", 3);
    }
    word[indWord] = '\0';

    return word;
}

/*
    Toto (va) au marché

    indText = 0 -> 0
    indText = 0 -> 6

    changes indText to where the next alpha character begins
    should start at a non alpha character
*/
void indNextWord(char *text, off_t lenText, off_t *indText) {
    while (((*indText) < lenText) && (!isalpha(text[*indText]))) {
        ++(*indText);
    }
    return;
}

//-----------------------------------------------------------------
/*
    Functions for the hash table

    -read_and_insert_words : reads a file (a dictionnary scrabble)
        and puts it into the hashtable
    -add_word : add a word to a hashtable
    -find_word : finds a word return true if found
    -clear_table : destroys the hashtable pointed by *dicoHash
*/
void read_and_insert_words(const char *filename, dictionnary **dicoHash) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    char line[50]; // Buffer to store each line from the file
    while (fgets(line, sizeof(line), file)) {
        // Remove the newline character if present
        line[strcspn(line, "\n")] = '\0';

        // Add the word to the hash table
        add_word(dicoHash, line);
    }

    fclose(file);
}

void add_word(dictionnary **dicoHash, const char *word) {
    dictionnary *entry;

    // Find the word in the hash table
    HASH_FIND_STR(*dicoHash, word, entry);
    if (entry == NULL) {
        // Allocate memory for the new entry
        entry = (dictionnary *) malloc(sizeof(dictionnary));
        pError(entry, "Erreur allocation memoire", 3);
        // Copy the word into the key field
        strcpy(entry->word, word);
        // Add the entry to the hash table
        HASH_ADD_STR(*dicoHash, word, entry);
    }
}

dictionnary *find_word(dictionnary *dicoHash, const char *word) {
    dictionnary *dico;
    HASH_FIND_STR(dicoHash, word, dico);
    return dico;
}

void clear_table(dictionnary **dicoHash) {
    dictionnary *entry, *tmp;
    HASH_ITER(hh, *dicoHash, entry, tmp) {
        HASH_DEL(*dicoHash, entry);
        free(entry);
    }
}
