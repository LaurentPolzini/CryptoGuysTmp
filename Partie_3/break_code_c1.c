#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <math.h>
#include "./crackage.h"
#include "./Pile.h"
#include "./break_code_c1.h"
#include "Tree.h"
#include "../utilitaire/utiL.h"

//int nbCara(unsigned char tabCar[]);
void cpyChaine(unsigned char *dest, unsigned char *from);
//int nbClefsTotal(unsigned char **carCandParIndice, int len_key);

void ajouteCandidats(unsigned char ***clefs, unsigned long *nbClefs, int *tailleClefs, unsigned char *carCandidats, int nbCarCand);
void *ajouteCandidatsThreads(void *threadIntel);

// pour free plus vite les clefs
void *threadFreeSegment(void *info);
   
typedef struct {
    unsigned char *carCandidats;
    int nbCarCand;
    unsigned char **clef;
    unsigned long debutSegment;
    unsigned long finSegment;
    unsigned char **newKeys;
    unsigned long tailleclef;
} threadInfoKey;

typedef struct {
    unsigned char ***arr;
    unsigned long debut, fin;
} threadFreeInfo;

/*
int main(int argc, char *argv[]) {
    (void) argc;
    (void) argv;
    
    if (argc != 4) {
        perror("Il faut 4 args : exec file_in key_len file_out");
        exit(1);
    }
    
    //char *file_in = argv[1];
    int key_len = atoi(argv[1]);
    //char *file_out = argv[3];

    char **clefCand = clefsCandidatesFinales("", key_len);

    for (unsigned long i = 0 ; i < (sizeof(clefCand) / key_len) ; ++i) {
        printf("%s\n", clefCand[i]);
    }

    free(clefCand);
    
    return 0;
}
*/
char *ouvreEtLitFichier(char *file_in) {
    (void) file_in;
    return "";
}

int break_code_c1(char *file_in, int keyLength, char *file_out) {
    // ouvrir fichier file_in

    //char **clef = clefsCandidatesFinales();
    (void) file_in;
    (void) keyLength;
    (void) file_out;

    return 0;
}

