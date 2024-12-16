#ifndef __TEST_CRACKAGE_H__
#define __TEST_CRACKAGE_H__

#include <pthread.h>
#include <sys/types.h>
#include "../utilitaire/utiL.h"

// pour voir si chaque clef générée est unique
typedef struct s_stock_key sstock_key;

int appel_serie_tests(void);

int test_c1(char *test_file_name, int taille_clef, int nbClefsTotal);
int test_c2(char *msg_crypted_file_name, char *msgCrypted, char *msgClair, off_t tailleMsg, int taille_clef, float *stats_used, char *key_used);
int test_c3(char *msg_crypted_file_name, char *msgCrypted, char *msgClair, off_t tailleMsg, char *key_used, 
    int taille_clef, dictionnary *dico_used);
int test_all(char *msg_crypted_file_name, char *msgCrypted, char *msgClair, off_t tailleMsg, 
    char *key_used, int taille_clef, dictionnary *dico_used, float *stats_used);

/*
    getters et insertion d'une clef
*/
void insert_key(sstock_key *stock, unsigned char *key);
unsigned char **get_keys_stockage(sstock_key *stock);
int get_actual_index_stockage(sstock_key *stock);
int get_nb_keys_max_s_stockage(sstock_key *stock);
bool get_keys_unique(sstock_key *stock);

#endif
