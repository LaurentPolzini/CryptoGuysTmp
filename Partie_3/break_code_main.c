#include <stdio.h>
#include "./crackage.h"
#include <getopt.h>
#include "../utilitaire/utiL.h"
#include <string.h>
#include <stdlib.h>
#include "./NTree.h"
#include "./Queue.h"
#include <stdbool.h>
#include "./break_code_c1.h"

void afficheManBreakCode(void);

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
    /*
    char *fileToCrack;
    char *method; // all or c1
    int keyLength;
    char *dict;
    char *logFile;

    int opt;


    if ( !argContainsHelp(argc, argv) ) {
        while ( (opt = getopt(argc, argv, ":i:m:k:dlh")) != -1 ) {
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
    }
    (void) fileToCrack, (void) method, (void) keyLength, (void) dict, (void) logFile;
    */
    (void) argc, (void) argv;
    
    int nbClefs = 0;
    unsigned char **test = clefsCandidatesFinales("512", 3, &nbClefs);
    
    freeDoubleArray(&test, nbClefs);

    return 0;
}


void afficheManBreakCode(void) {
    printf("Usage:\n");
    printf("./break_code [options]");

    printf("Mandatory Options :\n");
    printf("-i      The file containing the message to crack\n");
    printf("-m      The crack method (c1 or all)\n");
    printf("-k      The length of the key\n");
    printf("-d      The dictionnary of the language used to write  (only to be used if -m all)\n");

    printf("Optionnal Options :\n");
    printf("-l      The log file (if not specified, stdin)\n");
    printf("-h      This help message\n");

    return;
}