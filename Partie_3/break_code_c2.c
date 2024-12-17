#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <ctype.h>
#include <string.h>
#include <pthread.h>
#include <float.h>
#include <fcntl.h>
#include "./crackage.h"
#include "../utilitaire/utiL.h"
#include "../Partie_1/chiffrement.h"
#include "./break_code_c1.h"
#include "./break_code_c2.h"
#include "break_code_c2_c3.h"
#include "mutex.h"

extern float stat_thFr[26];
extern float stat_thEn[26];


/*
    msgAndTaille : le msg crypté, décrypté, sa taille
    distance la distance de la clef actuelle
    tab_meilleures_freq_lettres le tableau des meilleurs scores (0 = excellent)
    ptr_tailleActuelleTab le nombre total de score actuel
    tab_keys_correspondantes le tableau des clefs à l'indice 
        correspondant à tabMeilleurScore
*/
struct struct_c2 {
    struct sMsgAndTaille *msgAndTaille;
    int tailleScoreTab;
    int lenKey;

    int *ptr_tailleActuelleTab;
    float *tab_meilleures_freq_lettres;
    unsigned char **tab_keys_correspondantes;

    float *stats;

    float *distance;

    int fdScoreOut;
};


int break_code_c2(char *file_in, float *stats, char *score_out, int keyLength, char *logFileName) {
    off_t tailleMsg = 0;
    char *msgCrypted = ouvreEtLitFichier(file_in, &tailleMsg);

    int fdScoreOut = -1;
    FILE *logFile = NULL;

    init_params(score_out, &fdScoreOut, logFileName, &logFile);

    struct_c2 *s_c2 = init_struct_c2(msgCrypted, tailleMsg, TAILLE_TAB_SCORE, keyLength, stats, fdScoreOut);
    stC2_C3 *sc2c3 = init_stC2_C3(s_c2, NULL);

    printf("Attaque brutale avec dico : c1, c2 enchaînées...\n");

    unsigned long nbKeys;
    appelClefsFinales(file_in, keyLength, (void *) sc2c3, functorC2, logFileName, true, &nbKeys);

    printf("Début de C2...\n");
    printf("Fin de C2 : meilleure clef = %s (score : %.2f)\n", get_meilleur_clef_c2(s_c2), get_meilleur_score_c2(s_c2));

    printf("Nombre de clefs = %ld\n", nbKeys);

    int nbScoreAffichage = 50;
    printf("Voici les %d meilleurs clefs, avec leur score (0 = parfait) : \n", nbScoreAffichage);
    for (int i = 0 ; i < nbScoreAffichage ; ++i) {
        printf("key : %s ; score : %.2f\n", (s_c2 -> tab_keys_correspondantes)[i], (s_c2 -> tab_meilleures_freq_lettres)[i]);
    }

    affiche_meilleures_clefs_c2(s_c2, msgCrypted, tailleMsg, 1);

    ecritTab_c2(s_c2 -> tab_meilleures_freq_lettres, *(s_c2 -> ptr_tailleActuelleTab), s_c2 -> tab_keys_correspondantes, logFile);

    destruct_stC2_C3(&sc2c3);
    destroy_params(&fdScoreOut, &logFile);
	free(msgCrypted);

    return 0;
}

//---------------------------------------------------------------------------------------------------------------------
/*
    traitement de la clef en cours
*/
void functorC2(unsigned char *key, void *argStruc) {
    struct_c2 *varStructC2 = ((stC2_C3 *) argStruc) -> s_c2;

    // recoit un message codé, le décrypte avec la clef
    translateMsg(key, (void *) (varStructC2 -> msgAndTaille));
    traiteMsgClefC2((varStructC2 -> msgAndTaille) -> msgUncrypted, varStructC2 -> distance, varStructC2 -> stats);

    int inser = getIndexInsertionC2_struc(varStructC2);
    ajouteScoreC2(varStructC2, key, inser);

    ecritClefScore_c2(varStructC2 -> fdScoreOut, key, *(varStructC2 -> distance));

    free((void *) ((varStructC2 -> msgAndTaille) -> msgUncrypted));
}

