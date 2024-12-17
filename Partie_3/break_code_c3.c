#include <stdio.h>
#include <stdlib.h> // malloc
#include <ctype.h> // isalpha...
#include <string.h>
#include <pthread.h> // mutex d'ecriture
#include <fcntl.h> // open
#include <time.h> // calcul du temps
#include "./crackage.h" // library de cette partie
#include "../utilitaire/utiL.h" // pError...
#include "../utilitaire/uthash.h" // dictionnary
#include "./break_code_c3.h"
#include "./break_code_c1.h" // appel programme de recherche des clefs possible
#include "./break_code_c2.h" // filtrage sur les fréquences des lettres
#include "./break_code_c2_c3.h" // struct_c2_c3
#include "../Partie_1/chiffrement.h" // encrypt_decrypt_xor_msg

extern float stat_thFr[26];
extern float stat_thEn[26];

typedef struct scores_keys {
    int *tab_nb_mots_prez;
    unsigned char **keys;
    int tailleTab;
    int *tailleActuelle;
} scores_keys;

struct struct_c3 {
    scores_keys *struct_score;
    sMsgAndTaille *msgAndTaille;

    int *ptr_nb_mots_prez;
    dictionnary *dico;

    int len_key;

    int fdScoreOut;
};

scores_keys *init_struct_score(int taille_tab, int lenKey);
void destruct_struct_score(scores_keys **s_score);

int nbMotsPresents(char **mots, int nbMots, dictionnary *dico);

void test_text_word_splitter(dictionnary *dico);

//-----------------------------------------------------------------
/*
    3 "mains" (break_code_c3, break_code_all_exact_len, break_code_all_max_len) : 
    tous font la meme chose au début et a la fin -> compactage
*/
void init_params_mains_c3(char *score_out, int *fdScoreOut, char *logFileName, FILE **fileLog, dictionnary **dico,
    char *dict_file_in, off_t *tailleMsg, char **msgCrypted, char *file_in, float **stats) {
    init_params(score_out, fdScoreOut, logFileName, fileLog);

    // Read words from the Scrabble dictionary and insert them into the hash table
    read_and_insert_words(dict_file_in, dico, NULL);

    *msgCrypted = ouvreEtLitFichier(file_in, tailleMsg);

    if (stats) {
        *stats = stat_thEn;
        if (find_word(*dico, "abaisser")) {
            *stats = stat_thFr;
        }
    }
}

void destroy_params_mains_c3(int *fdScoreOut, FILE **fileLog, char **cryptedMsg, dictionnary **dico, 
    stC2_C3 **st_c2_c3) {

    clear_table(dico);

    destruct_stC2_C3(st_c2_c3);

    destroy_params(fdScoreOut, fileLog);
	
	free(*cryptedMsg);
}

//-----------------------------------------------------------------

/*
    juste c3 : sans passer par l'émondage c2
*/
int break_code_c3(char *file_in, char *dict_file_in, char *score_out, int keyLen, char *logFileName) {
    int fdScoreOut = -1;
    FILE *logFile = NULL;
    dictionnary *dicoHash = NULL;
    off_t tailleMsg = 0;
    char *cryptedMsg = NULL;

    init_params_mains_c3(score_out, &fdScoreOut, logFileName, &logFile, &dicoHash, dict_file_in,
        &tailleMsg, &cryptedMsg, file_in, NULL);

    
    printf("Attaque brutale avec dico : c1, c3 enchaînées...\n");
    printf("==>dico name = %s\n", dict_file_in);
    printf("Créé et initialisé\n");

    struct_c3 *s_c3 = init_struct_c3(cryptedMsg, tailleMsg, TAILLE_TAB_SCORE, keyLen, dicoHash, fdScoreOut);

    stC2_C3 *stC2C3 = init_stC2_C3(NULL, s_c3);

    unsigned long nbKeys;
    appelClefsFinales(file_in, keyLen, (void *) stC2C3, functorC3, logFileName, true, &nbKeys);
    printf("Début de C3...\n");
    printf("Fin de C3\n");

    printf("Nombre de clefs = %ld\n", nbKeys);

    affiche_meilleures_clefs_c3(s_c3, cryptedMsg, tailleMsg, 10);

    ecritTab_c3(get_tab_nb_mots(s_c3), get_tab_tailleActuelle(s_c3), get_keys_c3(s_c3), logFile);
    

    destroy_params_mains_c3(&fdScoreOut, &logFile, &cryptedMsg, &dicoHash, &stC2C3);
    
    return 0;
}

