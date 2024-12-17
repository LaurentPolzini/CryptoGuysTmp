#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <pthread.h> // mutex ecriture clef dans tableau
#include <assert.h>
#include "mutex.h"

#include "tests_crackage.h"
#include "break_code_c1.h"
#include "break_code_c2.h"
#include "break_code_c3.h"
#include "crackage.h" // all crack functions
#include "../utilitaire/utiL.h" // pError
#include "../utilitaire/uthash.h" // dictionnary

extern float stat_thFr[26];
extern float stat_thEn[26];

int appel_tests(char *msg_crypted_file_name, char *msgCrypted, char *msgClair, off_t tailleMsg, 
    char *key_used, int taille_clef, dictionnary *dico_used, int nbClefsTotal, float *stats_used);

// test c1
bool key_unique(sstock_key *stockage, unsigned char *key);
// test c2
bool test_c2_clef_se_trouve_dans_tab_meilleurs_score(struct_c2 *s_c2, float freqRef, char *key_used);

bool test_c3_clef_est_le_meilleur_score(struct_c3 *s_c3, int nbMotsPrezRef, char *key_used);

struct s_stock_key {
    unsigned char **keys;
    int *ind;
    int nbKeysMax;

    bool *keyAddedUnique;
};


int appel_serie_tests(void) {
    int sommeCodeErr = 0;

    dictionnary *dico_english = NULL;
    char *dico_name_english = "Dicos/english.txt";
    read_and_insert_words(dico_name_english, &dico_english, NULL);

    dictionnary *dico_french = NULL;
    char *dico_name_french = "Dicos/dicoFrSA.txt";
    read_and_insert_words(dico_name_french, &dico_french, NULL);

    char *test_file_name_msgClair = "../Source/ring.txt";
    char *test_file_name = "tests/Source_crypted/1234-ring.txt";

    unsigned char key_used[5] = "1234";
    int taille_clef = 4;
    

    off_t tailleMsg = 0;
    char *msgCrypted = ouvreEtLitFichier(test_file_name, &tailleMsg);
    off_t tailleMsgClair = 0;
    char *msgClair = ouvreEtLitFichier(test_file_name_msgClair, &tailleMsgClair);
    assert(tailleMsg == tailleMsgClair);

    int nbClefsTotal = 100000;

    sommeCodeErr += appel_tests(test_file_name, msgCrypted, msgClair, tailleMsg, (char *) key_used, taille_clef, dico_english, nbClefsTotal, stat_thEn);

    free(msgClair);
    free(msgCrypted);
	
    test_file_name_msgClair = "../Source/msg2.txt";
    test_file_name = "tests/Source_crypted/Clef1-msg2.txt";
    unsigned char secondKey[6] = "Clef1";
    taille_clef = 5;

    msgCrypted = ouvreEtLitFichier(test_file_name, &tailleMsg);
    msgClair = ouvreEtLitFichier(test_file_name_msgClair, &tailleMsgClair);
    assert(tailleMsg == tailleMsgClair);

    nbClefsTotal = 3500000;
    sommeCodeErr += appel_tests(test_file_name, msgCrypted, msgClair, tailleMsg, (char *) secondKey, taille_clef, dico_french, nbClefsTotal, stat_thFr);
	
	free(msgClair);
    free(msgCrypted);
	
    if (sommeCodeErr == 0) {
        printf("Tous les tests sont passés avec succès !!\n");
    } else {
        printf("Il y a un ou plusieurs tests qui ne sont pas passés.\n");
    }
	
	if (pthread_mutex_destroy(&MUTEX_TEST) != 0) {
        pError(NULL, "Erreur creation mutex", 1);
    }
	clear_table(&dico_english);
	clear_table(&dico_french);
	
    return sommeCodeErr;
}

int appel_tests(char *msg_crypted_file_name, char *msgCrypted, char *msgClair, off_t tailleMsg, 
    char *key_used, int taille_clef, dictionnary *dico_used, int nbClefsTotal, float *stats_used) {
    
    int sommeCodeErr = 0;

    printf("\nDébuts des tests sur le texte : %s\n\n", msgClair);

    if (nbClefsTotal < 1000000) {
        printf("Debut du test c1.\n");
        test_c1(msg_crypted_file_name, taille_clef, nbClefsTotal);
        printf("\nFin du test c1.\n");
        printf("\n");
    } else {
        printf("Pas de test c1 : trop de clefs\n");
    }

    printf("Debut du test c2.\n");
    sommeCodeErr += test_c2(msg_crypted_file_name, msgCrypted, msgClair, tailleMsg, taille_clef, stats_used, key_used);
    printf("\nFin du test c2.\n");
    printf("\n");

    printf("Debut du test c3.\n");
    sommeCodeErr += test_c3(msg_crypted_file_name, msgCrypted, msgClair, tailleMsg, key_used, taille_clef, dico_used);
    printf("\nFin du test c3.\n");
    printf("\n");

    printf("Debut du test all.\n");
    sommeCodeErr += test_all(msg_crypted_file_name, msgCrypted, msgClair, tailleMsg, key_used, taille_clef, dico_used, stats_used);
    printf("\nFin du test all.\n");
    printf("\n");

    printf("Fin des tests.\n");

    return sommeCodeErr;
}

