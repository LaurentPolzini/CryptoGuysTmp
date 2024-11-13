#include <stdio.h>
#include <getopt.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
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
    
    //char nameFileIn[] = "msgClair.txt";
    // Générer une clé pour le test
    char cle[] = "1234";
    char nameFileOut[] = "../script/CRACK/tests/crypted_crack/1234_msg2.txt";
    //char nameFileOut[] = "msgCrypte.txt";
    char nameFileUncrypted[] = "msgDecrypte.txt";

    // Chiffrer le message
    //char *msg = encrypt_decrypt_xor(nameFileOut, cle, nameFileUncrypted);
    //char *msgChiffre = encrypt_decrypt_xor(nameFileIn, cle, nameFileOut);
    
    // Déchiffrer le message (en utilisant la même clé)
    char *msgDechiffre = encrypt_decrypt_xor(nameFileOut, cle, nameFileUncrypted);


    printf("La clé est %s\n", cle);
    //printf("Message après chiffrement : %s\n\n", msgChiffre);
    printf("Message après déchiffrement : %s\n\n", msgDechiffre);

    break_code_c1(nameFileOut, strlen(cle), cle);

    /*
    for (unsigned long i = 0 ; i < nbClefs ; ++i) {
        if (strstr((const char *) clefs[i], cle) != NULL) {
            printf("Trouvé ! : %s\n", clefs[i]);

            break;
        }
    }
    
    
    printf("Libere le tableau de clefs...\n");

    tpsDepart = time(NULL);

    freeDoubleArray(&clefs, nbClefs);

    tpsFin = time(NULL);
    printf("Temps effacement toutes les clefs : %f\n", difftime(tpsFin, tpsDepart));
    */
    return 0;
}

char* readFileToBuffer(const char *fileName, long *fileSize) {
    FILE *file = fopen(fileName, "r");
    if (!file) {
        fprintf(stderr, "Erreur : impossible d'ouvrir le fichier %s\n", fileName);
        return NULL;
    }

    // Déplacement du pointeur de fichier à la fin pour obtenir la taille du fichier
    fseek(file, 0, SEEK_END);
    long internFileSize = ftell(file);
    if (fileSize) {
        *fileSize = internFileSize;
    }
    rewind(file);  // Retour au début du fichier

    // Allocation de mémoire pour le tampon
    char *buffer = (char*)malloc((internFileSize + 1) * sizeof(char));
    if (!buffer) {
        fprintf(stderr, "Erreur : impossible d'allouer de la mémoire\n");
        fclose(file);
        return NULL;
    }

    // Lecture du fichier dans le tampon
    size_t bytesRead = fread(buffer, sizeof(char), internFileSize, file);
    buffer[bytesRead] = '\0'; // Ajout du caractère de fin de chaîne

    fclose(file);
    return buffer;
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
