#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include "./crackage.h"
#include "./NTree.h"
#include "./Queue.h"
#include "./break_code_c1.h"

int nbCara(unsigned char tabCar[]);
void cpyChaine(unsigned char *dest, unsigned char *from);

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
    strcpy((char *) charClefPossibles, (const char *) charSet);
    //cpyChaine(charClefPossibles, charSet);

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
        //strcpy((char *) caraCandidat, (const char *) carTemp);
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

    unsigned char *tmp;
    
    for (int i = 0 ; i < len_key ; ++i) {
        tmp = caracteresCandidatIndKey(msgCode, i, len_key);
        clefCandidates[i] = malloc((strlen((char *) tmp) + 1) * sizeof(unsigned char));
        if (!clefCandidates[i]) {
            perror("Erreur allocation memoire caracteres possibles par indice");
            exit(1);
        }
        cpyChaine(clefCandidates[i], tmp);
        //strcpy((char *) clefCandidates[i], (const char *) tmp);
        free(tmp);
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
unsigned char **clefsCandidatesFinales(char *msgCode, int len_key, int *nbClefs) {
    unsigned char **carCandParIndice = caracteresCandidatsParIndice((unsigned char *) msgCode, len_key);

    TreeNaire *tree = NTreeCreate();

    int tailleCaraCand;

    for (int i = 0 ; i < len_key ; ++i) {
        printf("clefsCandidatesFinales l.194: cara retenus : %s\n", carCandParIndice[i]);
        tailleCaraCand = nbCara(carCandParIndice[i]);
        NTreeAdd(tree, carCandParIndice[i], tailleCaraCand);
    }

    int nbClefsFinales = nombreClefsCandidates(tree);
    *nbClefs = nbClefsFinales;

    unsigned char **clefsCandidates = malloc(nbClefsFinales * sizeof(unsigned char *));
    if (!clefsCandidates) {
        perror("Erreur alloc mem tab clefs candidates");
        exit(1);
    }

    getClefsCandidates(tree, clefsCandidates);

    freeDoubleArray(&carCandParIndice, len_key);

    NTreeDelete(tree);
    
    return clefsCandidates;
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
    //strcpy((char *) dest, (const char *) from);
    
    int ind = 0;

    while (from[ind] != '\0') {
        dest[ind] = from[ind];
        ++ind;
    }
    dest[ind] = '\0';
}

void freeDoubleArray(unsigned char ***clefs, int len_key) {
    if (!(clefs && *clefs)) {
        return;
    }
    for (int i = 0 ; i < len_key ; ++i) {
        free((*clefs)[i]);
    }
    free(*clefs);
    *clefs = NULL;
}