/*
    keylen utilisée en tant que tel
*/
int break_code_all_exact_len(char *file_in, char *dict_file_in, char *score_out, int keyLen, char *logFileName) {
    int fdScoreOut = -1;
    FILE *logFile = NULL;
    dictionnary *dicoHash = NULL;
    off_t tailleMsg = 0;
    char *cryptedMsg = NULL;
    float *stats = NULL;

    init_params_mains_c3(score_out, &fdScoreOut, logFileName, &logFile, &dicoHash, dict_file_in,
        &tailleMsg, &cryptedMsg, file_in, &stats);

    printf("Attaque brutale avec dico : c1, c2, c3 enchaînées...\n");
    printf("==>dico name = %s\n", dict_file_in);
    printf("Créé et initialisé\n");

    struct_c2 *s_c2 = init_struct_c2(cryptedMsg, tailleMsg, TAILLE_TAB_SCORE, keyLen, stats, fdScoreOut);

    // NULL pour pas qu'il y ait de copies de c3
    stC2_C3 *sc2c3 = init_stC2_C3(s_c2, NULL);

    unsigned long nbKeys;
    // true pour copier les structures
    appelClefsFinales(file_in, keyLen, (void *) sc2c3, functorC2, logFileName, true, &nbKeys);
    // comme je passe par un fonctor, c2 n'a que comme debut celui de c1
    // et comme fin, c1.
    // en passant par un stockage de clef en revanche,
    // il y aurait un debut de c1, fin c1, et debut c2, fin c2
    // (traitement des clefs apres generation, pas pendant)
    printf("Début de C2...\n");
    printf("Fin de C2\n");

    printf("Nombre de clefs = %ld\n", nbKeys);

    // on remarquera que s_c3 ne fait pas partie de la structure sc2c3
    // car sinon s_c3 sera copié par tous les threads alors qu'inutile 
    struct_c3 *s_c3 = init_struct_c3(cryptedMsg, tailleMsg, get_taille_actuelle_tab_s_c2(s_c2), keyLen, dicoHash, -1);

    printf("Début de C3...\n");
    traite_clefs_generee_c2(s_c3, s_c2);
    printf("Meilleur score = %d\n", get_meilleur_score_c3(s_c3));
    printf("Fin de C3 : meilleure clef = %s\n", get_meilleur_clef_c3(s_c3));

    ecritTab_c3(get_tab_nb_mots(s_c3), get_tab_tailleActuelle(s_c3), get_keys_c3(s_c3), logFile);


    destroy_params_mains_c3(&fdScoreOut, &logFile, &cryptedMsg, &dicoHash, &sc2c3);
    destruct_struct_c3(&s_c3);

    return 0;
}

