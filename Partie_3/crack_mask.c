#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include "../Partie_1/chiffrement.h"
#include "../utilitaire/utiL.h"

// Fonction pour réaliser l'attaque sur la réutilisation d'un masque jetable
/*
    file_c1, file_c2 : des messages chiffrés
    file_m1 : file_c1 en clair
    file_m2 : le deuxieme message qui sera en clair
*/
void crack_mask(const char* file_c1, const char* file_c2, const char* file_m1, const char* file_m2) {
    off_t len_c1, len_c2, len_m1;
    
    // Charger les fichiers
    unsigned char* C1 = (unsigned char *) ouvreEtLitFichier((char *) file_c1, &len_c1);
    unsigned char* C2 = (unsigned char *) ouvreEtLitFichier((char *) file_c2, &len_c2);
    unsigned char* M1 = (unsigned char *) ouvreEtLitFichier((char *) file_m1, &len_m1);

    if (!C1 || !C2 || !M1) {
        fprintf(stderr, "Erreur lors du chargement des fichiers.\n");
        return;
    }

    // Vérifier que les longueurs des fichiers sont compatibles
    if (len_c1 != len_c2 || len_c1 != len_m1) {
        fprintf(stderr, "Erreur : Les fichiers doivent avoir la même taille.\n");
        free(C1); free(C2); free(M1);
        return;
    }

    // Allouer la mémoire pour le message clair résultant
    unsigned char* M2 = malloc(len_c1);
    if (!M2) {
        perror("Erreur d'allocation mémoire pour M2");
        free(C1); free(C2); free(M1);
        return;
    }

    // Calcul de M2 = C1 ^ C2 ^ M1
    for (off_t i = 0; i < len_c1; i++) {
        M2[i] = C1[i] ^ C2[i] ^ M1[i];
    }

    printf("Décryptage réussi : %s\nRésultat enregistré dans %s\n", M2, file_m2);

    // Écriture du résultat dans le fichier de sortie
    ouvreEtEcritMsg((char*) file_m2, (char*) M2, len_c1, true);

    // Libération de la mémoire
    free(C1);
    free(C2);
    free(M1);
    free(M2);
}
