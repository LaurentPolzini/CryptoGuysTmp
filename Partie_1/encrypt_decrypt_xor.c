#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "xor.h"
#include "mask.h"

char* gen_key(int n, char* key);

// Chiffrer ou déchiffrer un message avec une clé
char *encrypt_decrypt_xor(char* file_in, char* key, char* file_out) {
    size_t taille_msg = strlen(file_in);
    size_t taille_cle = strlen(key);

    for (size_t i = 0; i < taille_msg; i++) {
        /*
            n'arrive que dans le cas du chiffrage
            un probleme surgit si dans le message
            chiffré se trouve le caractere \0
            (n'arrive pas lors du dechiffrage
            puisque personne n'écrit un message avec
            le caractere \0)
            donc dans ce cas le caractere de codage
            devient un nouveau caractere aleatoire
            et la clef est de nouveau codé
        */
        
        char tmp;
        char clefTmp[2];
        clefTmp[1] = '\0';

        do {
            tmp = file_in[i] ^ key[i % taille_cle];
            if (tmp == '\0') {
                gen_key(1, clefTmp);
                key[i % taille_cle] = clefTmp[0];
            }
        } while (tmp == '\0');
        
        file_out[i] = tmp;
    }
    file_out[taille_msg] = '\0';
    
    return file_out;
}

// Générer une clé aléatoire de taille n
char* gen_key(int n, char* key) {
    char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    size_t charset_size = strlen(charset);
    for (int i = 0; i < n; i++) {
        key[i] = charset[rand() % charset_size];
    }
    key[n] = '\0';
    return key;
}

// Fonction pour chiffrer un fichier avec un masque jetable
char* encrypt_mask(char* file_in, char* key, char* file_out) {
    FILE *fin = fopen(file_in, "rb");
    if (!fin) {
        perror("Erreur lors de l'ouverture du fichier d'entrée");
        return NULL;
    }

    fseek(fin, 0, SEEK_END);
    size_t taille_msg = ftell(fin);
    fseek(fin, 0, SEEK_SET);

    if (strlen(key) < taille_msg) {
        fclose(fin);
        fprintf(stderr, "Erreur : la clé est plus courte que le message (%ld octets attendus, mais la clé fait %lu octets)\n", taille_msg, strlen(key));
        return NULL;
    }

    char *msg = malloc(taille_msg);
    if (!msg) {
        fclose(fin);
        perror("Erreur lors de l'allocation mémoire pour le message");
        return NULL;
    }
    fread(msg, 1, taille_msg, fin);
    fclose(fin);

    char *sortie = malloc(taille_msg + 1);
    if (!sortie) {
        free(msg); 
        perror("Erreur lors de l'allocation mémoire pour le message chiffré");
        return NULL;
    }

    // Chiffrer le message
    encrypt_decrypt_xor(msg, key, sortie);

    // Ouverture du fichier de sortie
    FILE *fout = fopen(file_out, "wb");
    if (!fout) {
        perror("Erreur lors de l'ouverture du fichier de sortie");
        free(msg);
        free(sortie);
        return NULL;
    }

    // Écrire le message chiffré dans le fichier de sortie
    fwrite(sortie, 1, taille_msg, fout);
    fclose(fout);

    free(msg);
    return sortie;  // Retour du message chiffré
}

// Fonction pour déchiffrer un fichier avec un masque jetable
char* decrypt_mask(char* file_in, char* key, char* file_out) {
    return encrypt_mask(file_in, key, file_out); // Le déchiffrement est identique au chiffrement
}

// Test pour encrypt_decrypt_xor
void test_encrypt_decrypt(void) {
    printf("-------Début des tests de la fonction encrypt_decrypt_xor-------\n");

    // Test 1 : Vérifier que le chiffrement et déchiffrement fonctionnent correctement
    char original_msg[] = "Les carottes sont cuites";
    char msg[1024];
    strcpy(msg, original_msg);

    // Générer une clé pour le test
    char cle[] = "rutabaga";
    char sortie[1024];

    // Chiffrer le message
    encrypt_decrypt_xor(msg, cle, sortie);

    // Déchiffrer le message (en utilisant la même clé)
    encrypt_decrypt_xor(sortie, cle, msg);

    // Vérifier que le message déchiffré est identique au message original
    printf("La clé est %s\n", cle);
    printf("Message original : %s\n", original_msg);
    printf("Message après chiffrement : %s\n", sortie);
    printf("Message après déchiffrement : %s\n", msg);

    // Afficher les messages en hexadécimal
    printf("Le message original est :\n");
    for (size_t i = 0; i < strlen(original_msg); i++) {
        printf("%02x ", original_msg[i]);
    }
    printf("\n");

    printf("Le message chiffré est :\n");
    for (size_t i = 0; i < strlen(original_msg); i++) {
        printf("%02x ", sortie[i]);
    }
    printf("\n");

    printf("Le message déchiffré est :\n");
    for (size_t i = 0; i < strlen(original_msg); i++) {
        printf("%02x ", msg[i]);
    }
    printf("\n");

    // Test 2 : Test de gen_key
    char key[17];
    gen_key(17, key);
    printf("La clé générée est : %s\n", key);
}


void test_encrypt_decrypt_mask(void) {
    printf("-------Début des tests de encrypt_mask et decrypt_mask-------\n");

    // Fichiers d'entrée et de sortie
    char *file_in = "message.txt";
    char *file_encrypted = "message_encrypted.txt";
    char *file_decrypted = "message_decrypted.txt";

    // Message original
    char *message = "Les carottes sont cuites";

    // Créer un fichier temporaire contenant le message original
    FILE *f = fopen(file_in, "wb");
    if (!f) {
        perror("Erreur lors de la création du fichier de test");
        return;
    }
    fwrite(message, sizeof(char), strlen(message), f);
    fclose(f);

    // Générer une clé
    int taille_cle = strlen(message);  // Pour le masque jetable, la clé doit être au moins aussi longue que le message
    char *cle = (char *)malloc(taille_cle + 1);
    gen_key(taille_cle, cle);
    printf("Clé générée pour le test : %s\n", cle);

    // Chiffrement avec encrypt_mask
    char *message_chiffre = encrypt_mask(file_in, cle, file_encrypted);
    if (!message_chiffre) {
        printf("Erreur lors du chiffrement\n");
        free(cle);
        return;
    }

    // Déchiffrement avec decrypt_mask
    char *message_dechiffre = decrypt_mask(file_encrypted, cle, file_decrypted);
    if (!message_dechiffre) {
        printf("Erreur lors du déchiffrement\n");
        free(cle);
        return;
    }

    // Lire le fichier déchiffré pour comparer avec le message original
    FILE *f_decrypted = fopen(file_decrypted, "rb");
    if (!f_decrypted) {
        perror("Erreur lors de l'ouverture du fichier déchiffré");
        free(cle);
        return;
    }
    
    char buffer[1024];
    size_t read_size = fread(buffer, 1, sizeof(buffer) - 1, f_decrypted);
    buffer[read_size] = '\0';
    fclose(f_decrypted);

    // Vérifier que le message déchiffré correspond au message original
    printf("Message original : %s\n", message);
    printf("Message déchiffré : %s\n", buffer);
    if (strcmp(message, buffer) == 0) {
        printf("Test réussi : Le message déchiffré correspond au message original\n");
    } else {
        printf("Test échoué : Le message déchiffré ne correspond pas au message original\n");
    }

    // Libérer la mémoire allouée pour la clé
    free(cle);
}
/*
int main(void) {
    test_encrypt_decrypt();
    //test_encrypt_decrypt_mask();
    return 0;
}
*/