/*
    keylen utilisée de 1 à keyLen
*/
int break_code_all_max_len(char *file_in, char *dict_file_in, char *score_out, int keyLen, char *logFileName) {
    int fdScoreOut = -1;
    FILE *logFile = NULL;
    dictionnary *dicoHash = NULL;
    off_t tailleMsg = 0;
    char *cryptedMsg = NULL;
    float *stats = NULL;

    init_params_mains_c3(score_out, &fdScoreOut, logFileName, &logFile, &dicoHash, dict_file_in,
        &tailleMsg, &cryptedMsg, file_in, &stats);

    printf("Attaque brutale avec dico : c1, c2, c3 enchaînées...\n");
    printf("==>dico name = %s\n", dict_file_in);
    printf("Créé et initialisé\n");

    struct_c2 *compiled_best = init_struct_c2(cryptedMsg, tailleMsg, TAILLE_TAB_SCORE, keyLen, stats, fdScoreOut);

    struct_c2 *s_c2;
    stC2_C3 *sc2c3;
    unsigned long nbKeys = 0;
    unsigned long nbKeysTMP = 0;

    for (int i = 1 ; i <= keyLen ; ++i) {
        s_c2 = init_struct_c2(cryptedMsg, tailleMsg, TAILLE_TAB_SCORE, i, stats, fdScoreOut);

        sc2c3 = init_stC2_C3(s_c2, NULL);

        // true pour copier les structures
        appelClefsFinales(file_in, i, (void *) sc2c3, functorC2, logFileName, true, &nbKeysTMP);
		// ici il n'y a plus de threads
        nbKeys += nbKeysTMP;

        printf("\n\n");

        // on met dans compiled_best les meilleurs scores de s_c2
        // on peut donc retrouver dans compiled_best des clefs de tailles variées
        compile_structs_c2(compiled_best, s_c2);

        destruct_stC2_C3(&sc2c3);
    }
    printf("Début de C2...\n");
    printf("Fin de C2\n");

    printf("Nombre de clefs total = %ld\n", nbKeys);

    struct_c3 *s_c3 = init_struct_c3(cryptedMsg, tailleMsg, get_taille_actuelle_tab_s_c2(compiled_best), keyLen, dicoHash, -1);
    printf("Début de C3...\n");
    traite_clefs_generee_c2(s_c3, compiled_best);
	
	if (get_meilleur_clef_c3(s_c3)) {
		printf("Fin de C3 : meilleure clef = %s\n", get_meilleur_clef_c3(s_c3));
	} else {
		printf("Fin de C3 : n'a pas marché.\n");
	}


    affiche_meilleures_clefs_c3(s_c3, cryptedMsg, tailleMsg, 10);

    ecritTab_c3(get_tab_nb_mots(s_c3), get_tab_tailleActuelle(s_c3), get_keys_c3(s_c3), logFile);
	

	destruct_struct_c2(&compiled_best);
	destruct_struct_c3(&s_c3);
    destroy_params_mains_c3(&fdScoreOut, &logFile, &cryptedMsg, &dicoHash, &sc2c3);

    return 0;
}

/*
    pour "all" (émondage c1 -> c2 -> c3 final)

    assigne un nombre de mots présents dans le dictionnaire
    à partir des clefs de c2
        => Toutefois, il est possible et meme probable
        que c2 donne des clefs nulles selon le style
        de l'écrivain

        pour voir les différences on peut lancer juste c2 et juste c3
        afin de comparer leurs meilleures clefs
*/
void traite_clefs_generee_c2(struct_c3 *s_c3, struct_c2 *s_c2) {
    char *uncrypted_msg;
    int indInsertion;

    for (int i = 0 ; i < get_taille_actuelle_tab_s_c2(s_c2) ; ++i) {
        uncrypted_msg = encrypt_decrypt_xorMSG((s_c3 -> msgAndTaille) -> msg, (char *) get_keys_s_c2(s_c2)[i], (s_c3 -> msgAndTaille) -> lenMsg);
        traiteMsgClefC3(uncrypted_msg, s_c3 -> ptr_nb_mots_prez, s_c3 -> dico);
        free(uncrypted_msg);

        indInsertion = getIndexInsertionC3_struc(s_c3);
        ajouteScoreC3(s_c3, get_keys_s_c2(s_c2)[i], indInsertion);
    }
}

/*
    pour utiliser uniquement c3 (sans passer par c2)
    permet de voir les différences de scores de clefs
*/
void functorC3(unsigned char *key, void *argStruc) {
    struct_c3 *varStructC3 = ((stC2_C3 *) argStruc) -> s_c3;

    // recoit un message codé, le décrypte avec la clef
    translateMsg(key, (void *) (varStructC3 -> msgAndTaille));

    traiteMsgClefC3((varStructC3 -> msgAndTaille) -> msgUncrypted, varStructC3 -> ptr_nb_mots_prez, varStructC3 -> dico);
    int ind = getIndexInsertionC3_struc(varStructC3);
    ajouteScoreC3(varStructC3, key, ind);

    ecritClefScore_c3(varStructC3 -> fdScoreOut, key, *(varStructC3 -> ptr_nb_mots_prez));

    free((void *) ((varStructC3 -> msgAndTaille) -> msgUncrypted));
}


