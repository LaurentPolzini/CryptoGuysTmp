#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include "./Partie_1/chiffrement.h"
#include "./Partie_3/crackage.h"
#include "./utilitaire/utiL.h"

void appel_crackage(char *method, char *in, int len_key, char *dico, char *logName, char *scoresName, char *crypte2, char *clair1, char *out_clair2);

void list_keys(void);
void genkey(int longueur, char *key);
void del_key(const char *key);
void afficher_aidePR(void);

int main(int argc, char *argv[]) {
    (void) argc, (void) argv;
    int opt;
    char *commande = NULL;
    char *file_in = NULL;
    char *file_out = NULL;
    char *key = NULL;
    char *method = NULL;
    char *v_init = NULL;
    char *log_file = NULL;
    char *scores_file = NULL;
    char *dico = NULL;
    int key_length = 0;

    // msg 1 crypté : in
    char *m2_crypte = NULL;
    char *m1_clair = NULL;
    // msg 2 clair : out

    // Détection de la commande
    static struct option long_options[] = {
        {"command", required_argument, 0, 'c'},  // Commande principale
        {"in", required_argument, 0, 'i'},       // Fichier d'entrée (peut aussi etre m1 crypté (crack mask))
        {"msg1", required_argument, 0, 'r'},    // message m2 crypté (crack mask)
        {"clair1", required_argument, 0, 'e'},  // message m1 (in) en clair (crack mask)
        {"out", required_argument, 0, 'o'},      // Fichier de sortie (peut aussi etre m2 en clair (crack mask))
        {"key", required_argument, 0, 'k'},      // Clé
        {"key_file", required_argument, 0, 'f'}, // Fichier de la clef
        {"length", required_argument, 0, 'l'},   // Longueur de la clé
        {"method", required_argument, 0, 'm'},   // Méthode de chiffrement
        {"init", required_argument, 0, 'v'},     // Vecteur d’initialisation
        {"log", required_argument, 0, 'g'},      // Fichier de log
        {"scores", required_argument, 0, 's'},   // Fichier des scores
        {"dico", required_argument, 0, 'd'},     // le dictionnaire de mots pour le crackage
        {"help", no_argument, 0, 'h'},           // Aide
        {"quit", no_argument, 0, 'q'},           // Quitter le logiciel
        {0, 0, 0, 0}
    };
    /*
    ne marche pas je ne sais pas pourquoi
    printf("Entrez votre commande !\n");
    while (1) {
        
        char input[512];
        if (!fgets(input, sizeof(input), stdin)) {
            fprintf(stderr, "Erreur de lecture.\n");
            break;
        }

        input[strcspn(input, "\n")] = 0;

        // Convertir l'entrée utilisateur en tableau d'arguments
        // pour getopt
        int arg_count = 0;
        char *args[64];

        char *token = strtok(input, " ");
        while (token && arg_count < 64) {
            args[arg_count++] = token;
            token = strtok(NULL, " ");
        }
        args[arg_count] = NULL;
        
        // reset getopt
        optind = 1;
        optreset = 1;
    */
        // Traitement des options avec getopt_long
        while ((opt = getopt_long(argc, argv, "c:i:o:k:l:m:v:g:d:s:r:e:f:hq", long_options, NULL)) != -1) {
            switch (opt) {
                case 'c': // Commande
                    commande = optarg;
                    break;
                case 'm':
                    method = optarg;
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
                case 'd': // dico for frequences crack
                    dico = optarg;
                    break;
                case 's': // the log file of the keys' scores
                    scores_file = optarg;
                    break;
                case 'r': // the second crypted message for crack mask
                    m2_crypte = optarg;
                    break;
                case 'e': // the first clear message for crack mask
                    m1_clair = optarg;
                    break;
                case 'v': // le vecteur d'initialisation pour cbc
                    v_init = optarg;
                    break;
                case 'f': // le fichier contenant la clef
                    key = ouvreEtLitFichier(optarg, NULL);
                    break;
                case 'h': // Aide
                    afficher_aidePR();
                    return 0;
                case 'q': // quitter le logiciel
                    return 0;
                default:
                    fprintf(stderr, "Option invalide. Utilisez --help (-h) pour voir les commandes disponibles.\n");
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
                genkey(key_length, generated_key);
                printf("Clé générée : %s\n", generated_key);
                if (file_out) {
                    ouvreEtEcritMsg(file_out, generated_key, key_length, false);
                }

            } else {
                fprintf(stderr, "Erreur : spécifiez une longueur de clé avec --length (-l).\n");
            }
        } else if (strcmp(commande, "del-key") == 0) {
            if (key != NULL) {
                del_key(key);
            } else {
                fprintf(stderr, "Erreur : spécifiez une clé avec --key (-k).\n");
            }
        } else if (strcmp(commande, "chiffrement") == 0) {
            appel_chiffrement(file_in, file_out, key, method, v_init);
        } else if (strcmp(commande, "crack") == 0) {
            appel_crackage(method, file_in, key_length, dico, log_file, scores_file, m2_crypte, m1_clair, file_out);   
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
        fflush(stdout);
    }

    fclose(file);
}