/*
    assigne une distance a une clef 
    (par rapport aux fréquences de lettres d'une langue)
*/
float traiteMsgClefC2(char *msg, float *distance, float *stats) {
    float *freqMsg = freq(msg, (int) strlen(msg));
	float dist = distanceFreqs(stats, freqMsg);
	if (distance) {
		*distance = dist;
	}
    
	free(freqMsg);
    return dist;
}


//---------------------------------------------------------------------------------------------------------------------
/*
    creation/ destruction de la structure struct_c2
    et getters sur ses attributs
*/
struct_c2 *init_struct_c2(char *msgCrypted, off_t tailleMsgCrypted, int tailleTab, int keyLen, float *stat, int fdScore) {
    struct_c2 *s_c2 = malloc(sizeof(struct_c2));
    pError(s_c2, "Erreur allocation memoire", 2);

    s_c2 -> msgAndTaille = malloc(sizeof(sMsgAndTaille));
    pError(s_c2 -> msgAndTaille, "Erreur allocation memoire", 2);
    (s_c2 -> msgAndTaille) -> msg = malloc(tailleMsgCrypted);
    pError((s_c2 -> msgAndTaille) -> msg, "Erreur allocation memoire", 2);
    memcpy((s_c2 -> msgAndTaille) -> msg, msgCrypted, tailleMsgCrypted);
    
    (s_c2 -> msgAndTaille) -> lenMsg = tailleMsgCrypted;

    s_c2 -> tailleScoreTab = tailleTab;

    s_c2 -> tab_meilleures_freq_lettres = malloc(sizeof(float) * tailleTab);
    pError(s_c2 -> tab_meilleures_freq_lettres, "Erreur allocation memoire", 2);

    s_c2 -> tab_keys_correspondantes = malloc(sizeof(char *) * tailleTab);
    pError(s_c2 -> tab_keys_correspondantes, "Erreur allocation memoire", 2);

    for (int i = 0 ; i < tailleTab ; ++i) {
        (s_c2 -> tab_meilleures_freq_lettres)[i] = FLT_MAX;

        (s_c2 -> tab_keys_correspondantes)[i] = malloc(keyLen + 1);
        pError((s_c2 -> tab_keys_correspondantes)[i], "Erreur allocation mémoire", 2);
    }

    s_c2 -> stats = stat;

    s_c2 -> distance = malloc(sizeof(float));
    pError(s_c2 -> distance, "Erreur allocation memoire", 2);
    *(s_c2 -> distance) = DBL_MAX;

    s_c2 -> ptr_tailleActuelleTab = malloc(sizeof(int));
    pError(s_c2 -> ptr_tailleActuelleTab, "Erreur allocation memoire", 2);
    *(s_c2 -> ptr_tailleActuelleTab) = 0;

    s_c2 -> lenKey = keyLen;

    s_c2 -> fdScoreOut = fdScore;

    return s_c2;
}

void destruct_struct_c2(struct_c2 **s_c2) {
    if (s_c2) {
        free(((*s_c2) -> msgAndTaille) -> msg);
        (*s_c2) -> msgAndTaille -> msg = NULL;
        free((void *) ((*s_c2) -> msgAndTaille));
        (*s_c2) -> msgAndTaille = NULL;

        for (int i = 0 ; i < (*s_c2) -> tailleScoreTab ; ++i) {
            free((void *) ((*s_c2) -> tab_keys_correspondantes)[i]);
            ((*s_c2) -> tab_keys_correspondantes)[i] = NULL;
        }
        free((void *) (*s_c2) -> tab_meilleures_freq_lettres);
        (*s_c2) -> tab_meilleures_freq_lettres = NULL;
        free((void *) (*s_c2) -> tab_keys_correspondantes);
        (*s_c2) -> tab_keys_correspondantes = NULL;

        free((void *) (*s_c2) -> ptr_tailleActuelleTab);
        (*s_c2) -> ptr_tailleActuelleTab = NULL;
        free((void *) (*s_c2) -> distance);
        (*s_c2) -> distance = NULL;

        free((void *) (*s_c2));
        *s_c2 = NULL;
    }
}

struct_c2 *copy_s_c2(struct_c2 *to_copy) {
    struct_c2 *copied = NULL;
    if (to_copy) {
        copied = init_struct_c2((to_copy -> msgAndTaille) -> msg, (to_copy -> msgAndTaille) -> lenMsg, to_copy -> tailleScoreTab, to_copy -> lenKey, to_copy -> stats, to_copy -> fdScoreOut);
    }
    return copied;
}