//------------------------------------------------------------------------------------------------------------
/*
    Fonctions d'initialisations/ destructions des structures

    ces structures servent pour le functor sur les clefs dans c1

    On a aussi des getters des attributs
*/
scores_keys *init_struct_score(int taille_tab, int lenKey) {
    scores_keys *s_score = malloc(sizeof(scores_keys));
    pError(s_score, "Erreur allocation memoire", 3);
    s_score -> tab_nb_mots_prez = malloc(sizeof(double) * taille_tab);
    pError(s_score -> tab_nb_mots_prez, "Erreur allocation memoire", 3);

    s_score -> keys = malloc(sizeof(char *) * taille_tab);
    pError(s_score -> keys, "Erreur allocation memoire", 3);

    for (int i = 0 ; i < taille_tab ; ++i) {
        (s_score -> tab_nb_mots_prez)[i] = 0;

        (s_score -> keys)[i] = malloc(lenKey + 1);
        pError((s_score -> keys)[i], "Erreur allocation mémoire", 3);
    }

    s_score -> tailleActuelle = malloc(sizeof(int));
    *(s_score -> tailleActuelle) = 0;

    s_score -> tailleTab = taille_tab;

    return s_score;
}

void destruct_struct_score(scores_keys **s_score) {
    for (int i = 0 ; i < (*s_score) -> tailleTab ; ++i) {
        free((void *) ((*s_score) -> keys)[i]);
    }
    free((void *) (*s_score) -> tab_nb_mots_prez);
    free((void *) (*s_score) -> keys);
    
    free((void *) (*s_score) -> tailleActuelle);

    free((void *) *s_score);
}

struct_c3 *init_struct_c3(char *msgCrypted, off_t tailleMsgCrypted, int tailleTab, int keyLen, dictionnary *dicO, int fdScore) {
    struct_c3 *s_c3 = malloc(sizeof(struct_c3));
    pError(s_c3, "Erreur allocation memoire", 3);

    s_c3 -> msgAndTaille = malloc(sizeof(sMsgAndTaille));
    pError(s_c3 -> msgAndTaille, "Erreur allocation memoire", 3);
    (s_c3 -> msgAndTaille) -> msg = malloc(tailleMsgCrypted);
    pError((s_c3 -> msgAndTaille) -> msg, "Erreur allocation memoire", 3);
    memcpy((s_c3 -> msgAndTaille) -> msg, msgCrypted, tailleMsgCrypted);
    
    (s_c3 -> msgAndTaille) -> lenMsg = tailleMsgCrypted;

    s_c3 -> struct_score = init_struct_score(tailleTab, keyLen);
    
    s_c3 -> dico = dicO;

    s_c3 -> ptr_nb_mots_prez = malloc(sizeof(int));
    *(s_c3 -> ptr_nb_mots_prez) = 0;
    pError(s_c3 -> ptr_nb_mots_prez, "Erreur allocation memoire", 3);

    s_c3 -> len_key = keyLen;

    s_c3 -> fdScoreOut = fdScore;

    return s_c3;
}

struct_c3 *copy_s_c3(struct_c3 *to_copy) {
    struct_c3 *copied = NULL;
    if (to_copy) {
        copied = init_struct_c3((to_copy -> msgAndTaille) -> msg, (to_copy -> msgAndTaille) -> lenMsg, get_taille_tab_s_c3(to_copy), to_copy -> len_key, to_copy -> dico, to_copy -> fdScoreOut);
    }
    return copied;
}

