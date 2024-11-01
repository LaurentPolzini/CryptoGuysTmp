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

int nbCara(unsigned char tabCar[]);
void cpyChaine(unsigned char *dest, unsigned char *from);
int nbClefsTotal(unsigned char **carCandParIndice, int len_key);

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
unsigned char *caracteresCandidatIndKey(unsigned char *msgCode, int indice, int len_key) {
    // cara possible d'une clef (gen_key ne genere que des caracteres alphanumérique)
    unsigned char *caraCandidat = malloc(63 * sizeof(unsigned char));
    if (!caraCandidat) {
        perror("Erreur allocation memoire tableau par indice");
        exit(1);
    }
    strcpy((char *) caraCandidat, "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789");


    unsigned long indCur = indice;
    unsigned char *carTemp;

    while (indCur < strlen((const char *) msgCode)) {
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
    
    for (int i = 0 ; i < len_key ; ++i) {
        // TODO THREADS ICI
        clefCandidates[i] = caracteresCandidatIndKey(msgCode, i, len_key);
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

    return clefs;
}


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

void cpyChaine(unsigned char *dest, unsigned char *from) {
    strcpy((char *) dest, (const char *) from);
}

void freeDoubleArray(unsigned char ***clefs, int len_key) {
    if (!(clefs && *clefs)) {
        return;
    }
    for (int i = 0 ; i < len_key ; ++i) {
        free((void *) (*clefs)[i]);
        (*clefs)[i] = NULL;
    }
    free((void *) *clefs);
    *clefs = NULL;
}