void genkey(int longueur, char *key) {
    gen_key(longueur, key, false); // Appel à la fonction de génération
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
    printf("  --command (-c) <cmd> : Commande à exécuter (list-keys, gen-key, del-key, chiffrement, crack.)\n");

    printf("\n- list-keys ne requiert pas d'option.\n");

    printf("\n- gen-key : \n");
    printf("\t  --length (-l) <n> : Longueur de clé à générer\n");

    printf("- del-key : \n");
    printf("\t  --key (-k) <key> : Clé de chiffrement/déchiffrement\n");
    printf("\t  Ou  --key_file (-f) <file> : Fichier d'entrée de la clef. Attention au fichier entré : tout peut être effacé.\n");

    printf("\n- chiffrement : \n");
    printf("\t  --method (-m) <m> : Une methode de cryptage/decryptage (xor, mask-crypt, mask-uncrypt, cbc-crypt ou cbc-uncrypt)\n");
    printf("\t  --in (-i) <file> : Fichier d'entrée\n");
    printf("\t  --out (-o) <file> : Fichier de sortie\n");

    printf("\tOptionnel pour mask-crypt :\n");
    printf("\t  --key (-k) <key> : Clé de chiffrement/déchiffrement\n");
    printf("\t  --key_file (-f) <file> : Fichier d'entrée de la clef\n");

    printf("\tObligatoire pour cbc :\n");
    printf("\t  --v_init (-v) <file> : Le vecteur d'initialisation\n");

    printf("\n- crack : \n");
    printf("\t  --method (-m) <m> : Une methode de crackage (freq ou mask)\n\n");

    printf("\tObligatoires : \n");
    printf("\t  --in (-i) <file> : Fichier à cracker (ou le premier fichier crypté pour mask)\n");
    printf("\t  --length (-l) <n> : Longueur de clé maximale suspectée\n\n");
    printf("\t  --out (-o) <file> : Fichier ou sera écrit le deuxieme message clair\n\n");

    printf("\tPour freq : \n");
    printf("\t  --dico (-d) <file> : Un dictionnaire de mots d'une langue cible\n");

    printf("\tPour mask : \n");
    printf("\t  --crypte (-r) <file> : 2ème fichier crypté (crack mask)\n");
    printf("\t  --clair (-e) <file> : 1er message en clair (crack mask)\n");
    printf("\t Voir le dernier exemple\n\n");

    printf("\tOptionnelles : \n");
    printf("\t  --scores (-s) <file> : Le fichier des scores des clefs (crack)\n");
    printf("\t  --log (-l) <file> : Le fichier de log\n");
    
    
    printf("\n  --help (-h) : Affiche cette aide\n");
    //printf("  --quit (-q) : Quitte le logiciel\n");

    printf("\nExemples : ./crypto -c chiffrement -m xor -i clair.txt -o crypte.txt -k clef (clair.txt s'encrypte dans crypte.txt avec la clef clef)\n");
    printf("./crypto -c gen-key -l taille_souhaitée (genere une clef de la longueur souhaitée)\n");
    printf("./crypto -c list-keys (liste les clefs dans keys.txt)\n");
    printf("./crypto -c del-key -k clef (retire du fichier keys.txt la clef clef)\n");
    printf("./crypto -c chiffrement -m mask-crypt -i clair.txt -o crypte.txt (clair.txt est encrypté dans crypte.txt si la clef n'est pas fournie ou trop courte elle est générée)\n");
    printf("./crypto -c chiffrement -m mask-uncrypt -i crypte.txt -o clair.txt -k clef (la clef est obligatoire !)\n");
    printf("./crypto -c crack -m freq -i crypte.txt -d dictionnaire_langue_cible -l longueur_maximum_suspecté -g log_file -s scores_out (-g et -s pas obligatoires)\n");
    printf("./crypto -c crack -m mask -i crypte1.txt -r crypte2.txt -e clair1.txt -o clair2.txt\n");
}


/* 
    method = freq / mask
    in : crypté

    scoresName : ou seront stocker les scores des clefs

*/
void appel_crackage(char *method, char *in, int len_key, char *dico, char *logName, char *scoresName, char *crypte2, char *clair1, char *out_clair2) {
    if (strstr(method, "freq")) {
        if (len_key == 0) {
            fprintf(stderr, "Erreur : spécifiez une taille de clé avec --length (-l).\n");
        }
        pError(dico, "Le dictionnaire doit être entré !", 1);
        break_code_all_max_len(in, dico, scoresName, len_key, logName);
    } else {
        pError(in, "Il manque le premier fichier crypté !", 1);
        pError(crypte2, "Il manque le second fichier crypté !", 1);
        pError(clair1, "Il manque le premier fichier en clair !", 1);
        pError(out_clair2, "Il manque le fichier de sortie comportant le second message en clair", 1);
        crack_mask(in, crypte2, clair1, out_clair2);
    }
}