void destruct_struct_c3(struct_c3 **s_c3) {
    if (s_c3) {
        free(((*s_c3) -> msgAndTaille) -> msg);
        free((void *) ((*s_c3) -> msgAndTaille));

        destruct_struct_score(&((*s_c3) -> struct_score));

        free((void *) (*s_c3) -> ptr_nb_mots_prez);

        free((void *) (*s_c3));
    }
}

int *get_tab_nb_mots(struct_c3 *s_c3) {
    return (s_c3 -> struct_score) -> tab_nb_mots_prez;
}

unsigned char **get_keys_c3(struct_c3 *s_c3) {
    if (s_c3) {
        return (s_c3 -> struct_score) -> keys;
    }
    return NULL;
}

int get_taille_tab_s_c3(struct_c3 *s_c3) {
    if (s_c3) {
        return (s_c3 -> struct_score) -> tailleTab;
    }
    return -1;
}

int get_tab_tailleActuelle(struct_c3 *s_c3) {
    if (s_c3) {
        return *((s_c3 -> struct_score) -> tailleActuelle);
    }
    return -1;
}

int get_meilleur_score_c3(struct_c3 *s_c3) {
    if (s_c3) {
        return (s_c3 -> struct_score -> tab_nb_mots_prez)[0];
    }
    return -1;
}

unsigned char *get_meilleur_clef_c3(struct_c3 *s_c3) {
    if (s_c3) {
        return (s_c3 -> struct_score -> keys)[0];
    }
    return NULL;
}

int get_len_key_c3(struct_c3 *s_c3) {
    return s_c3 -> len_key;
}

//------------------------------------------------------------------------------------------------------------
/*
    Fonctions pour gérer les scores de la structure struct_c3
*/

/*
    insere les valeurs de from dans to (si utile)
*/
void compile_structs_c3(struct_c3 *to, struct_c3 *from) {
    int indInser;
    for (int i = 0 ; i < get_tab_tailleActuelle(from) ; ++i) {
        *(to -> ptr_nb_mots_prez) = get_tab_nb_mots(from)[i];
        indInser = getIndexInsertionC3_struc(to);

        ajouteScoreC3(to, get_keys_c3(from)[i], indInser);
    }
}

/*
    une structure struct_c3 contenant 2 tableaux :
        un tableau 1 possédant les meilleurs scores (plus on est haut mieux c'est)
        un tableau 2 de clefs à l'indice correspondant au tableau 1

    ajoute le score s_c3 -> *(ptr_nb_mots_prez) dans le 
    tableau avec la clef key correspondante
*/
void ajouteScoreC3(struct_c3 *s_c3, unsigned char *key, int ind) {
    if (ind < get_taille_tab_s_c3(s_c3) && ind >= 0) {
        for (int i = get_tab_tailleActuelle(s_c3) ; i > ind ; --i) {
            // décaler tout le tableau vers la droite
            // (copier tous les elements pour en inserer un nouveau)
            if (i < get_taille_tab_s_c3(s_c3) && i != 0) {
                get_tab_nb_mots(s_c3)[i] = get_tab_nb_mots(s_c3)[i - 1];
				strcpy((char *) get_keys_c3(s_c3)[i], (char *) get_keys_c3(s_c3)[i - 1]);
            }
        }
        // insertion du nouvel element
        get_tab_nb_mots(s_c3)[ind] = *(s_c3 -> ptr_nb_mots_prez);
		strcpy((char *) get_keys_c3(s_c3)[ind], (char *) key);
		
        if (*((s_c3 -> struct_score) -> tailleActuelle) < (s_c3 -> struct_score) -> tailleTab) {
            *((s_c3 -> struct_score) -> tailleActuelle) += 1;
        }
    }
}

/*
    Retournent l'indice d'insertion d'une valeur
        ( getIndexInsertionC3_struc -> valeur = s_c3 -> *(ptr_nb_mots_prez)
        getIndexInsertionValueC3 -> valeur = value )
    dans le tableau de nombres de mots présents
*/
int getIndexInsertionC3_struc(struct_c3 *s_c3) {
    return getIndexInsertionValueC3(get_tab_nb_mots(s_c3), get_tab_tailleActuelle(s_c3), *(s_c3 -> ptr_nb_mots_prez));
}

