#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include "../Partie_1/mask.h"
#include "../Partie_1/xor.h"

// Fonction pour lire un fichier binaire et charger son contenu dans un tableau
unsigned char* read_file(const char* filename, size_t* length) {
    FILE *file = fopen(filename, "rb");
    if (!file) {
        perror("Erreur d'ouverture du fichier");
        return NULL;
    }
    
    fseek(file, 0, SEEK_END);
    *length = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    unsigned char* buffer = malloc(*length);
    if (!buffer) {
        perror("Erreur d'allocation mémoire");
        fclose(file);
        return NULL;
    }
    
    fread(buffer, 1, *length, file);
    fclose(file);
    return buffer;
}

// Fonction pour écrire le contenu d'un tableau dans un fichier binaire
void write_file(const char* filename, unsigned char* data, size_t length) {
    FILE *file = fopen(filename, "wb");
    if (!file) {
        perror("Erreur d'ouverture du fichier de sortie");
        return;
    }
    
    fwrite(data, 1, length, file);
    fclose(file);
}

// Fonction pour réaliser l'attaque sur la réutilisation d'un masque jetable
void crack_mask(const char* file_c1, const char* file_c2, const char* file_m1, const char* file_m2) {
    size_t len_c1, len_c2, len_m1;
    
    // Charger les fichiers
    unsigned char* C1 = read_file(file_c1, &len_c1);
    unsigned char* C2 = read_file(file_c2, &len_c2);
    unsigned char* M1 = read_file(file_m1, &len_m1);

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
    for (size_t i = 0; i < len_c1; i++) {
        M2[i] = C1[i] ^ C2[i] ^ M1[i];
    }

    // Écriture du résultat dans le fichier de sortie
    write_file(file_m2, M2, len_c1);

    // Libération de la mémoire
    free(C1);
    free(C2);
    free(M1);
    free(M2);

    printf("Décryptage réussi. Résultat enregistré dans %s\n", file_m2);
}
