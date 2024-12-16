#ifndef __BREAK_CODE_C2_H__
#define __BREAK_CODE_C2_H__

int break_code_c2(char *file_in, float *stats, char *score_out, int keyLength, char *logFileName);

void functorC2(unsigned char *key, void *argStruc);
float traiteMsgClefC2(char *msg, float *distance, float *stats);

/*
    creation / destruction / getters struct_c2
*/
typedef struct struct_c2 struct_c2;

struct_c2 *init_struct_c2(char *msgCrypted, off_t tailleMsgCrypted, int tailleTab, int keyLen, float *stat, int fdScore);
struct_c2 *copy_s_c2(struct_c2 *to_copy);
void destruct_struct_c2(struct_c2 **s_c2);

unsigned char **get_keys_s_c2(struct_c2 *s_c2);
int get_taille_tab_s_c2(struct_c2 *s_c2);
int get_taille_actuelle_tab_s_c2(struct_c2 *s_c2);

float get_meilleur_score_c2(struct_c2 *s_c2);
unsigned char *get_meilleur_clef_c2(struct_c2 *s_c2);

float *get_meilleur_scores_c2(struct_c2 *s_c2);
unsigned char **get_meilleur_clefs_c2(struct_c2 *s_c2);

int get_len_key_c2(struct_c2 *s_c2);

//--------------------------------------------------------------------------------------------------------
/*
    gestion des scores de struct_c2
*/
int getIndexInsertionC2_struc(struct_c2 *st);
int getIndexInsertionValueC2(struct_c2 *st, float value);
void ajouteScoreC2(struct_c2 *st, unsigned char *key, int ind);

// met dans to les meilleurs scores de from
void compile_structs_c2(struct_c2 *to, struct_c2 *from);

// affiche les clefs de meme score (nbClefsAffichee au maximum)
void affiche_meilleures_clefs_c2(struct_c2 *sc2, char *msgCrypte, off_t lenMsg, int nbClefsAffichee);

/*
    ecrit ""clef : 1234 distance des fréquences réelles et référentielles : 100""
    dans fdFile
*/
// pour les threads
void ecritClefScore_c2(int fdFile, unsigned char *key, float freq_lettres);

// pas pour les threads
void ecritTab_c2(float *tab, int nbElems, unsigned char **keys, FILE *file);

//--------------------------------------------------------------------------------------------------------
/*
    Fonctions propres à c2 : calcul des fréquences des lettres
*/
// renvoie la fréquence des lettres d'un message
float *freq(char *msg, int msgLen);
/*
    Renvoie la distance de la fréquence message crypté 
    et de la fréquence théorique de la langue cible

    plus c'est proche de 0 plus on est proche de la fréquence théorique
*/
float distanceFreqs(float *freqLanguage, float *decryptedFreq);


// retourne l'indice d'une lettre (0 -> 26)
// ou -1 si c'est n'est pas une lettre
int indice_lettre(char lettre);

#endif