/*
    recoit un char xoré entre un char du message et un char de clef possible
    retourne si ce char est bien un caractere valide en ASCII

    attention : je vérifie les guillements américain et non francais
        pas << >> mais " "
*/
bool estCaractereValideASCII(unsigned char *charDechiffre) {
    // caracteres valides 
    char charValides[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789,?;.:()[]\"-/{} ";

    return charDechiffre[0] != '\0' && (strstr(charValides, (const char *) charDechiffre) != NULL);
}

/*
    Fait un xor de chaque caractere de charSet avec carChiffre
    et filtre toutes les valeurs non possibles
    (La fonction est alors réutilisable avec les caracteres filtrés et le caractere
        d'indice i * k)
        -> clef[0] code caractere 0 + k * longueur de la clef
        donc un caractere candidat de msg[0] est candidat aussi pour msg[2] par exemple
    
    retourne les caracteres candidats pour un caractere du msg chiffre
*/
unsigned char *caracteresPossibles(unsigned char *charSet, unsigned char carChiffre) {
    unsigned char *charClefPossibles = malloc(strlen((const char *) charSet) + 1);
    if (!charClefPossibles) {
        perror("Erreur allocation memoire caracteres possibles");
        exit(1);
    }
    cpyChaine(charClefPossibles, charSet);

    int indCharPossible = 0;

    // le caractere xoré
    unsigned char strCarDechiffre[2];
    strCarDechiffre[1] = '\0';

    // tous les caracteres de charset possible
    // autrement dit, tous les caracteres alphanumerique et ponctuations
    for (unsigned long i = 0 ; i < strlen((char *) charSet) ; ++i) {
        strCarDechiffre[0] = carChiffre ^ charSet[i];

        if (estCaractereValideASCII(strCarDechiffre)) {
            charClefPossibles[indCharPossible++] = charSet[i];
        }
    }
    charClefPossibles[indCharPossible] = '\0';

    return charClefPossibles;
}

/*
    len_key : longueur réelle de clef (commence à 1)

    retourne les caracteres candidats d'un caractere d'un certain indice du msg codé

    autrement dit:
        indice = 0
        len_key = 3
        msg = "s(/1&!"

        clef[0] a code s et 1
        ainsi les caracteres candidats alphanumérique sont filtrés avec s
        puis avec 1
    
    ainsi, au debut on a "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789"
    puis apres avec s disons qu'on obtient "acdef"
    puis avec 1 "ac"
*/
unsigned char *caracteresCandidatIndKey(unsigned char *msgCode, int tailleMsgCode, int indice, int len_key) {
    // cara possible d'une clef (gen_key ne genere que des caracteres alphanumérique)
    unsigned char *caraCandidat = malloc(63 * sizeof(unsigned char));
    if (!caraCandidat) {
        perror("Erreur allocation memoire tableau par indice");
        exit(1);
    }
    strcpy((char *) caraCandidat, "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789");


    unsigned long indCur = indice;
    unsigned char *carTemp;

    while (indCur < (unsigned long) tailleMsgCode) {
        carTemp = caracteresPossibles(caraCandidat, msgCode[indCur]);
        cpyChaine(caraCandidat, carTemp);
        free(carTemp);
        indCur += len_key;
    }
    
    return caraCandidat;
}

/*
    Recoit un msg codé et la longueur d'une clef

    Retourne les clefs candidates par indice
    c-a-d:
        — clef [0] ∈ [′5′,′ 6′,′ 8′]
        — clef [1] ∈ [′1′]
        — clef [2] ∈ [′0′,′ 2′]
*/
unsigned char **caracteresCandidatsParIndice(unsigned char *msgCode, int len_key) {
    // les clefs candidates finales
    unsigned char **clefCandidates = malloc(len_key * sizeof(unsigned char *));
    if (!clefCandidates) {
        perror("Erreur allocation memoire tableau de caractères candidats");
        exit(1);
    }
    int tailleMsgCod = sizeof(msgCode); // / sizeof(unsigned char) qui vaut 1
    
    for (int i = 0 ; i < len_key ; ++i) {
        // TODO THREADS ICI; pas sur, c'est assez rapide
        clefCandidates[i] = caracteresCandidatIndKey(msgCode, tailleMsgCod, i, len_key);
    }
    for (int i = 0 ; i < len_key ; ++i) {
        printf("Possibilité par indice : %s\n", clefCandidates[i]);
    }

    return clefCandidates;
}

/*
    Retourne le prod carthesien des caracteresCandidatsParIndice
        — clef [0] ∈ [′5′,′ 6′,′ 8′]
        — clef [1] ∈ [′1′]
        — clef [2] ∈ [′0′,′ 2′]

        renvoie clef ∈ 
        [”510”, 
        ”512”, 
        ”610”, 
        ”612”, 
        ”810”, 
        ”812”]

        clef[0] composée de caraCandidat[0][0] caraCandidat[1][0] et caraCandidat[2][0]
        caraCandidat[0][0] apparait (nbClef / len(clefs candidates)) fois

        Utilise les arbres N-aire
*/
unsigned char **clefsFinales(char *msgCode, int len_key, unsigned long *nbClefs) {
    unsigned char **carCandParIndice = caracteresCandidatsParIndice((unsigned char *) msgCode, len_key);

    unsigned char **clefs = malloc(sizeof(unsigned char *));
    pError(clefs, "Erreur creation tableau de clefs de base", 1);
    clefs[0] = malloc(1);
    pError(clefs[0], "Erreur allocation chaine de caractere vide", 1);
    strcpy((char *) clefs[0], "\0");
    
    unsigned long nbClefsTMP = 1;
    int tailleClefs = 0;
    for (int i = 0 ; i < len_key ; ++i) {
        ajouteCandidats(&clefs, &nbClefsTMP, &tailleClefs, carCandParIndice[i], (int) strlen((const char *) carCandParIndice[i]));
    }
    /*
    Tree *t = createTree();
    for (int i = 0 ; i < len_key ; ++i) {
        t = addTree(t, carCandParIndice[i], ((int) strlen((const char *) carCandParIndice[i])) );
    }

    *nbClefs = getNbClef(t);
    printf("323 break code c1 Il y a %lu clefs\n", *nbClefs);

    unsigned char **clefs = malloc((*nbClefs) * sizeof(unsigned char *));
    pError((void *) clefs, "Erreur création lot de clefs", 1);

    getClefsCandidates(t, clefs);

    freeDoubleArray(&carCandParIndice, len_key);
    deleteTree(t);
    */

    *nbClefs = nbClefsTMP;
    return clefs;
}

/*
int nbClefsTotal(unsigned char **carCandParIndice, int len_key) {
    int nbClefs = 1;
    for (int i = 0 ; i < len_key ; ++i) {
        nbClefs *= nbCara(carCandParIndice[i]);
    }
    return nbClefs;
}

// equivalent d'un strlen mais pour un tableau : {'a', 'b', '\0'}
// sizeof(tableau) ne compile pas faut que je me penche dessus
int nbCara(unsigned char tabCar[]) {
    unsigned char curCar = tabCar[0];
    int nbCara = 0;

    while (curCar != '\0') {
        curCar = tabCar[nbCara + 1];
        ++nbCara;
    }
    return nbCara;
}
*/

void cpyChaine(unsigned char *dest, unsigned char *from) {
    strcpy((char *) dest, (const char *) from);
}

void freeDoubleArray(unsigned char ***arr, unsigned long len) {
    if (!(arr && *arr)) {
        return;
    }
    /*
    for (unsigned long i = 0 ; i < len ; ++i) {
        free((void *) (*arr)[i]);
        (*arr)[i] = NULL;
    }
    */
    unsigned long nbSeg = (unsigned long) sqrt(len);
    unsigned long tailleSeg = len / nbSeg + (len % nbSeg != 0 ? 1 : 0);

    pthread_t *thid = malloc(nbSeg * sizeof(pthread_t));
    pError(thid, "Erreur creation tableau thread free", 1);
    threadFreeInfo *ti = malloc(nbSeg * sizeof(threadFreeInfo));
    pError(ti, "Erreur creation tableau informations pour threads free", 1);
    
    for (unsigned long i = 0 ; i < nbSeg ; ++i) {
        ti[i].arr = arr;
        ti[i].debut = i * tailleSeg;
        ti[i].fin = (i + 1) * tailleSeg > len ? len : (i + 1) * tailleSeg;
        pthread_create(&thid[i], NULL, threadFreeSegment, &ti[i]);
    }
    for (unsigned long i = 0 ; i < nbSeg ; ++i) {
        pthread_join(thid[i], NULL);
    }
    
    free((void *) *arr);
    *arr = NULL;

    free(thid);
    free(ti);
}

void *threadFreeSegment(void *info) {
    threadFreeInfo ti = *((threadFreeInfo *) info);

    for (unsigned long i = ti.debut ; i < ti.fin ; ++i) {
        if ((*(ti.arr))[i]) {
            free((void *) (*(ti.arr))[i]);
            (*(ti.arr))[i] = NULL;
        }
    }

    pthread_exit(NULL);
}

/*

// pour tester les différences de temps
// faut juste le remettre a jour, je crois qu'il croit que le nb de caracteres candidats
// par indice de clef sont de meme taille, d'ou le n

void produit_cartesienChatGPT(char *tableau[][10], int taille[], int n) {
    int indices[n]; // Indices actuels pour chaque position dans la chaîne
    memset(indices, 0, sizeof(indices)); // Initialiser tous les indices à 0
    
    while (1) {
        // Afficher une combinaison
        for (int i = 0; i < n; i++) {
            printf("%c", tableau[i][indices[i]][0]); // Afficher le symbole actuel
        }
        printf("\n");
        
        // Incrémenter les indices
        int pos = n - 1;
        while (pos >= 0) {
            indices[pos]++; // Incrémenter l'indice de la position actuelle
            if (indices[pos] < taille[pos]) { // Si l'indice est dans la limite
                break;
            }
            indices[pos] = 0; // Remettre à zéro et passer à la position précédente
            pos--;
        }
        
        // Si tous les indices sont remis à zéro, toutes les combinaisons sont générées
        if (pos < 0) {
            break;
        }
    }
}
*/

threadInfoKey createThreadInfo(unsigned char *carCandidats, int nbCarCand, unsigned char **clef, unsigned long debutSegment,
    unsigned long finSegment, unsigned char **newKeys, unsigned long tailleclef) {
        threadInfoKey ti;
        ti.carCandidats = carCandidats;
        ti.nbCarCand = nbCarCand;
        ti.clef = clef;
        ti.debutSegment = debutSegment;
        ti.finSegment = finSegment;
        ti.newKeys = newKeys;
        ti.tailleclef = tailleclef;

        return ti;
    }

/*
    Dans clefs les clefs pour l'instant 
    soit les caracteres candidats 
    [123]
    [45]

    on ajoute [123] ce qui fait les clefs ["1", "2", "3"]
    puis on ajoute [45] ce qui donne ["14", "15", "24", "25", "34", "35"]
    
    on ajoute a chaque clefs actuels les nouveaux caracteres candidats
        -> double boucle 
*/
void ajouteCandidats(unsigned char ***clefs, unsigned long *nbClefs, int *tailleClefs, unsigned char *carCandidats, int nbCarCand) {
    // initialise les nouvelles infos : 
    // nouveaux nb de clefs et nouveau tableau de clefs en consequence
    unsigned long newNbClefs = (*nbClefs) * nbCarCand;;
    unsigned char **newKeys = malloc(newNbClefs * sizeof(unsigned char *));
    pError(newKeys, "Erreur creation nouveau tableau clefs", 0);

    unsigned long nbSegment = (unsigned long) sqrt(*nbClefs);
    unsigned long tailleSegment = *nbClefs / nbSegment + (*nbClefs % nbSegment != 0 ? 1 : 0);
    
    pthread_t *thid = malloc(nbSegment * sizeof(pthread_t));
    pError(thid, "Erreur creation tableau thread clefs", 1);
    threadInfoKey *ti = malloc(nbSegment * sizeof(threadInfoKey));
    pError(ti, "Erreur creation tableau informations pour threads clefs", 1);

    unsigned long fin;
    
    for (unsigned long i = 0 ; i < nbSegment ; ++i) {
        fin = (i + 1) * tailleSegment > *nbClefs ? *nbClefs : (i + 1) * tailleSegment;

        ti[i] = createThreadInfo(carCandidats, nbCarCand, *clefs, i * tailleSegment, fin, newKeys, *tailleClefs);
        pthread_create(&thid[i], NULL, ajouteCandidatsThreads, (void *) &(ti[i]));
    }
    for (unsigned long i = 0 ; i < nbSegment ; ++i) {
        pthread_join(thid[i], NULL);
    }
    
    freeDoubleArray(clefs, *nbClefs);
    *clefs = newKeys;
    *nbClefs = newNbClefs;
    *tailleClefs += 1;

    free(thid);
    free(ti);

    return;
}

/*
    un thread est crée par segment de clefs
        son role est d'ajouter tous les nouveaux caracteres candidats
        a chaque clef du segment

        => threadInfo doit etre constitué de
        unsigned char *carCandidats; // les caracteres candidats
        int nbCarCand;
        unsigned char **clef; // le tableau de clefs
        int debutSegment; // debut des clefs qu'on traitera
        int finSegment; // fin des clefs qu'on traitera
        unsigned char **newKeys; // ou ecrire les nouvelles clefs
        int tailleclef; // le nombre de clefs actuelles
*/
void *ajouteCandidatsThreads(void *threadIntel) {
    threadInfoKey ti = *((threadInfoKey *) threadIntel);
    unsigned long indTmp;

    for (unsigned long i = ti.debutSegment ; i < ti.finSegment ; ++i) {
        for (int j = 0 ; j < ti.nbCarCand ; ++j) {
            indTmp = (i * ti.nbCarCand) + j;
            ti.newKeys[indTmp] = malloc(ti.tailleclef + 2); // 2 : nouveau caractere et \0
            pError(ti.newKeys[indTmp], "Erreur creation nouvelle clef", 1);

            // copie le 1 dans les 2 premieres cases
            strcpy((char *) ti.newKeys[indTmp], (const char *) (ti.clef)[i]);
            // met le 4 et le 5 dans les cases 1 et 2 (fin 0 et 1 quoi)
            ti.newKeys[indTmp][ti.tailleclef] = ti.carCandidats[j];
            ti.newKeys[indTmp][ti.tailleclef + 1] = '\0';
        }
    }

    pthread_exit(NULL);
}

