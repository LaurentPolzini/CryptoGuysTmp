#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <fcntl.h>
#include <time.h>
#include "../utilitaire/utiL.h"
#include "./crackage.h"
#include "./Pile.h"
#include "./break_code_c1.h"
#include "../Partie_1/xor.h"

void afficheManBreakCode(void);
char* readFileToBuffer(const char *fileName, long *fileSize);

/*
    Programme principal pour le crackage d'une clef menant au déchiffrage d'un texte crypté

    Options:
        (Obligatoire) -i Fichier à cracker
        (Obligatoire) -m Méthode de crackage (c1 ou all)
        (Obligatoire) -k La longueur de la clef (peut être la longueur maximale, donc clef plus petite)
        (Obligatoire) -d Le dictionnaire de la langue du message crypté
        -l Le fichier de log
        -h affiche un manuel d'utilisation, annihile toutes autres options
*/


int main(int argc, char *argv[]) {
    char *fileToCrack = NULL;
    char *method = NULL; // all or c1
    int keyLength = 0;
    char *dict = NULL;
    char *logFile = NULL;

    int opt;

    if ( !argContainsHelp(argc, argv) ) {
        while ( (opt = getopt(argc, argv, ":i:m:k:d:l:")) != -1 ) {
            switch (opt) {
                case 'i':
                    fileToCrack = optarg;
                    break;
                
                case 'm':
                    method = optarg;
                    break;
                
                case 'k':
                    keyLength = atoi(optarg);
                    break;

                case 'd':
                    dict = optarg;
                    break;

                case 'l':
                    logFile = optarg;
                    break;

                case ':':
                    fprintf(stderr, "Option expected a value : %d", opt);
                    exit(2);
                
                case '?':
                    fprintf(stderr, "Unkown option : %d", opt);
                    exit(2);
            }
        }
    } else {
        afficheManBreakCode();
        return 0;
    }

    if (method == NULL || fileToCrack == NULL) {
        fprintf(stderr, "Error: required arguments -i (file to crack) and -m (method) are missing\n");
        exit(1);
    }
    if (strstr(method, "c1") != NULL) {
        break_code_c1(fileToCrack, keyLength, logFile);
    } else if (strcmp(method, "all") == 0) {
        if (dict != NULL) {
            break_code_c2(fileToCrack, dict, NULL, keyLength, logFile);
            break_code_c3(fileToCrack, dict, logFile);
        } else {
            fprintf(stderr, "Error: dictionary is required for method 'all'\n");
            exit(1);
        }
    } else {
        fprintf(stderr, "Error: unknown method '%s'\n", method);
        exit(1);
    }
    
    return 0;
}

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
    *sizeMessage = (off_t) totalBytesRead;
    close(fdFile_In);

    return msgLu;
}


void afficheManBreakCode(void) {
    printf("\nUsage:\n");
    printf("./break_code [options]\n");

    printf("Mandatory Options :\n");
    printf("-i\tThe file containing the message to crack\n");
    printf("-m\tThe crack method (c1 or all)\n");
    printf("-k\tThe length of the key\n");
    printf("-d\tThe dictionnary of the language used to write (only to be used if -m all)\n");

    printf("Optionnal Options :\n");
    printf("-l\tThe log file (if not specified, stdin)\n");
    printf("-h\tThis help message\n");

    return;
}
