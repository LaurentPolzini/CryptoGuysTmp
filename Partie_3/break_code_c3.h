#ifndef __BREAK_CODE_C3_H__
#define __BREAK_CODE_C3_H__

#include <unistd.h> // off_t
#include "../utilitaire/uthash.h" // dictionnary

struct dictionnary;

// lancements de c3
int break_code_c3(char *file_in, char *dict_file_in, char *score_out, int keyLen, char *logFileName);
int break_code_all_exact_len(char *file_in, char *dict_file_in, char *score_out, int keyLen, char *logFileName);
int break_code_all_max_len(char *file_in, char *dict_file_in, char *score_out, int keyLen, char *logFileName);

//---------------------------------------------------------------------------------------------------------------
// fonctions pour la structure c3
// initialisation / destruction / getters
typedef struct struct_c3 struct_c3;

struct_c3 *init_struct_c3(char *msgCrypted, off_t tailleMsgCrypted, int tailleTab, int keyLen, struct dictionnary *dicO, int fdScore);
struct_c3 *copy_s_c3(struct_c3 *to_copy);
void destruct_struct_c3(struct_c3 **s_c3);

int *get_tab_nb_mots(struct_c3 *s_score);
unsigned char **get_keys_c3(struct_c3 *s_score);

int get_tailleTab(struct_c3 *s_score);
int get_tab_tailleActuelle(struct_c3 *s_score);

int get_meilleur_score_c3(struct_c3 *s_c3);
unsigned char *get_meilleur_clef_c3(struct_c3 *s_c3);

/*
    Gestion d'ajout des scores
*/
int getIndexInsertionC3_struc(struct_c3 *s_c3);
int getIndexInsertionValueC3(int *tab_score_c3, int tailleTab, int value);

void ajouteScoreC3(struct_c3 *s_c3, unsigned char *key, int ind);

void compile_structs_c3(struct_c3 *to, struct_c3 *from);

/*
    ecrit ""clef : 1234  nombre de mots présents dans le dictionnaire : 100""
    dans fdFile
*/
// pour les threads
void affiche_meilleures_clefs_c3(struct_c3 *sc3, char *msgCrypte, off_t lenMsg, int nbClefsAffichee);

// pas pour les threads
void ecritTab_c3(int *tab, int nbElems,  unsigned char **keys, FILE *file);

/*
    ecrit "clef : 1234  nombre de mots présents dans le dictionnaire : 100"
    dans fdFile
*/
void ecritClefScore_c3(int fdFile, unsigned char *key, int nbMotsPrez);

//---------------------------------------------------------------------------------------------------------------
/*
    Fonctions propres à c3
*/
/*
    Traduit un message crypté grace a key
    compte son nombre de mots présents dans le dico
    l'ajoute eventuellement aux meilleurs scores
*/
void functorC3(unsigned char *key, void *argStruc);

int traiteMsgClefC3(char *msg, int *nbMotsPrez, struct dictionnary *dico);


/*
    calcul du nombre de mots
*/
char **wordsArrayFromText(char *text, off_t lenText, int *nbMot);

char *nextWord(char *text, off_t lenText, off_t *indText);

void indNextWord(char *text, off_t lenText, off_t *indText);


#endif
