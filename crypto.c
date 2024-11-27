#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include "./Partie_1/xor.h"
#include "./Partie_3/crackage.h"

// Prototypes
void list_keys(void);
void genkey(int longueur, char *key, bool mask);
void del_key(const char *key);
void afficher_aidePR(void);

int main(int argc, char *argv[]) {
    int opt;
    char *commande = NULL;
    char *file_in = NULL;
    char *file_out = NULL;
    char *key = NULL;
    char *method = NULL;
    char *v_init = NULL;
    char *log_file = NULL;
    char *dico = NULL;
    int key_length = 0;

    (void) opt, (void) commande, (void) file_in, (void) file_out;
    (void) key, (void) method, (void) v_init, (void) log_file, (void) key_length;

    // Détection de la commande
    static struct option long_options[] = {
        {"command", required_argument, 0, 'c'},  // Commande principale
        {"in", required_argument, 0, 'i'},       // Fichier d'entrée
        {"out", required_argument, 0, 'o'},      // Fichier de sortie
        {"key", required_argument, 0, 'k'},      // Clé ou nom de clé
        {"length", required_argument, 0, 'l'},   // Longueur de la clé
        {"method", required_argument, 0, 'm'},   // Méthode de chiffrement
        {"init", required_argument, 0, 'v'},     // Vecteur d’initialisation
        {"log", required_argument, 0, 'g'},      // Fichier de log
        {"dico", required_argument, 0, 'd'}, // le dictionnaire de mots pour le crackage
        {"help", no_argument, 0, 'h'},           // Aide
        {0, 0, 0, 0}
    };

    // Traitement des options avec getopt_long
    while ((opt = getopt_long(argc, argv, "c:i:o:k:l:m:v:g:d:h", long_options, NULL)) != -1) {
        switch (opt) {
            case 'c': // Commande
                commande = optarg;
                break;
            case 'i': // Fichier d'entrée
                file_in = optarg;
                break;
            case 'o': // Fichier de sortie
                file_out = optarg;
                break;
            case 'k': // Clé
                key = optarg;
                break;
            case 'l': // Longueur de clé
                key_length = atoi(optarg);
                break;
            case 'g': // Log file
                log_file = optarg;
                break;
            case 'd':
                dico = optarg;
                break;
            case 'h': // Aide
                afficher_aidePR();
                return 0;
            default:
                fprintf(stderr, "Option invalide. Utilisez --help pour voir les commandes disponibles.\n");
                return EXIT_FAILURE;
        }
    }

    // Gestion des commandes
    if (commande == NULL) {
        fprintf(stderr, "Erreur : aucune commande spécifiée.\n");
        afficher_aidePR();
        return EXIT_FAILURE;
    }

    if (strcmp(commande, "list-keys") == 0) {
        list_keys();
    } else if (strcmp(commande, "gen-key") == 0) {
        if (key_length > 0) {
            char generated_key[256] = {0};
            // si la methode demandée est mask, il faut s'assurer que la clef n'existe pas
            genkey(key_length, generated_key, (strcmp(method, "mask") == 0));
            printf("Clé générée : %s\n", generated_key);
        } else {
            fprintf(stderr, "Erreur : spécifiez une longueur de clé avec --length.\n");
        }
    } else if (strcmp(commande, "del-key") == 0) {
        if (key != NULL) {
            del_key(key);
        } else {
            fprintf(stderr, "Erreur : spécifiez une clé avec --key.\n");
        }
    } else if ( (strcmp(commande, "encrypt") == 0) || (strcmp(commande, "decrypt") == 0) ) {
        appel_chiffrement(file_in, file_out, key, method, v_init);
    } else if (strcmp(commande, "crack") == 0) {
        break_code_c2_c3(file_in, dico, file_out, key_length, log_file);
    } else if (strcmp(commande, "crack-mask") == 0) {
        //crack_mask(const char* file_c1, const char* file_c2, const char* file_m1, const char* file_m2)
    } else {
        fprintf(stderr, "Commande inconnue : %s\n", commande);
        afficher_aidePR();
    }

    return 0;
}

void list_keys(void) {
    FILE *file = fopen("keys.txt", "r");
    if (!file) {
        perror("Erreur lors de l'ouverture du fichier de clés");
        return;
    }

    char line[1024];
    while (fgets(line, sizeof(line), file)) {
        printf("%s", line);
    }

    fclose(file);
}

void genkey(int longueur, char *key, bool mask) {
    gen_key(longueur, key, mask); // Appel à la fonction de génération
    FILE *file = fopen("keys.txt", "a");
    if (!file) {
        perror("Erreur lors de l'ouverture du fichier de clés");
        return;
    }

    fprintf(file, "%s\n", key); // Écriture dans le fichier
    fclose(file);
}

void del_key(const char *key) {
    FILE *file = fopen("keys.txt", "r");
    if (!file) {
        perror("Erreur lors de l'ouverture du fichier de clés");
        return;
    }

    FILE *temp = fopen("temp.txt", "w");
    if (!temp) {
        perror("Erreur lors de l'ouverture du fichier temporaire");
        fclose(file);
        return;
    }

    char line[1024];
    int found = 0;
    while (fgets(line, sizeof(line), file)) {
        if (strstr(line, key) == NULL) {
            fprintf(temp, "%s", line);
        } else {
            found = 1;
        }
    }

    fclose(file);
    fclose(temp);

    if (!found) {
        fprintf(stderr, "Erreur : la clé spécifiée n'a pas été trouvée.\n");
        remove("temp.txt");
        return;
    }

    if (remove("keys.txt") != 0) {
        perror("Erreur lors de la suppression du fichier de clés");
        return;
    }

    if (rename("temp.txt", "keys.txt") != 0) {
        perror("Erreur lors du renommage du fichier temporaire");
        return;
    }

    printf("Clé supprimée avec succès.\n");
}

void afficher_aidePR(void) {
    printf("Usage : programme [OPTIONS]\n");
    printf("Options disponibles :\n");
    printf("  --command (-cmd) <cmd> : Commande à exécuter (list-keys, gen-key, del-key, encrypt, decrypt, crack ou crack-mask.)\n");
    printf("  --in (-i) <file> : Fichier d'entrée\n");
    printf("  --out (-o) <file> : Fichier de sortie\n");
    printf("  --key (-k) <key> : Clé de chiffrement/déchiffrement\n");
    printf("  --length (-lk) <n> : Longueur de clé à générer\n");
    printf("  --log (-l) <file> : Le fichier de log\n");
    printf("  --dico (-d) <file> : Un dictionnaire de mots d'une langue cible\n");
    printf("  --method (-m) <m> : Une methode de cryptage/decryptage (xor, mask-crypt, mask-uncrypt, cbc-crypt ou cbc-uncrypt)\n");
    printf("  --v_init (-vi) <file> : Le vecteur d'initialisation, obligatoire uniquement si method cbc\n");
    printf("  --help (-h) : Affiche cette aide\n");
}
