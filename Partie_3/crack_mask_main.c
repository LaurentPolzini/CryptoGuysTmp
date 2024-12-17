#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include "crackage.h"


void afficheAide_crack_mask(void);

int main(int argc, char *argv[]) {
    char* file_c1 = NULL;
    char* file_c2 = NULL;
    char* file_m1 = NULL;
    char* file_m2 = NULL;
    
    int opt;

    while ((opt = getopt(argc, argv, "i:o:c:l:h")) != -1) {
        switch(opt) {
            case 'i':
                file_c1 = optarg;
                break;
            case 'o':
                file_m2 = optarg;
                break;
            case 'c':
                file_c2 = optarg;
                break;
            case 'l':
                file_m1 = optarg;
                break;
            case 'h':
                afficheAide_crack_mask();
                return 0;
            case ':':
                fprintf(stderr, "Option expected a value : %d\n", opt);
                exit(2);
            default:
                fprintf(stderr, "Option n°%s invalide. Utilisez --help (-h) pour voir les commandes disponibles.\n", optarg);
                return EXIT_FAILURE;
        }
    }
    if (!(file_c1 && file_c2 && file_m1 && file_m2)) {
        fprintf(stderr, "Il faut le fichier crypté 1 (-i), crypté 2 (-c), clair 1 (-l) et clair 2 (-o)\n");
        exit(2);
    }
    crack_mask(file_c1, file_c2, file_m1, file_m2);

    return 0;
}

void afficheAide_crack_mask(void) {
    printf("\nUsage:\n");
    printf("./crack_mask [options]\n");

    printf("Mandatory Options :\n");
    printf("-i\tThe file containing the first crypted message\n");
    printf("-o\tThe file where the second clear message will be written\n");
    printf("-c\tThe file containing the second crypted message\n");
    printf("-l\tThe file containing the first clear message\n");

    return;
}
