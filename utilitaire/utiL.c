#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdbool.h>
#include <string.h>
#include <fcntl.h>
#include "./utiL.h"
#include "./uthash.h"

/*
    to know if there is a -h in a command
*/
bool argContainsHelp(int argc, char* argv[]) {
    int i = 0;
    bool helpPresent = false;

    while ( i != argc && !helpPresent ) {
        helpPresent = ( strstr(argv[i], "-h") != NULL ); // not equal to -h
        helpPresent = ( helpPresent || (strstr(argv[i], "--help") != NULL) ); // not equal to --help
        ++i;
    }

    return helpPresent;
}

/*
    to use as perror but this way
        pError(NULL (or ptr), the message to show, the exit status)
*/
void pError(void *ptr, char *msg, int exitStatus) {
    if (!ptr) {
        perror(msg);
        exit(exitStatus);
    }
}

/*
    opens a file and reads the content
*/
char *ouvreEtLitFichier(char *file_in, off_t *sizeMessage) {
    int fdFile_In = open(file_in, O_RDONLY, 0644);
    if (fdFile_In == -1) {
        pError(NULL, "Erreur ouverture fichier d'entrée", 1);
    }
    long sizeBuffer = 512;
    char *msgLu = malloc(sizeBuffer);
    long curSizeBuff = sizeBuffer;

    ssize_t bytesRead = 0;
    ssize_t totalBytesRead = 0;
    while ((bytesRead = read(fdFile_In, msgLu + totalBytesRead, sizeBuffer)) > 0) {
        totalBytesRead += bytesRead;
        if (totalBytesRead + sizeBuffer > curSizeBuff) {
            curSizeBuff *= 2;
            char *temp = realloc(msgLu, curSizeBuff);
            if (!temp) {
                close(fdFile_In);
                pError(NULL, "Erreur allocation tableau temporaire de lecture du fichier", 1);
            }
            msgLu = temp;
        }        
    }
    msgLu[totalBytesRead] = '\0';
    if (sizeMessage) {
        *sizeMessage = (off_t) totalBytesRead;
    }
    
    close(fdFile_In);

    return msgLu;
}

/*
    opens a file and put the word word into the file file_name
*/
void ouvreEtEcritMsg(char *file_in, char *word, int lenWord) {
    int fdFile_In = open(file_in, O_WRONLY | O_APPEND | O_CREAT, 0644);
    if (fdFile_In == -1) {
        pError(NULL, "Erreur ouverture fichier d'entrée", 1);
    }
    char tmpWord[lenWord + 2];
    memcpy(tmpWord, word, lenWord);
    tmpWord[lenWord] = '\n';

    if (write(fdFile_In, tmpWord, lenWord + 1) == -1) {
        pError(NULL, "Erreur ecriture fichier (ouvreEtEcritMsg)", 1);
    }
    close(fdFile_In);
}

/*
    true random number generator
*/
uint32_t trueRandom(uint32_t value) {
    return arc4random_uniform(value);
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
void read_and_insert_words(const char *filename, dictionnary **dicoHash, int *sizeOfWords) {
    int siz = 50;
    if (sizeOfWords) {
        siz = *sizeOfWords;
    }
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    char line[siz]; // Buffer to store each line from the file
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
        pError(entry, "Erreur allocation memoire", 1);
        // Copy the word into the key field
        entry -> word = malloc(strlen(word) + 1);
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