sstock_key *init_struct_stock_key(int tailleKey, int nbKeyMax) {
    sstock_key *stockage = malloc(sizeof(sstock_key));
    pError(stockage, "Erreur allocation mémoire", 1);
    stockage -> ind = malloc(sizeof(int));
    pError(stockage -> ind, "Erreur allocation mémoire", 1);

    stockage -> keyAddedUnique = malloc(sizeof(bool));
    pError(stockage -> keyAddedUnique, "Erreur allocation mémoire", 1);

    stockage -> keys = malloc(sizeof(unsigned char *) * nbKeyMax);
    pError(stockage -> keys, "Erreur allocation mémoire", 1);
    for (int i = 0 ; i < nbKeyMax ; ++i) {
        (stockage -> keys)[i] = malloc(sizeof(unsigned char) * (tailleKey + 1));
        pError((stockage -> keys)[i], "Erreur allocation mémoire", 1);
    }

    *(stockage -> ind) = 0;

    stockage -> nbKeysMax = nbKeyMax;

    *(stockage -> keyAddedUnique) = true;

    return stockage;
}

void destroy_struct_stock_key(sstock_key **structStock) {
    if (!structStock || !(*structStock)) {
        return;
    }
	
    for (int i = 0; i < (*structStock)->nbKeysMax; ++i) {
        if ((*structStock)->keys[i]) {
            free((*structStock)->keys[i]);
            (*structStock)->keys[i] = NULL;
        }
    }

    free((*structStock)->keys);
    (*structStock)->keys = NULL;

    free((*structStock)->ind);
    (*structStock)->ind = NULL;

    free((*structStock)->keyAddedUnique);
    (*structStock)->keyAddedUnique = NULL;

    free(*structStock);
    *structStock = NULL;
}

int get_nb_keys_max_s_stockage(sstock_key *stock) {
    return stock -> nbKeysMax;
}

int get_actual_index_stockage(sstock_key *stock) {
    return *(stock -> ind);
}

unsigned char **get_keys_stockage(sstock_key *stock) {
    return stock -> keys;
}

bool get_keys_unique(sstock_key *stock) {
    return *(stock -> keyAddedUnique);
}


void insert_key(sstock_key *stock, unsigned char *key) {
    int indInsert = get_actual_index_stockage(stock);
    if (indInsert < (stock -> nbKeysMax)) {
        strcpy((char *) (stock -> keys)[indInsert], (char *) key);
        ++(*(stock -> ind));
    }
    *(stock -> keyAddedUnique) = get_keys_unique(stock) && key_unique(stock, key);
}

/*
    Test l'unicité des clefs

    A ne pas utiliser avec beaucoup de clefs ca prend des plombes
*/
int test_c1(char *test_file_name, int taille_clef, int nbClefsTotal) {
    printf("Le test : unicité de la génération des clefs\n");

    sstock_key *sstock = init_struct_stock_key(taille_clef, nbClefsTotal);

    appelClefsFinales(test_file_name, taille_clef, (void *) sstock, stock_key, NULL, false, NULL);

    if (get_keys_unique(sstock)) {
        printf("\nTest passé ! Toutes les clefs sont uniques.\n");
        destroy_struct_stock_key(&sstock);
        return 0;
    } else {
        printf("\nLe test n'est pas passé ! Les clefs ne sont pas uniques.\n");
        destroy_struct_stock_key(&sstock);
        return 1;
    }
}

bool key_unique(sstock_key *stockage, unsigned char *key) {
    int nb_actual_key = 0;
    for (int i = 0 ; i < get_actual_index_stockage(stockage) ; ++i) {
        if (strcmp((char *) key, (char *) (get_keys_stockage(stockage)[i])) == 0) {
            ++nb_actual_key;
        }
    }
    return nb_actual_key == 1;
}

/*
    la clef reelle se trouve dans le tableau des meilleurs scores
*/
int test_c2(char *msg_crypted_file_name, char *msgCrypted, char *msgClair, off_t tailleMsg, int taille_clef, float *stats_used, char *key_used) {
    struct_c2 *s_c2 = init_struct_c2(msgCrypted, tailleMsg, TAILLE_TAB_SCORE, taille_clef, stats_used, -1);
    stC2_C3 *sc2c3 = init_stC2_C3(s_c2, NULL);

    printf("Le test : la clef utilisée pour chiffrer se trouve dans le tableau de meilleurs scores\n");

    float *frequenceTexte = freq(msgClair, tailleMsg);
    float distanceRef = distanceFreqs(stats_used, frequenceTexte);
    
    appelClefsFinales(msg_crypted_file_name, taille_clef, (void *) sc2c3, functorC2, NULL, true, NULL);

    int retVal = 0;
    if (test_c2_clef_se_trouve_dans_tab_meilleurs_score(s_c2, distanceRef, key_used)) {
        printf("\nTest passé ! La clef se trouve dans les %d meilleurs scores. Le score de la clef colle avec celui du texte aussi.\n", get_taille_tab_s_c2(s_c2));
    } else {
        printf("\nTest pas passé ! La clef ne se trouve pas dans les %d meilleurs scores\n", get_taille_tab_s_c2(s_c2));
        retVal = 1;
    }

    destruct_stC2_C3(&sc2c3);
	free(frequenceTexte);

    return retVal;
}

