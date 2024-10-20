#include <stdio.h>
#include <getopt.h>
#include <stdbool.h>
#include <string.h>
#include "./xor.h"
#include "./cbc.h"
#include "./mask.h"
#include "../utilitaire/utiL.h"

void afficheManSym_crypt();
char *keyByUser(char* key);
void linkMethod(void *f, char *method);

/*

    Premier programme principal sym_crypt v1.

    Permet d'appeller les fonctions encrypt_decrypt_xor, 
    gen_key, encrypt_mask, decrypt_mask, encrypt_cbc, decrypt_cbc

    options possibles : 
    (obligatoire) -i le fichier où lire le message en clair
    (obligatoire) -o le fichier où l'on écrira le texte déchiffré
    (obligatoire) -k sinon -f, la clé ne peut qu'être alphanumérique
    (obligatoire) -m la méthode de chiffrement souhaitée
    (obligatoire si m) -v le vecteur d'initialisation si m = "encrypt_cbc" / "decrypt_cbc"
    -l le fichier de log
    -h affiche un manuel d'utilisation, empeche toutes les autres options

*/

int main(int argc, char* argv[]) {
    
    int opt;
    bool v_init_needed = false;
    bool not_k_then_f_needed = true;

    char *file_in = "";
    char *file_out = "";
    char *key = "";
    void *method = NULL;

    char *v_init = "";

    char *log_file = "";

    if ( !argContainsHelp(argc, argv) ) {
       // check if k then not f OR if f not k
        while ( (opt = getopt(argc, argv, ":i:okf:mvlh")) != -1 ) {
            switch (opt) {
                case 'i':
                    file_in = optarg;
                    break;

                case 'o':
                    file_out = optarg;
                    break;

                case 'k':
                    not_k_then_f_needed = false;
                    key = keyByUser(optarg);
                    break;

                case 'f':
                    if ( !not_k_then_f_needed ) {
                        perror("-f must not be used if -k !\n");
                        return -1;
                    }
                    key = optarg;
                    break;

                case 'm':
                    linkMethod(method, optarg);
                    if (method == NULL) {
                        perror("Enter a correct method ! (xor, cbc-crypt, cbc-uncrypt, mask)");
                        exit(1);
                    }
                    char *isCBC;
                    strncpy(isCBC, optarg, 3); // to know if v_init needed

                    if ( strcmp(isCBC, "cbc") == 0 ) {
                        v_init_needed = true;
                    }
                    break;
                
                case 'v':
                    if (v_init_needed) {
                        v_init = optarg;
                    } else {
                        perror("If the method is not of cbc, don't use -v");
                        exit(1);
                    }
                    break;

                case 'l':
                    log_file = optarg;
                    break;

                case ':':
                    perror(strcat("Option expected a value : ", opt));
                    exit(2);
                
                case '?':
                    perror(strcat("Unknown option", optopt));
                    exit(2);
            }
            
        }
    } 
    else {
        afficheManSym_crypt();
    }

    return 0;
}

void afficheManSym_crypt() {
    printf("Usage:\n");
    printf("./sym_crypt [options]");

    printf("Mandatory Options :\n");
    printf("-i      The file containing the message to encrypt\n");
    printf("-o      The file to write the transformation\n");
    printf("-k      The key to encrypt the file (not to be used if -f)\n");
    printf("-f      The file containing the key to encrypt the file (not to be used if -k)\n");
    printf("-m      The encrypt or decrypt method; can only be xor, cbc-crypt, cbc-uncrypt or mask\n");
    printf("-v      The initialization vector (only to use if -m cbc-crypt or -m cbc-uncrypt)\n");

    printf("Optionnal Options :\n");
    printf("-l      The log file (if not specified, stdin)\n");
    printf("-h      This help message\n");

    return;
}

char *keyByUser(char* key) {
    char *keyFileName = strcat(key, ".key");
    FILE *keyFile = fopen(keyFileName, "a");

    size_t nbOfBlocks = sizeof(key) / sizeof(char);

    if ( fwrite(key, sizeof(char), nbOfBlocks, keyFile) != nbOfBlocks ) {
        perror(strcat("Erreur d'écriture du fichier", keyFileName));
        exit(1);
    }

    fclose(keyFile);
    return keyFileName;
}

void linkMethod(void *f, char *method) {
    if (strcmp(method, "xor") == 0) {
        f = encrypt_decrypt_xor;
        return;
    }
    if (strcmp(method, "cbc-crypt") == 0) {
        f = encrypt_cbc;
        return;
    }
    if (strcmp(method, "cbc-uncrypt") == 0) {
        f = decrypt_cbc;
        return;
    }
    if (strcmp(method, "mask") == 0) {
        f = encrypt_decrypt_mask;
        return;
    }
    f = NULL;
    return;
}