unsigned char **get_keys_s_c2(struct_c2 *s_c2) {
    if (s_c2) {
        return s_c2 -> tab_keys_correspondantes;
    }
    return NULL;
}

int get_taille_tab_s_c2(struct_c2 *s_c2) {
    if (s_c2) {
        return *(s_c2 -> ptr_tailleActuelleTab);
    }
    return -1;
}

float get_meilleur_score_c2(struct_c2 *s_c2) {
    if (s_c2) {
        return (s_c2 -> tab_meilleures_freq_lettres)[0];
    }
    return -1;
}

unsigned char *get_meilleur_clef_c2(struct_c2 *s_c2) {
    if (s_c2) {
        return (s_c2 -> tab_keys_correspondantes)[0];
    }
    return NULL;
}

int get_taille_actuelle_tab_s_c2(struct_c2 *s_c2) {
    return *(s_c2 -> ptr_tailleActuelleTab);
}

float *get_meilleur_scores_c2(struct_c2 *s_c2) {
    return s_c2 -> tab_meilleures_freq_lettres;
}

unsigned char **get_meilleur_clefs_c2(struct_c2 *s_c2) {
    return s_c2 -> tab_keys_correspondantes;
}

int get_len_key_c2(struct_c2 *s_c2) {
    return s_c2 -> lenKey;
}

//---------------------------------------------------------------------------------------------------------------------
/*
    gestion des scores
*/
void compile_structs_c2(struct_c2 *to, struct_c2 *from) {
    int indInser;
    for (int i = 0 ; i < *(from -> ptr_tailleActuelleTab) ; ++i) {
        *(to -> distance) = (from -> tab_meilleures_freq_lettres)[i];
        indInser = getIndexInsertionC2_struc(to);

        ajouteScoreC2(to, (from -> tab_keys_correspondantes)[i], indInser);
    }
}

void ajouteScoreC2(struct_c2 *st, unsigned char *key, int ind) {
    if (ind < st -> tailleScoreTab && ind >= 0) {
        for (int i = *(st -> ptr_tailleActuelleTab) ; i > ind ; --i) {
            // décaler tout le tableau vers la droite
            // (copier tous les elements pour en inserer un nouveau)
            if (i < st -> tailleScoreTab && i != 0) { // indices inexistants
                (st -> tab_meilleures_freq_lettres)[i] = (st -> tab_meilleures_freq_lettres)[i - 1];
                strcpy((char *) (st -> tab_keys_correspondantes)[i], (char *) (st -> tab_keys_correspondantes)[i - 1]);
            }
        }
        // insertion du nouvel element
        (st -> tab_meilleures_freq_lettres)[ind] = *(st -> distance);
		strcpy((char *) (st -> tab_keys_correspondantes)[ind], (char *) key);

		if (*(st -> ptr_tailleActuelleTab) < st -> tailleScoreTab) {
            *(st -> ptr_tailleActuelleTab) += 1;
        }
    }
}

int getIndexInsertionC2_struc(struct_c2 *st) {
    return getIndexInsertionValueC2(st, *(st -> distance));
}

int getIndexInsertionValueC2(struct_c2 *st, float value) {
    int ind = 0;
    while ((ind < *(st -> ptr_tailleActuelleTab)) && ((st -> tab_meilleures_freq_lettres)[ind] < value)) {
        ++ind;
    }

    return ind;
}

