#define _GNU_SOURCE
#define POSIX_C_SOURCE 200112L
#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include "../utilitaire/utiL.h"
#include "chiffrement.h"

void afficher_aideP1(void);
void creer_fichier_cle(char *fileName, const char* key);

/*
int main(int argc, char* argv[]) {
    int opt;
    bool v_init_needed = false;
    bool key_needed = true;
    char *file_in = NULL;
    char *file_out = NULL;
    char *key = NULL;
    char *key_filename = NULL;
    char *methode = NULL;
    char *v_init = NULL;
    char *log_file = NULL;

    (void) log_file;

    // Gestion des options
    while ((opt = getopt(argc, argv, "i:o:k:f:m:v:l:h")) != -1) {
        switch (opt) {
            case 'i':
                file_in = optarg;
                break;
            case 'o':
                file_out = optarg;
                break;
            case 'k':
                key = optarg;
                creer_fichier_cle("key.txt", key);
                key = ouvreEtLitFichier("key.txt", NULL);
                break;
            case 'f':
                key = ouvreEtLitFichier(optarg, NULL);
                break;
            case 'm':
                methode = optarg;
                if (strcmp(methode, "cbc-crypt") == 0 || strcmp(methode, "cbc-uncrypt") == 0) {
                    v_init_needed = true;
                } else if (strcmp(methode, "mask-crypt") == 0) {
                    key_needed = false;
                }
                break;
            case 'v':
                if (v_init_needed) {
                    v_init = optarg;
                } else {
                    fprintf(stderr, "Erreur : l'option -v est uniquement nécessaire pour les méthodes cbc.\n");
                    exit(EXIT_FAILURE);
                }
                break;
            case 'l':
                log_file = optarg;
                break;
            case 'h':
                afficher_aideP1();
                return 0;
            default:
                fprintf(stderr, "Option invalide.\n");
                afficher_aideP1();
                exit(EXIT_FAILURE);
        }
    }

    // Vérification des arguments obligatoires
    // clef pas utilisée si methode mask
    if (!file_in || !file_out || !methode || (!key && key_needed)) {
        fprintf(stderr, "Erreur : les options -i, -o, -k ou -m sont manquantes.\n");
        afficher_aideP1();
        exit(EXIT_FAILURE);
    }

    // Appel de la fonction de chiffrement/déchiffrement
    appel_chiffrement(file_in, file_out, key, methode, v_init);

    // Libération de la clé si elle a été allouée
    if (key_filename) free(key);
    return 0;
}
*/

void afficher_aideP1(void) {
    printf("Usage:\n");
    printf("./sym_crypt -i <fichier_entrée> -o <fichier_sortie> -k <clé> [-f <fichier_clé>] -m <methode> [-v <vecteur_initialisation>] [-l <fichier_log>] [-h]\n");
    printf("Options:\n");
    printf("  -i\tFichier contenant le message en clair\n");
    printf("  -o\tFichier où sera écrit le message chiffré\n");
    printf("  -k\tClé de chiffrement (obligatoire sauf si -f est utilisé)\n");
    printf("  -f\tFichier contenant la clé\n");
    printf("  -m\tMéthode de chiffrement : xor, mask-crypt, mask-uncrypt, cbc-crypt, cbc-uncrypt\n");
    printf("  -v\tFichier vecteur d'initialisation (obligatoire pour cbc-crypt et cbc-uncrypt)\n");
    printf("  -l\tFichier de log (optionnel)\n");
    printf("  -h\tAffiche cette aide\n");
}

void creer_fichier_cle(char *fileName, const char* key) {
    FILE *fichier = fopen(fileName, "wb");
    if (!fichier) {
        perror("Erreur lors de l'ouverture ou de la création du fichier");
        return;
    }

    // Écrire la clé dans le fichier
    fwrite(key, 1, strlen(key), fichier); // Écriture de la clé sans saut de ligne

    // Fermer le fichier
    fclose(fichier);
}

void appel_chiffrement(char* namefile_in, char* namefile_out, char* key, char* methode, char* v_init) {
    printf("Debut du chiffrement ou déchiffrement du message :\n %s\n\n avec la clef %s\n", ouvreEtLitFichier(namefile_in, NULL), key);
    if (strcmp(methode, "xor") == 0) {
        pError(key, "La clef est vide !", 1);
        // Chiffrement XOR
        encrypt_decrypt_xor(namefile_in, key, namefile_out);
    } else if (strcmp(methode, "mask-crypt") == 0) {
        // Chiffrement avec masque (clef pas obligatoire)
        encrypt_mask(namefile_in, key, namefile_out);
    } else if (strcmp(methode, "mask-uncrypt") == 0) {
        // dechiffrement du mask (clef obligatoire)
        pError(key, "La clef est vide !", 1);
        decrypt_mask(namefile_in, key, namefile_out);
    } else if (strcmp(methode, "cbc-crypt") == 0) {
        // Chiffrement CBC
        if (!v_init) {
            fprintf(stderr, "Erreur : le vecteur d'initialisation est requis pour cbc-crypt.\n");
            exit(EXIT_FAILURE);
        }
        encrypt_cbc(namefile_in, "key.txt", namefile_out, v_init);
    } else if (strcmp(methode, "cbc-uncrypt") == 0) {
        // Déchiffrement CBC
        if (!v_init) {
            fprintf(stderr, "Erreur : le vecteur d'initialisation est requis pour cbc-uncrypt.\n");
            exit(EXIT_FAILURE);
        }
        decrypt_cbc(namefile_in, "key.txt", namefile_out, v_init);
    } else {
        fprintf(stderr, "Erreur : méthode de chiffrement invalide.\n");
        exit(EXIT_FAILURE);
    }
}
