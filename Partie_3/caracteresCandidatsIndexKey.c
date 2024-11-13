#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include "caracteresCandidatsIndexKey.h"


char ASCII_CHAR_VALIDES[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789'\",?!;.:()\n ";

char ACCENTED_CHAR_VALIDE[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789',?!;.:()\n "
 "éèêàâôîûù";

char UTF8_CHAR_VALIDES[] = 
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"
    "0123456789";


/*
    recoit un char xoré entre un char du message et un char de clef possible
    retourne si ce char est bien un caractere valide en ASCII
*/
bool estCaractereValideASCII(unsigned char charDechiffre) {
    return isalnum(charDechiffre) || ispunct(charDechiffre) || isspace(charDechiffre);
}

bool estCaractereValideUTF8(unsigned char charDechiffre) {
    return strchr(UTF8_CHAR_VALIDES, charDechiffre) != NULL;
}

bool estCaractereValideACCENT(unsigned char charDechiffre) {
    return strchr(ACCENTED_CHAR_VALIDE, charDechiffre) != NULL;
}

/*
    Recoit un msg codé et la longueur d'une clef

    Retourne les clefs candidates par indice
    c-a-d:
        — clef [0] ∈ [′5′,′ 6′,′ 8′]
        — clef [1] ∈ [′1′]
        — clef [2] ∈ [′0′,′ 2′]
*/
unsigned char **caracteresCandidatsParIndice(char *msgCode, off_t tailleMsgCode, int len_key) {
    // les clefs candidates finales
    unsigned char **clefCandidates = malloc(len_key * sizeof(unsigned char *));
    if (!clefCandidates) {
        perror("Erreur allocation memoire tableau de caractères candidats");
        exit(1);
    }
    
    for (int i = 0 ; i < len_key ; ++i) {
        clefCandidates[i] = caracteresCandidatIndKey(msgCode, tailleMsgCode, i, len_key);
    }

    return clefCandidates;
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
unsigned char *caracteresCandidatIndKey(char *msgCode, off_t tailleMsgCode, int indice, int len_key) {
    // cara possible d'une clef (gen_key ne genere que des caracteres alphanumérique)
    unsigned char *caraCandidat = malloc(63 * sizeof(unsigned char));
    if (!caraCandidat) {
        perror("Erreur allocation memoire tableau par indice");
        exit(1);
    }
    strcpy((char *) caraCandidat, "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789");

    unsigned long indCur = indice;
    unsigned char *tmp;

    while ((off_t) indCur < tailleMsgCode) {
        tmp = caracteresPossibles(caraCandidat, msgCode[indCur]);
        free(caraCandidat);
        caraCandidat = tmp;

        indCur += len_key;
    }
    
    return caraCandidat;
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

    int indCharPossible = 0;

    // le caractere xoré avec un caractere de la clef 
    // et tous les caracteres possibles pour cet indice de clef
    unsigned char carDechiffre;

    // tous les caracteres de charset possible
    // autrement dit, tous les caracteres alphanumerique et ponctuations
    for (unsigned long i = 0 ; i < strlen((char *) charSet) ; ++i) {
        carDechiffre = carChiffre ^ charSet[i];

        if (estCaractereValideASCII(carDechiffre)) {
            charClefPossibles[indCharPossible++] = charSet[i];
        }
    }
    charClefPossibles[indCharPossible] = '\0';

    return charClefPossibles;
}