bool test_c2_clef_se_trouve_dans_tab_meilleurs_score(struct_c2 *s_c2, float freqRef, char *key_used) {
    bool sytrouve = false;
    for (int i = 0 ; i < get_taille_actuelle_tab_s_c2(s_c2) ; ++i) {
        if (strcmp((char *) get_meilleur_clefs_c2(s_c2)[i], key_used) == 0) {
            sytrouve = get_meilleur_scores_c2(s_c2)[i] == freqRef;
        }
    }
    return sytrouve;
}

/*
    la meilleure clef est la clef
*/
int test_c3(char *msg_crypted_file_name, char *msgCrypted, char *msgClair, off_t tailleMsg, char *key_used, 
    int taille_clef, dictionnary *dico_used) {

    struct_c3 *s_c3 = init_struct_c3(msgCrypted, tailleMsg, TAILLE_TAB_SCORE, taille_clef, dico_used, -1);
    stC2_C3 *sc2c3 = init_stC2_C3(NULL, s_c3);

    printf("Le test : la clef au meilleur score est la clef utilisée\n");

    int nbMotsPrezRef = traiteMsgClefC3(msgClair, NULL, dico_used);
    
    unsigned long nbKeys;
    appelClefsFinales(msg_crypted_file_name, taille_clef, (void *) sc2c3, functorC3, NULL, true, &nbKeys);

    int retVal = 0;
    if (test_c3_clef_est_le_meilleur_score(s_c3, nbMotsPrezRef, key_used)) {
        printf("\nTest passé ! La meilleure clef (%s) trouvée est la même que celle utilisée.\n", get_meilleur_clef_c3(s_c3));
    } else {
        printf("\nTest pas passé ! La meilleure clef (%s) trouvée n'est pas la même que celle utilisée (%s).\n", get_meilleur_clef_c3(s_c3), key_used);
        retVal = 1;
    }

    destruct_stC2_C3(&sc2c3);

    return retVal;
}

bool test_c3_clef_est_le_meilleur_score(struct_c3 *s_c3, int nbMotsPrezRef, char *key_used) {
    return ( (strcmp((char *) get_meilleur_clef_c3(s_c3), key_used) == 0) && 
        (get_meilleur_score_c3(s_c3) == nbMotsPrezRef) );
}


/*
    de même que pour c3, la meilleure clef est la clef
*/
int test_all(char *msg_crypted_file_name, char *msgCrypted, char *msgClair, off_t tailleMsg, 
    char *key_used, int taille_clef, dictionnary *dico_used, float *stats_used) {

    float *frequenceTexte = freq(msgClair, tailleMsg);
    float distanceRef = distanceFreqs(stats_used, frequenceTexte);

    int nbMotsPrezRef = traiteMsgClefC3(msgClair, NULL, dico_used);

    struct_c2 *s_c2 = init_struct_c2(msgCrypted, tailleMsg, TAILLE_TAB_SCORE, taille_clef, stats_used, -1);

    // NULL pour pas qu'il y ait de copies de c3
    stC2_C3 *sc2c3 = init_stC2_C3(s_c2, NULL);

    // true pour copier les structures
    appelClefsFinales(msg_crypted_file_name, taille_clef, (void *) sc2c3, functorC2, NULL, true, NULL);

    int retVal = 0;
    if (test_c2_clef_se_trouve_dans_tab_meilleurs_score(s_c2, distanceRef, key_used)) {
        printf("\nTest c2 passé avec succès !\n");
    } else {
        printf("\nTest c2 n'est pas passé ! La clef n'est pas dans les %d meilleurs scores.\n", TAILLE_TAB_SCORE);
        ++retVal;
    }

    struct_c3 *s_c3 = init_struct_c3(msgCrypted, tailleMsg, get_taille_tab_s_c2(s_c2), taille_clef, dico_used, -1);

    traite_clefs_generee_c2(s_c3, s_c2);

    if (test_c3_clef_est_le_meilleur_score(s_c3, nbMotsPrezRef, key_used)) {
        printf("\nTest c3 passé ! La clef est %s.\n", get_meilleur_clef_c3(s_c3));
    } else {
        printf("\nTest c3 n'est pas passé ! La meilleure clef (%s) trouvée n'est pas la même que celle utilisée (%s).\n", get_meilleur_clef_c3(s_c3), key_used);
        ++retVal;
    }

    destruct_stC2_C3(&sc2c3);
	free(frequenceTexte);
	destruct_struct_c3(&s_c3);

    return retVal;
}