int getIndexInsertionValueC3(int *tab_score_c3, int tailleTab, int value) {
    int ind = 0;
    while ((ind < tailleTab) && (tab_score_c3[ind] > value)) {
        ++ind;
    }
    return ind;
}

/*
    affiche les meilleures clefs avec le message decrypté
*/
void affiche_meilleures_clefs_c3(struct_c3 *sc3, char *msgCrypte, off_t lenMsg, int nbClefsAffichee) {
    if (sc3) {
        printf("Voici les scores et la traduction des %d meilleures clefs (de même score)\n", nbClefsAffichee);

        char *msgUncrypted;
        int indScore = 0; // afficher le premier score

        int tailleActuelle = get_tab_tailleActuelle(sc3);
        double bestScore = get_tab_nb_mots(sc3)[0];

        char *bestKeyCur = (char *) get_keys_c3(sc3)[0];
        int bestScoreCur = get_tab_nb_mots(sc3)[0];

        while ((indScore < tailleActuelle) && indScore < nbClefsAffichee && bestScoreCur == bestScore) {
            msgUncrypted = encrypt_decrypt_xorMSG(msgCrypte, bestKeyCur, lenMsg);

            printf("\nMessage : %s\ndécrypté avec la meilleure clef (\"%s\" : score %d)\n", msgUncrypted, bestKeyCur, bestScoreCur);

            free(msgUncrypted);
            printf("\n");
            ++indScore;
            bestKeyCur = (char *) get_keys_c3(sc3)[indScore];
            bestScoreCur = get_tab_nb_mots(sc3)[indScore];
        }
    }   
}


/*
    ecrit les scores de nombres de mots presents (c3)
    faite pour etre utilisee par des threads (acces parallele a un fichier)

    Ecrit dans un fichier (généralement un log file) 
    une chaine de caracteres de cette forme
    "clef : 1234  nombre de mots présents dans le dictionnaire : 100"
*/
void ecritClefScore_c3(int fdFile, unsigned char *key, int nbMotsPrez) {
    if (fdFile != -1) {
        if (pthread_mutex_lock(&MUTEX_ECRITURE_SCORE) != 0) {
            pError(NULL, "Erreur prise token mutex ecriture file", 4);
        }

        int lenKey = strlen((const char *) key);
        // clef : 1234  nombre de mots présents dans le dictionnaire : 100
        size_t tailleAllouee = lenKey + strlen("clef : \tnombre de mots présents dans le dictionnaire : ") + 10 + 2;
        char *textToWrite = malloc(tailleAllouee);
        pError(textToWrite, "Erreur allocation memoire", 4);

        int ret = snprintf(textToWrite, tailleAllouee, 
                   "clef : %s\tnombre de mots présents dans le dictionnaire : %d\n", 
                   key, nbMotsPrez);
        if (ret < 0 || (size_t)ret >= tailleAllouee) {
            free(textToWrite);
            pError(NULL, "Erreur snprintf c3 ecritClefScore", 4);
        }
        if (write(fdFile, textToWrite, strlen(textToWrite)) == -1) {
            close(fdFile);
            pError(NULL, "Error writing to file", 4);
        }

        free(textToWrite);

        if (pthread_mutex_unlock(&MUTEX_ECRITURE_SCORE) != 0) {
            pError(NULL, "Erreur don token mutex ecriture file", 4);
        }
    }
}

/*
    pareil mais pas pour les threads
*/
void ecritTab_c3(int *tab, int nbElems,  unsigned char **keys, FILE *file) {
    if (file) {
        fprintf(file, "\nscores c3 : \n");
        for (int i = 0 ; i < nbElems ; ++i) {
            fprintf(file, "%s : nombres de mots existants : %d\n", keys[i], tab[i]);
        }
        fprintf(file, "\n\n");
    }
}

//--------------------------------------------------------------------------------------------------
/*
    Fonctions indépendantes de recherches et calcul du nombre
    de mots présents dans le dictionnaire dico
*/

