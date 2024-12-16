#include "mutex.h"
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>
#include <stdlib.h>
#include <stdbool.h>
#include <fcntl.h>
#include <time.h>
#include "../utilitaire/utiL.h"
#include "./crackage.h"
#include "./Pile.h"
#include "./break_code_c1.h"
#include "./break_code_c2.h"
#include "./break_code_c3.h"
#include "./break_code_c2_c3.h"
#include "../Partie_1/chiffrement.h"
#include "tests_crackage.h"

extern float stat_thFr[26];
extern float stat_thEn[26];

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
    char *fileToCrack = NULL;
    char *method = NULL; // all or c1
    int keyLength = 0;
    char *dict = NULL;
    char *logFile = NULL;
    char *scoreOut = NULL;
    float *stat = stat_thFr;

    int opt;
    
    if ( !argContainsHelp(argc, argv) ) {
        while ( (opt = getopt(argc, argv, ":i:m:k:d:l:s:t:e")) != -1 ) {
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

                case 's':
                    scoreOut = optarg;
                    break;

                case 't':
                    if (strstr(optarg, "english")) {
                        stat = stat_thEn;
                        printf("\nStatistiques de fréquences de lettres anglaises utilisées\n\n");
                    } else {
                        printf("\nStatistiques de fréquences de lettres françaises utilisées\n\n");
                    }
                    break;
                
                case 'e':
                    return appel_serie_tests();
                case ':':
                    fprintf(stderr, "Option expected a value : %d\n", opt);
                    exit(2);
                
                default:
                    fprintf(stderr, "Option n°%d invalide. Utilisez --help (-h) pour voir les commandes disponibles.\n", opt);
                    return EXIT_FAILURE;
            }
        }
    } else {
        afficheManBreakCode();
        return 0;
    }

    if (method == NULL || fileToCrack == NULL || keyLength == 0) {
        fprintf(stderr, "Error: required arguments -i (file to crack) or -m (method) are missing -k (length of the key) cannot be 0 !\n");
        exit(1);
    }
    if (strstr(method, "c1")) {
        break_code_c1(fileToCrack, keyLength, logFile);
    } else if (strstr(method, "c2")) {
        break_code_c2(fileToCrack, stat, scoreOut, keyLength, logFile);
    } else if (strstr(method, "c3") || strstr(method, "all")) {
        if (!dict) {
            fprintf(stderr, "Error: dictionary is required for method 'all' or 'c3'\n");
            exit(1);
        }
        if (strstr(method, "c3")) {
            break_code_c3(fileToCrack, dict, scoreOut, keyLength, logFile);
        } else {
            break_code_all_max_len(fileToCrack, dict, scoreOut, keyLength, logFile);
        }
    } else {
        fprintf(stderr, "Error: unknown method '%s'\n", method);
        exit(1);
    }
    
    return 0;
}

void appel_crackage(char *method, char *in, int len_key, char *dico, char *logName, char *scoresName, char *crypte2, char *clair1, char *out_clair2) {
    if (strstr(method, "freq")) {
        pError(dico, "Le dictionnaire doit être entré !", 1);
        break_code_all_max_len(in, dico, scoresName, len_key, logName);
    } else {
        pError(in, "Il manque le premier fichier crypté !", 1);
        pError(crypte2, "Il manque le second fichier crypté !", 1);
        pError(clair1, "Il manque le premier fichier en clair !", 1);
        pError(out_clair2, "Il manque le fichier de sortie comportant le second message en clair", 1);
        crack_mask(in, crypte2, clair1, out_clair2);
    }
}


void afficheManBreakCode(void) {
    printf("\nUsage:\n");
    printf("./break_code [options]\n");

    printf("Mandatory Options :\n");
    printf("-i\tThe file containing the message to crack\n");
    printf("-m\tThe crack method (c1, c2, c3 or all)\n");
    printf("-k\tThe length of the key\n");
    printf("-d\tThe dictionnary of the language used to write (only to be used if -m all or c3)\n");
    printf("-t\tThe target language used to write (english (french is default)) (only to be used if -m c2)\n");

    printf("Optionnal Options :\n");
    printf("-l\tThe log file (if not specified, stdout)\n");
    printf("-s\tThe file for the keys' scores\n");
    printf("-h\tThis help message\n");
    printf("-e\tThe crack tests launching\n");

    return;
}
