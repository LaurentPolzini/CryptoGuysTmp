#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <math.h>
#include "../utilitaire/utiL.h"
#include "./dh_gen_group.h"
#include "./Code_C/dh_prime.h"

void afficheMan_dh_gen_group(void);

/*
    Programme principal pour la génération de clés pour la méthode Dillie-Hellman

    Options :
        -o Fichier où écrire les résultats de la génération
        -h affiche un manuel d'utilisation, annule l'option -o si présente
*/
/*
int main(int argc, char *argv[]) {
    
    char *fileResults = NULL;
    int opt;

    if ( !argContainsHelp(argc, argv) ) {
        while ( (opt = getopt(argc, argv, ":o:h")) != -1 ) {
            switch (opt) {
                case 'o':
                    fileResults = optarg;
                    break;
                
                case ':':
                    fprintf(stderr, "Option expected a value : %d", opt);
                    exit(2);
                
                case '?':
                    fprintf(stderr, "Unkown option : %d", opt);
                    exit(2);
            }
        }
        generateSophieGermain(fileResults);

    } else {
        afficheMan_dh_gen_group();
    }
    
    return 0;
}
*/

/*
    generates the parameter (g and p) for the exponential
    writes g,p in the file file_out

        attention a ne pas depasser la limite des long
        etant de 2^16 (décrit ici https://www.labri.fr/perso/betrema/deug/poly/exp-rapide.html)
*/
void generateSophieGermain(char *file_out) {
    //ouverture du fichier file_out
	FILE *f_out;
    if (file_out) {
        f_out = fopen(file_out, "w");
        if (!f_out) {
            pError(NULL, "Erreur lors de l'ouverture du fichier (generateSophieGermain)", 1);
        }
    } else {
        f_out = stdout;
    }
     
    // Définir les bornes
    long lowerBound = int_pow(2, 15);  // 32768
    long upperBound = int_pow(2, 16);  // 65536
    
    generate_shared_key(lowerBound, upperBound, f_out);

    //fermeture du fichier
    fclose(f_out);
}

void afficheMan_dh_gen_group(void) {
    printf("Usage:\n");
    printf("./dh_gen_group [options]");

    printf("Mandatory Option :\n");
    printf("-o      The file to write the generator (g) and the primary number (p)\n");

    printf("Optionnal Option :\n");
    printf("-h      This help message\n");

    return;
}