/*
    Retourne et met (si non null) dans nbMotsPrez
    le nombre de mots présents du message msg dans dico
*/
int traiteMsgClefC3(char *msg, int *nbMotsPrez, dictionnary *dico) {
    int nbMots = 0;
    char **mots = wordsArrayFromText(msg, strlen(msg), &nbMots);
    int nbMotsPresent = nbMotsPresents(mots, nbMots, dico);
    if (nbMotsPrez) {
        *nbMotsPrez = nbMotsPresent;
    }

    freeTabs((void ***) &mots, nbMots);

    return nbMotsPresent;
}

/*
    Retourne à partir d'un tableau de mots le nombre de mots
    présents dans le dictionnaires
    le retour est donc forcément <= à nbMots
*/
int nbMotsPresents(char **mots, int nbMots, dictionnary *dico) {
    int nbMotsPrez = 0;
    for (int i = 0 ; i < nbMots ; ++i) {
        if (find_word(dico, mots[i])) {
            ++nbMotsPrez;
        }
    }

    return nbMotsPrez;
}


/*
    Functions to transform a text to an array of words

    toto (and titi) go ->tothe market

    return [toto, and, titi, go, tothe, market]
*/
char **wordsArrayFromText(char *text, off_t lenText, int *nbMot) {
    int arrayLen = 100; // 100 words at the beginning
    char **wordsArray = malloc(sizeof(char *) * arrayLen);
    pError(wordsArray, "Erreur allocation memoire", 3);

    int indWord = 0;
    off_t indText = 0;
    *nbMot = 0; // il faut réassigner la valeur au cas ou !

    while (indText < lenText) {
        if (*nbMot == arrayLen) {
            arrayLen *= 2;
            wordsArray = realloc(wordsArray, sizeof(char *) * arrayLen);
            pError(wordsArray, "Erreur allocation memoire", 3);
        }
        wordsArray[indWord++] = nextWord(text, lenText, &indText);
        indNextWord(text, lenText, &indText);
        ++(*nbMot);
    }

    return wordsArray;
}

/*
    Retourne le prochain mot à partir du curseur indText
    Un mot est composé uniquement de caractères alphabétiques
*/
char *nextWord(char *text, off_t lenText, off_t *indText) {
    int lenWord = 50;
    char *word = malloc(sizeof(char) * lenWord);
    pError(word, "Erreur allocation mémoire", 3);
    int indWord = 0;
    char readChar;

    while (((*indText) < lenText) && isalpha(text[(*indText)])) {
        readChar = tolower(text[(*indText)]);
        if (indWord == lenWord) {
            lenWord *= 2;
            word = realloc(word, sizeof(char) * lenWord);
            pError(word, "Erreur allocation mémoire", 3);
        }
        word[indWord++] = readChar;
        ++(*indText);
    }
    if (indWord == lenWord) {
        word = realloc(word, sizeof(char) * (lenWord + 1)); // need the \0
        pError(word, "Erreur allocation mémoire", 3);
    }
    word[indWord] = '\0';

    return word;
}

/*
    Toto (va) au marché

    indText = 0 -> 0
    indText = 4 -> 6

    changes indText to where the next alpha character begins
    should start at a non alpha character
*/
void indNextWord(char *text, off_t lenText, off_t *indText) {
    while (((*indText) < lenText) && (!isalpha(text[*indText]))) {
        ++(*indText);
    }
    return;
}

void test_text_word_splitter(dictionnary *dico) {
    char *text = "Je dois etre decode le plus rapidement possible, idealement en moins de dix minutes";
    off_t lenText = strlen(text);
    int nbMots = 0;
    
    char **words = wordsArrayFromText(text, lenText, &nbMots);
    int nbMotsPrez = 0;

    for (int i = 0 ; i < nbMots ; ++i) {
        if (find_word(dico, words[i])) {
            ++nbMotsPrez;
        }
    }
    printf("Il y a %d mots presents sur %d\n", nbMotsPrez, nbMots);
}

