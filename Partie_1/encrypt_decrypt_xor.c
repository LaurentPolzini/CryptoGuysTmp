#define _GNU_SOURCE
#define POSIX_C_SOURCE 200112L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <stdbool.h>
#include "chiffrement.h"
#include "../utilitaire/utiL.h"
#include "../utilitaire/uthash.h"
#define BLOCK_SIZE 16

char absolutMaxKeyLenPath[2048];
char absolutKeysListsPath[2048];

void setPaths(void) {
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        snprintf(absolutMaxKeyLenPath, sizeof(absolutMaxKeyLenPath), "%s/Partie_1/maxLenGeneratedKeys.txt", cwd);
        snprintf(absolutKeysListsPath, sizeof(absolutKeysListsPath), "%s/keys.txt", cwd);
    } else {
        perror("getcwd() error");
        exit(EXIT_FAILURE);
    }
}

const char CHARSET_KEY[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";

char *gen_key_unique(int length, char *key);

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


// Chiffrer ou déchiffrer un message avec une clé
char* encrypt_decrypt_xor(char* file_in, char* key, char* file_out) {
    // Ouvrir le fichier d'entrée
    FILE* in_f = fopen(file_in, "rb");
    if (!in_f) {
        printf("%s : ", file_in);
        fflush(stdout);
        perror("Erreur d'ouverture du fichier d'entrée");
        exit(EXIT_FAILURE);
    }

    // Charger la clé
    size_t key_size = strlen(key);
    if (key_size == 0) {
        fprintf(stderr, "Erreur : la clé ne doit pas être vide\n");
        fclose(in_f);
        return NULL;
    }

    // Ouvrir le fichier de sortie
    FILE* out_f = fopen(file_out, "wb");
    if (!out_f) {
        printf("%s : ", file_out);
        fflush(stdout);
        perror("Erreur d'ouverture du fichier de sortie");
        fclose(in_f);
        exit(EXIT_FAILURE);
    }

    // Calculer la taille du fichier d'entrée
    fseek(in_f, 0, SEEK_END);
    size_t file_size = ftell(in_f);
    rewind(in_f);

    // Allouer de la mémoire pour tout le message d'un coup
    char* message = malloc(file_size + 1);
    if (!message) {
        perror("Erreur d'allocation de mémoire");
        fclose(in_f);
        fclose(out_f);
        exit(EXIT_FAILURE);
    }

    // Lire et traiter le fichier en blocs
    size_t bytes_read;
    size_t total_bytes = 0;
    char buffer[1024];

    while ((bytes_read = fread(buffer, 1, sizeof(buffer), in_f)) > 0) {
        for (size_t i = 0; i < bytes_read; i++) {
            buffer[i] ^= key[(total_bytes + i) % key_size]; // Appliquer XOR
        }
        fwrite(buffer, 1, bytes_read, out_f); // Écrire dans le fichier de sortie
        memcpy(message + total_bytes, buffer, bytes_read); // Copier dans le message
        total_bytes += bytes_read;
    }

    // Terminer la chaîne de caractères
    message[total_bytes] = '\0';

    // Fermer les fichiers
    fclose(in_f);
    fclose(out_f);
    return message;
}

char* encrypt_decrypt_xorMSG(char* msg, char* key, off_t tailleMsg) {
    char* message = malloc(tailleMsg + 1);
    if (!message) {
        perror("Erreur allocation memoire encrypted/decrypted message");
        exit(1);
    }
    for (off_t i = 0 ; i < tailleMsg ; ++i) {
        message[i] = msg[i] ^ key[i % (strlen(key))];
    }
    message[tailleMsg] = '\0';
    return message;
}

// Génère une clé aléatoire de longueur spécifiée
char* gen_key(int length, char *key, bool mask) {
    if (mask) {
        return gen_key_unique(length, key);
    } else {
        int charset_size = sizeof(CHARSET_KEY) - 1;

        for (int i = 0; i < length; i++) {
            int random_index = true_random(charset_size);
            key[i] = CHARSET_KEY[random_index];
        }
        key[length] = '\0'; // Terminer la clé par un caractère nul
    }
    
    return key;
}

char *gen_key_unique(int length, char *key) {
    setPaths();
    dictionnary *dicoHash = NULL;

    // the length of the longest key is needed for the hashmap
    off_t lenMsg;

    char *cLenMaxKey = ouvreEtLitFichier(absolutMaxKeyLenPath, &lenMsg);
    int tailleKeyMax = length; // taille max des clefs generees

    if (tailleKeyMax < atoi(cLenMaxKey)) { // longueur max enregistrée
        tailleKeyMax = atoi(cLenMaxKey);
    } else { // nouvelle longueur max
        char cMaxKey[20];
        snprintf(cMaxKey, sizeof(cMaxKey), "%d", tailleKeyMax);
        remove(absolutMaxKeyLenPath);
        ouvreEtEcritMsg(absolutMaxKeyLenPath, cMaxKey, strlen(cMaxKey), false);
    }

    read_and_insert_words(absolutKeysListsPath, &dicoHash, &tailleKeyMax);

    do {
        key = gen_key(length, key, false);
    } while (find_word(dicoHash, key));

    ouvreEtEcritMsg(absolutKeysListsPath, key, length, false);
    clear_table(&dicoHash);

    return key;
}

// Fonction pour chiffrer un fichier avec un masque jetable
char* encrypt_mask(char* file_in, char* key, char* file_out) {
    // Ouvrir le fichier d'entrée
    FILE *fin = fopen(file_in, "rb");
    if (!fin) {
        printf("%s : ", file_in);
        fflush(stdout);
        perror("Erreur lors de l'ouverture du fichier d'entrée");
        return NULL;
    }

    // Déterminer la taille du message
    fseek(fin, 0, SEEK_END);
    size_t taille_msg = ftell(fin);
    fseek(fin, 0, SEEK_SET);

    char *tmpKey = malloc(taille_msg);
    pError(tmpKey, "Erreur allocation mémoire generation clef (pour masque)", 1);

    // Vérifier la taille de la clé
    if (!key || (strlen(key) < taille_msg)) {
        printf("Soit la clef n'est pas fournie, soit la clef n'est pas aussi longue que le message, génération de la clef...\n");
        
        gen_key(taille_msg, tmpKey, true);
        printf("clef gen : %s\n", tmpKey);
        printf("Clef générée et écrite dans ../keys.txt\n");
    } else {
        // sinon la clef est fournie, et de bonne taille, on utilise celle ci.
        strcpy(tmpKey, (const char *) key);
    }

    // Ouvrir le fichier de sortie
    FILE *fout = fopen(file_out, "wb");
    if (!fout) {
        fclose(fin);
        perror("Erreur lors de l'ouverture du fichier de sortie");
        return NULL;
    }

    // Lire et chiffrer le fichier par blocs
    char buffer[1024]; // Buffer pour stocker des portions du fichier
    size_t bytes_read;
    while ((bytes_read = fread(buffer, 1, sizeof(buffer), fin)) > 0) {
        for (size_t i = 0; i < bytes_read; i++) {
            buffer[i] ^= tmpKey[i % strlen(tmpKey)]; // Appliquer le chiffrement par masque
        }
        fwrite(buffer, 1, bytes_read, fout); // Écrire le résultat dans le fichier de sortie
    }

    // Fermer les fichiers
    fclose(fin);
    fclose(fout);
    free(tmpKey);

    return strdup(file_out); // Retourne le nom du fichier de sortie
}

// Fonction pour déchiffrer un fichier avec un masque jetable
char* decrypt_mask(char* file_in, char* key, char* file_out) {
    pError(key, "La clef ne doit pas etre vide !", 1);
    return encrypt_mask(file_in, key, file_out); // Le déchiffrement est identique au chiffrement
}

// Supprimer le padding d'un fichier
void remove_padding(char *file_out) {
    FILE *file = fopen(file_out, "rb+"); // Ouverture en lecture/écriture
    if (!file) {
        perror("Erreur d'ouverture du fichier pour suppression du padding");
        return;
    }

    fseek(file, -1, SEEK_END); // Aller au dernier octet
    int padding_value = fgetc(file); // Lire la valeur de padding

    if (padding_value >= 1 && padding_value <= BLOCK_SIZE) {
        // Vérifier que les derniers octets correspondent bien à la valeur de padding
        fseek(file, -padding_value, SEEK_END);
        int valid_padding = 1;
        for (int i = 0; i < padding_value; i++) {
            if (fgetc(file) != padding_value) {
                valid_padding = 0;
                break;
            }
        }

        if (valid_padding) {
            // Supprimer le padding en réduisant la taille du fichier
            int file_desc = fileno(file); 
            if (ftruncate(file_desc, ftell(file) - padding_value) != 0) {
                pError(NULL, "Erreur troncage du fichier (remove_padding)", 1);
            }
        }
    }

    fclose(file);
}

int encrypt_cbc(char *file_in, char *file_key, char *file_out, char *v_init) {
    FILE *input = fopen(file_in, "rb");
    FILE *output = fopen(file_out, "wb");
    if (!input || !output) {
        fprintf(stderr, "Erreur d'ouverture de fichier cbc\n");
        return -1;
    }

    unsigned char buffer[BLOCK_SIZE];
    unsigned char prev_block[BLOCK_SIZE];
    unsigned char current_block[BLOCK_SIZE];
    unsigned char key[BLOCK_SIZE];
    
    // Charger la clé depuis file_key
    FILE *key_file = fopen(file_key, "rb");
    if (!key_file || fread(key, 1, BLOCK_SIZE, key_file) != BLOCK_SIZE) {
        fprintf(stderr, "Erreur de lecture de la clé OU clé < 16\n");
        fclose(input);
        fclose(output);
        if (key_file) fclose(key_file);
        return -1;
    }
    fclose(key_file);

    // Initialiser le vecteur d'initialisation
    memcpy(prev_block, v_init, BLOCK_SIZE);

    // Lire chaque bloc et appliquer le chiffrement CBC
    size_t bytes_read;
    while ((bytes_read = fread(buffer, 1, BLOCK_SIZE, input)) > 0) {
        // Appliquer le padding si nécessaire
        if (bytes_read < BLOCK_SIZE) {
            memset(buffer + bytes_read, BLOCK_SIZE - bytes_read, BLOCK_SIZE - bytes_read);
        }

        // XOR avec le bloc précédent (ou IV pour le premier bloc)
        for (int i = 0; i < BLOCK_SIZE; i++) {
            current_block[i] = buffer[i] ^ prev_block[i];
        }

        // Appliquer la clé (XOR)
        for (int i = 0; i < BLOCK_SIZE; i++) {
            current_block[i] ^= key[i];
        }

        // Écrire le bloc chiffré et mettre à jour le bloc précédent
        fwrite(current_block, 1, BLOCK_SIZE, output);
        memcpy(prev_block, current_block, BLOCK_SIZE);
    }

    fclose(input);
    fclose(output);
    return 0;
}

// Déchiffrement CBC
int decrypt_cbc(char *file_in, char *file_key, char *file_out, char *v_init) {
    FILE *input = fopen(file_in, "rb");
    FILE *output = fopen(file_out, "wb");
    if (!input || !output) {
        fprintf(stderr, "Erreur d'ouverture de fichier decrypt cbc\n");
        return -1;
    }

    unsigned char buffer[BLOCK_SIZE];
    unsigned char prev_block[BLOCK_SIZE];
    unsigned char current_block[BLOCK_SIZE];
    unsigned char decrypted_block[BLOCK_SIZE];
    unsigned char key[BLOCK_SIZE];
    
    // Charger la clé depuis file_key
    FILE *key_file = fopen(file_key, "rb");
    if (!key_file || fread(key, 1, BLOCK_SIZE, key_file) != BLOCK_SIZE) {
        fprintf(stderr, "Erreur de lecture de la clé OU clé < 16\n");
        fclose(input);
        fclose(output);
        if (key_file) fclose(key_file);
        return -1;
    }
    fclose(key_file);

    // Initialiser le vecteur d'initialisation
    memcpy(prev_block, v_init, BLOCK_SIZE);

    // Lire chaque bloc et appliquer le déchiffrement CBC
    size_t bytes_read;
    while ((bytes_read = fread(current_block, 1, BLOCK_SIZE, input)) > 0) {
        // Sauvegarder le bloc chiffré actuel pour mise à jour du bloc précédent après traitement
        memcpy(buffer, current_block, BLOCK_SIZE);

        // Appliquer la clé (XOR) pour obtenir un bloc intermédiaire
        for (int i = 0; i < BLOCK_SIZE; i++) {
            decrypted_block[i] = current_block[i] ^ key[i];
        }

        // XOR avec le bloc précédent (ou IV pour le premier bloc)
        for (int i = 0; i < BLOCK_SIZE; i++) {
            decrypted_block[i] ^= prev_block[i];
        }

        // Écrire le bloc déchiffré et mettre à jour le bloc précédent
        fwrite(decrypted_block, 1, bytes_read, output);
        memcpy(prev_block, buffer, BLOCK_SIZE);
    }
    fclose(input);
    fclose(output);
    remove_padding(file_out); // Supprime le padding après le déchiffrement
    return 0;
}
