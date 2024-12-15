#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include "chiffrement.h"
#include "../utilitaire/utiL.h"


// Afficher le contenu d'un fichier en hexadécimal
void print_file_content(const char *file_path) {
    FILE *file = fopen(file_path, "rb");
    if (!file) {
        perror("Erreur d'ouverture du fichier");
        return;
    }

    printf("Contenu du fichier %s en hexadécimal :\n", file_path);
    int ch;
    while ((ch = fgetc(file)) != EOF) {
        printf("%02x ", (unsigned char)ch);
    }
    printf("\n");
    fclose(file);
}

// Afficher le contenu d'un fichier en clair
void print_file_content_clair(const char *file_path) {
    FILE *f_decrypted = fopen(file_path, "rb");
    if (!f_decrypted) {
        perror("Erreur lors de l'ouverture du fichier déchiffré");
        return;
    }
    
    char buffer[1024];
    size_t read_size = fread(buffer, 1, sizeof(buffer) - 1, f_decrypted);
    buffer[read_size] = '\0';
    fclose(f_decrypted);

    printf("%s\n", buffer);
}

void test_encrypt_decrypt(void) {
    printf("-------Début des tests de la fonction encrypt_decrypt_xor-------\n\n");

    char* file_in = "clair.txt";
    char* file_out_encrypted = "crypt.txt";
    char* file_out_decrypted = "decrypt.txt";
    char* key_file = "key.txt";
    
    char* key = ouvreEtLitFichier(key_file, NULL);
    // Test 1 : Vérifier que le chiffrement et déchiffrement fonctionnent correctement

    //Chiffrer le message
    encrypt_decrypt_xor(file_in, key, file_out_encrypted);
    //Déchiffrer le message
    encrypt_decrypt_xor(file_out_encrypted, key, file_out_decrypted);

    // Vérifier que le message déchiffré est identique au message original
    printf("Message original :");
    print_file_content_clair(file_in);
    print_file_content(file_in);
    printf("La clé est : %s\n", key);
    printf("Message chiffré :");
    print_file_content_clair(file_out_encrypted);
    print_file_content(file_out_encrypted);
    print_file_content(file_out_decrypted);
    printf("Message déchiffré :");
    print_file_content_clair(file_out_decrypted);

    //la taille de la phrase du fichier clair.txt
    FILE *fileclair = fopen("clair.txt", "rb");
    fseek(fileclair, 0, SEEK_END);
    long fileclair_length = ftell(fileclair);
    fclose(fileclair);
    printf("Taille du fichier clair en bytes : %ld\n", fileclair_length);
     FILE *file = fopen("crypt.txt", "rb");
    fseek(file, 0, SEEK_END);
    long file_length = ftell(file); 
    fclose(file);
    printf("Taille du fichier crypté en bytes : %ld\n", file_length);

    // Comparer la taille du fichier clair et du fichier crypté
    if (file_length == fileclair_length) {
        printf("La taille du fichier crypté est identique à celle du fichier clair\n");
    } else {
        printf("La taille du fichier crypté est différente de celle du fichier clair\n");
    }

    // Test 2 : Test de gen_key
    printf("\n Test 2 : Test de gen_key\n");
    char keyy[17];
    gen_key(17, keyy, false);
    printf("\nLa clé générée est : %s\n\n", keyy);

    //Ecrire la clé généré dans le fichier key.txt
    FILE *f = fopen("key.txt", "wb");
    if (!f) {
        perror("Erreur lors de la création du fichier de test");
        return;
    }
    fwrite(keyy, sizeof(char), strlen(keyy), f);
    fclose(f);

    //Test du cryptage decryptage avec la clé généré
    printf("Test 3 : Cryptage et décryptage avec la clé générée\n\n");
    file_in = "clair2.txt";
    print_file_content_clair(file_in);
    print_file_content(file_in);
    encrypt_decrypt_xor(file_in, key_file, file_out_encrypted);
    encrypt_decrypt_xor(file_out_encrypted, key_file, file_out_decrypted);
    print_file_content(file_out_decrypted);
    print_file_content_clair(file_out_decrypted);

    // Test 4 : Clé incorrecte
    printf("\n \n Test 4 : Déchiffrement avec une clé incorrecte\n");
    FILE *incorrect_key = fopen("incorrect_key.txt", "wb");
    key = ouvreEtLitFichier("incorrect_key.txt", NULL);
    fclose(incorrect_key);

    encrypt_decrypt_xor(file_out_encrypted, key, file_out_decrypted);
    if (system("cmp -s clair.txt decrypt.txt") != 0) {
        printf("Test réussi : Déchiffrement échoue avec une clé incorrecte\n");
    } else {
        printf("Test échoué : Le message déchiffré ne devrait pas correspondre\n");
    }

}

