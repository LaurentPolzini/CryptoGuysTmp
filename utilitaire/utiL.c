#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdbool.h>
#include <string.h>
#include <fcntl.h>
#include <math.h>
#include "./utiL.h"
#include "./uthash.h"

#define MAX_RANDOM ((1 << 31) - 1)

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
int pError(void *ptr, char *msg, int exitStatus) {
    if (!ptr) {
        perror(msg);
        exit(exitStatus);
		return 1;
    }
	return 0;
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
    pError(msgLu, "Erreur allocation memoire", 1);
    long curSizeBuff = sizeBuffer;

    ssize_t bytesRead = 0;
    ssize_t totalBytesRead = 0;
    while ((bytesRead = read(fdFile_In, msgLu + totalBytesRead, sizeBuffer)) > 0) {
        totalBytesRead += bytesRead;
        if (totalBytesRead + sizeBuffer > curSizeBuff) {
            curSizeBuff *= 2;
            char *temp = realloc(msgLu, curSizeBuff);
            if (!temp) {
                free(msgLu);
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

void ouvreEtEcritMsg_noCut(char *file_in, char *text, off_t len_text);
void ouvreEtEcritMsg_cut(char *file_in, char *text, off_t len_text);

void ouvreEtEcritMsg(char *file_in, char *text, off_t len_text, bool cut) {
    if (cut) {
        ouvreEtEcritMsg_cut(file_in, text, len_text);
    } else {
        ouvreEtEcritMsg_noCut(file_in, text, len_text);
    }
}

/*
    opens a file and put the text text into the file file_name
*/
void ouvreEtEcritMsg_noCut(char *file_in, char *text, off_t len_text) {
    int fdFile_In = open(file_in, O_CREAT | O_WRONLY | O_APPEND, 0644);
    if (fdFile_In == -1) {
        pError(NULL, "Erreur ouverture fichier d'entrée", 1);
    }
    char tmpWord[len_text + 2];
    memcpy(tmpWord, text, len_text);
    tmpWord[len_text] = '\n';

    if (write(fdFile_In, tmpWord, len_text + 1) == -1) {
        pError(NULL, "Erreur ecriture fichier (ouvreEtEcritMsg)", 1);
    }
    close(fdFile_In);
}

void ouvreEtEcritMsg_cut(char *file_in, char *text, off_t len_text) {
    int fdFile_In = open(file_in, O_CREAT | O_WRONLY | O_TRUNC, 0644);
    if (fdFile_In == -1) {
        pError(NULL, "Erreur ouverture fichier d'entrée", 1);
    }
    char tmpWord[len_text + 2];
    memcpy(tmpWord, text, len_text);
    tmpWord[len_text] = '\n';

    if (write(fdFile_In, tmpWord, len_text + 1) == -1) {
        pError(NULL, "Erreur ecriture fichier (ouvreEtEcritMsg)", 1);
    }
    close(fdFile_In);
}



/*
    true random number generator
*/
#if defined(__linux__)
unsigned int get_random_value(void) {
    int fd = open("/dev/urandom", O_RDONLY);
    if (fd == -1) {
        perror("Unable to open /dev/urandom");
        exit(1);
    }

    unsigned int random_value;
    ssize_t bytes_read = read(fd, &random_value, sizeof(random_value));
    if (bytes_read != sizeof(random_value)) {
        perror("Unable to read enough random data");
        exit(1);
    }

    close(fd);
    return random_value;
}
#endif

uint32_t true_random(uint32_t value) {
	#ifdef __APPLE__
	return arc4random_uniform(value);
	#else
	return get_random_value() % value;
	#endif
}

void freeTabs(void ***tabs, int nbElems) {
	if (*tabs) {
		for (int i = 0 ; i < nbElems ; ++i) {
			if ((*tabs)[i]) {
				free((*tabs)[i]);
			}
		}
		free(*tabs);
	}
}

/*
    transform 1000 to 1'000
    for readability for the number of keys
*/
char *format_number_with_thousands_separator(unsigned long number) {
    char temp[100];
    int len = snprintf(temp, sizeof(temp), "%lu", number);
    if (len < 0) {
        pError(NULL, "Erreur snprintf format_number_with_thousands_separator", 1);
    }

    int nbElem = len + (len - 1) / 3; 
    char *result = malloc(nbElem + 1);  
    if (!result) {
        pError(result, "Erreur allocation memoire", 1);  
    }

    int j = 0;
    
    for (int i = len - 1; i >= 0; i--) {
        result[j++] = temp[i];
        if ((len - i) % 3 == 0 && i != 0) {
            result[j++] = '\'';  
        }
    }

    result[j] = '\0';  

    for (int i = 0; i < j / 2; i++) {
        char temp_char = result[i];
        result[i] = result[j - i - 1];
        result[j - i - 1] = temp_char;
    }

    return result;
}


char *format_seconds_to_string(double timeInSeconds) {
    int hours = (int)(timeInSeconds / 3600);
    int minutes = (int) (fmod(timeInSeconds, 3600) / 60);
    int seconds = (int) fmod(timeInSeconds, 60);

    int tailleMsg = strlen("00 heures 05 minutes et 10 secondes") + 10;
    char *msg = malloc(tailleMsg);
    pError(msg, "Erreur allocation mémoire", 1);

    snprintf(msg, tailleMsg, "%d heures %d minutes et %d secondes", hours, minutes, seconds);

    return msg;
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
    int siz = 100; // aucun risque
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

void clear_table(dictionnary **dicoHash) {
    dictionnary *entry, *tmp;
    HASH_ITER(hh, *dicoHash, entry, tmp) {
		free(entry->word);
        HASH_DEL(*dicoHash, entry);
        free(entry);
    }
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
		pError(entry->word, "Erreur allocation memoire", 1);
		
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



