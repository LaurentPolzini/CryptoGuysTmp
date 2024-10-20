#include <stdio.h>
#include "./dh_gen_group.h"
#include <getopt.h>
#include "../utilitaire/utiL.h"
#include <string.h>
#include "../Code_C/dh_prime.h"

void afficheMan_dh_gen_group();

/*
    Programme principal pour la génération de clés pour la méthode Dillie-Hellman

    Options :
        -o Fichier où écrire les résultats de la génération
        -h affiche un manuel d'utilisation, annule l'option -o si présente
*/

int main(int argc, char *argv[]) {
    
    char *fileResults;
    int opt;

    if ( !argContainsHelp(argc, argv) ) {
        while ( (opt = getopt(argc, argv, ":oh")) != -1 ) {
            switch (opt) {
                case 'o':
                    fileResults = optarg;
                    break;
                
                case ':':
                    perror(strcat("Option expected a value : ", opt));
                    exit(2);
                
                case '?':
                    perror(strcat("Unknown option", optopt));
                    exit(2);
            }
        }
    } else {
        afficheMan_dh_gen_group();
    }
    
    
    
    return 0;
}

gen_group generateSophieGermain(char *file_out) {
    return;
}

void afficheMan_dh_gen_group() {
    printf("Usage:\n");
    printf("./dh_gen_group [options]");

    printf("Mandatory Option :\n");
    printf("-o      The file to write the generator (g) and the primary number (p)\n");

    printf("Optionnal Option :\n");
    printf("-h      This help message\n");

    return;
}