// Test pour encrypt_mask et decrypt_mask
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
    int taille_cle = strlen(message) + 1;  // Pour le masque jetable, la clé doit être au moins aussi longue que le message
    char *cle = (char *) malloc(taille_cle + 1);
    gen_key(taille_cle, cle, true);
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

// Générer des fichiers de test
void generate_test_files(void) {
    // Crée un fichier de test avec un message clair
     FILE *file = fopen("message.txt", "wb");
    fprintf(file, "Ceci est un message secret.");
    fclose(file);

    // Crée un fichier avec une clé de 16 octets
    file = fopen("key.txt", "wb");
    unsigned char key[16] = "1234567890abcdef";
    fwrite(key, 1, 16, file);
    fclose(file);

    // Crée un fichier avec un vecteur d'initialisation de 16 octets
    file = fopen("iv.txt", "wb");
    unsigned char iv[16] = "initializationV";
    fwrite(iv, 1, 16, file);
    fclose(file);
}

void test_cbc(void) {
    printf("-------Début des tests de chiffrement et déchiffrement CBC-------\n");
    generate_test_files();
    // Fichiers d'entrée, de sortie et de clé
    char *file_in = "clair.txt";
    char *file_key = "key.txt";
    char *v_init = "iv.txt";
    char *file_out_encrypted = "message_encrypted.bin";
    char *file_out_decrypted = "message_decrypted.txt";

    // Test 1 : Chiffrement et déchiffrement avec CBC
    printf("Test 1 : Chiffrement et déchiffrement avec CBC\n");

    printf("Contenu du fichier clair :\n");
    print_file_content_clair(file_in);

    // Chiffrement
    if (encrypt_cbc(file_in, file_key, file_out_encrypted, v_init) == 0) {
        printf("Chiffrement réussi.\n");
        print_file_content(file_out_encrypted); // Afficher le contenu du fichier chiffré
    } else {
        printf("Erreur lors du chiffrement.\n");
        return;
    }

    print_file_content(file_out_decrypted);

    // Déchiffrement
    if (decrypt_cbc(file_out_encrypted, file_key, file_out_decrypted, v_init) == 0) {
        printf("Déchiffrement réussi.\n");
        print_file_content_clair(file_out_decrypted); // Afficher le contenu du fichier déchiffré
    } else {
        printf("Erreur lors du déchiffrement.\n");
    }
    // Test 2 : Déchiffrement avec un IV incorrect
    printf("Test 2 : Déchiffrement avec un IV incorrect\n");
    FILE *f_incorrect_iv = fopen("incorrect_iv.txt", "wb");
    fwrite("invalid_iv_value", 1, 16, f_incorrect_iv);
    fclose(f_incorrect_iv);

    decrypt_cbc(file_out_encrypted, file_key, file_out_decrypted, "incorrect_iv.txt");
    if (system("cmp -s clair.txt message_decrypted.txt") != 0) {
        printf("Test réussi : Déchiffrement échoue avec un IV incorrect\n");
    } else {
        printf("Test échoué : Le message déchiffré ne devrait pas correspondre\n");
    }
}
/*
int main() {
    test_encrypt_decrypt();
    printf("\n");
    test_encrypt_decrypt_mask();
    printf("\n");
    test_cbc();
    printf("\n-------Fin des tests-------\n");
    return 0;
}
*/