/*
    affichage des nbClefsAffichee meilleurs scores
    affiche les clefs ayant le meme score
*/
void affiche_meilleures_clefs_c2(struct_c2 *sc2, char *msgCrypte, off_t lenMsg, int nbClefsAffichee) {
    if (sc2) {
        printf("Voici les scores et la traduction des %d meilleures clefs (de même score)\n", nbClefsAffichee);

        char *msgUncrypted;
        int indScore = 0; // afficher le premier score

        int tailleActuelle = get_taille_actuelle_tab_s_c2(sc2);
        float bestScore = get_meilleur_score_c2(sc2);

        char *bestKeyCur = (char *) get_meilleur_clefs_c2(sc2)[indScore];
        float bestScoreCur = get_meilleur_score_c2(sc2);

        while ((indScore < tailleActuelle) && indScore < nbClefsAffichee && bestScoreCur == bestScore) {
            msgUncrypted = encrypt_decrypt_xorMSG(msgCrypte, bestKeyCur, lenMsg);

            printf("\nMessage : %s\ndécrypté avec la meilleure clef (\"%s\" : score %.2f)\n", msgUncrypted, bestKeyCur, bestScoreCur);

            free(msgUncrypted);
            printf("\n");
            ++indScore;
            bestKeyCur = (char *) get_meilleur_clefs_c2(sc2)[indScore];
            bestScoreCur = get_meilleur_scores_c2(sc2)[indScore];
        }
    }   
}

/*
    ecrit les fréquences des lettres (c2)
    faite pour etre utilisee par des threads (acces parallele a un fichier)

    Ecrit dans un fichier (généralement un log file) 
    une chaine de caracteres de cette forme
    "clef : 1234 distance des fréquences réelles et référentielles : 100"
*/
void ecritClefScore_c2(int fdFile, unsigned char *key, float freq_lettres) {
    if (fdFile != -1) {
        if (pthread_mutex_lock(&MUTEX_ECRITURE_SCORE) != 0) {
            pError(NULL, "Erreur prise token mutex ecriture file", 4);
        }

        int lenKey = strlen((const char *) key);
        // clef : 1234  nombre de mots présents dans le dictionnaire : 100
        size_t tailleAllouee = lenKey + strlen("clef : \tdistance des fréquences réelles et référentielles : ") + 10 + 2;
        char *textToWrite = malloc(tailleAllouee);
        pError(textToWrite, "Erreur allocation memoire", 4);

        int ret = snprintf(textToWrite, tailleAllouee, 
                   "clef : %s\tdistance des fréquences réelles et référentielles : %.2f\n", 
                   key, freq_lettres);
        if (ret < 0 || (size_t)ret >= tailleAllouee) {
            free(textToWrite);
            pError(NULL, "Erreur snprintf c2 ecritClefScore", 4);
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

void ecritTab_c2(float *tab, int nbElems, unsigned char **keys, FILE *file) {
    if (file) {
        fprintf(file, "\nscores c2 : \n");
        for (int i = 0 ; i < nbElems ; ++i) {
            fprintf(file, "%s : fréquences des lettres : %f\n", keys[i], tab[i]);
        }
        fprintf(file, "\n\n");
    }
}

//-------------------------------------------------------------------------------------
// Fonctions de calculs des fréquences de lettres
/*
    Renvoit un tableau des fréquences des 26 lettres de l'alphabet
    dans le message msg.
*/
float *freq(char *msg, int msgLen) {
    float *freqs = malloc(sizeof(float) * 26);
    pError(freqs, "Erreur allocation tableau fréquence lettre clef actuelle", 2);
    for (int i = 0 ; i < 26 ; ++i) {
        freqs[i] = 0;
    }

    int nbAlpha = 0;
    char readChar;
    for (int i = 0 ; i < msgLen ; ++i) {
        readChar = msg[i];
        if (isalpha(readChar)) {
            readChar = tolower(readChar);
            ++freqs[indice_lettre(readChar)];
            ++nbAlpha;
        }
    }
    for (int i = 0 ; i < 26 ; ++i) {
        freqs[i] = (freqs[i] / nbAlpha) * 100;
    }

    return freqs;
}

/*
    Retourne l'indice d'une lettre (pour le tableau de fréquence)
*/
int indice_lettre(char lettre) {
    if (lettre >= 'a' && lettre <= 'z') {
        return lettre - 'a';
    }
    return -1; // Si la lettre n'est pas entre 'a' et 'z'
}

/*
    Renvoie la distance de la fréquence message crypté 
    et de la fréquence théorique de la langue cible

    plus c'est proche de 0 plus on est proche de la fréquence théorique
*/
float distanceFreqs(float *freqLanguage, float *decryptedFreq) {
    float distance = 0;

    for (int i = 0 ; i < 26 ; ++i) {
        distance += pow((freqLanguage[i] - decryptedFreq[i]), 2);
    }

    return distance;
}

