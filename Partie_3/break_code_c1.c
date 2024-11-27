#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <math.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/resource.h>
#include "./crackage.h"
#include "./Pile.h"
#include "./break_code_c1.h"
#include "Tree.h"
#include "../utilitaire/utiL.h"
#include "caracteresCandidatsIndexKey.h"
#include "ThreadSegmentationTableauxIndex.h"
#include "../Partie_1/xor.h"

void appelCaracteresCandidats(char *file_in, int keyLength);

pthread_mutex_t *mutexEcritureClefsFichier;
pthread_mutex_t *mutexLectureClefsFichier;

void cpyChaine(unsigned char *dest, unsigned char *from);

void ajouteCandidats(unsigned char ***clefs, unsigned long *nbClefs, int *tailleClefs, unsigned char *carCandidats, int nbCarCand);
void *ajouteCandidatsThreads(void *threadIntel);

// pour free plus vite les clefs
void *threadFreeSegment(void *info);

unsigned long nbClefsTotal(unsigned char **carCandParIndice, int len_key);

typedef struct {
    int *tableauIndCour;
    int *tableauIndMax;
} sTableauIndCourantEtMax;


typedef struct {
    sTableauIndCourantEtMax tabs;
    unsigned char **carCandParIndice;
    int len_key;
    void *userData;
    FunctorC1 f;
} threadInfoClefVolee;

// Threads juste sur la premiere ligne
threadInfoClefVolee creeInfoThread(sTableauIndCourantEtMax tabs, 
    unsigned char **carCandParIndice, FunctorC1 f, int len_key, void *userData);
void *clefsThread(void *arg);

sTableauIndCourantEtMax initialiseTableauxIndiceThreadLIGNE0(int tailleClef, unsigned char **carCand, 
    int indThread, int nbThreads, int tailleSegment);

sTableauIndCourantEtMax initialiseTableauxIndice(int tailleClef, unsigned char **carCand);
sTableauIndCourantEtMax prochaineClefSelonTableau(sTableauIndCourantEtMax tabs, int tailleClef);

void clefsByThreads(char *msgCode, off_t tailleMsgCode, int len_key, unsigned long *nbClefs, void *uD, FunctorC1 functor, bool c2c3);

void functorOnKey(unsigned char *key, FunctorC1 f, void *userData);

void afficheClef(unsigned char *key, void *userData);
void clefTrouve(unsigned char *curKey, void *actualKey);
void ecritClef(unsigned char *clef, void *fileOutDescriptor);
void doNothing(unsigned char *none, void *userData);

unsigned char *getKeyFromTab(int *tableauInd, unsigned char **carCandidats, int len_key);

void afficheTab(int *tab, int len_key);
   
stC2_C3 **init_stC2C3_forThread(stC2_C3 *original, int nbThreads);
void destroysInitedStC2C3(stC2_C3 ***array, int nbThreads);
void associeMaxTabs(stC2_C3 **array, stC2_C3 *toWhere, int nbThreads);

char *format_number_with_thousands_separator(unsigned long number);

typedef struct {
    unsigned char ***arr;
    unsigned long debut, fin;
} threadFreeInfo;

typedef struct {
    sPileIndCourFin *piles;
    unsigned char **carCandParIndice;
    FunctorC1 f;
    int len_key;
    void *userData;
} threadsInfosVolees;

threadsInfosVolees creeInfoThreadPILES(sPileIndCourFin *piles, 
    unsigned char **carCandParIndice, FunctorC1 f, int len_key, void *userData) {

    threadsInfosVolees ti;
    ti.piles = piles;
    ti.carCandParIndice = carCandParIndice;
    ti.f = f;
    ti.len_key = len_key;
    ti.userData = userData;
    
    return ti;
}

void *clefsThreadPiles(void *arg) {
    threadsInfosVolees ti = *((threadsInfosVolees *) arg);
    unsigned char *curKey;
    sPileIndCourFin *tmp = ti.piles;

    unsigned long *nbClefTraitee = malloc(sizeof(unsigned long));
    pError(nbClefTraitee, "Erreur allocation nbClefTraitee", 1);
    *nbClefTraitee = 0;

    while (tmp != NULL) {
        ti.piles = tmp;
        curKey = clefActuelle(ti.piles, ti.carCandParIndice, ti.len_key);
        functorOnKey(curKey, ti.f, ti.userData);
        free(curKey);

        tmp = prochaineClefSelonPile(ti.piles, ti.len_key);
        *nbClefTraitee += 1;
    }
    freeSPiles(&(ti.piles), 1);

    pthread_exit((void *) nbClefTraitee);
}


int break_code_c1(char *file_in, int keyLength, char *logFile) {

    appelClefsFinales(file_in, keyLength, NULL, doNothing, logFile, false);
    
    return 0;
}

/*
    File out peut etre la clef qu'on cherche aussi
    c'est userData quoi
*/
void appelClefsFinales(char *file_in, int keyLength, void *userData, FunctorC1 functor, char *logFile, bool c2c3) {
    off_t tailleMsg;

    char *msg = ouvreEtLitFichier(file_in, &tailleMsg);

    unsigned long nbClefs = 0;

    time_t tpsDepart = time(NULL);
    //clefsFinales(msg, tailleMsg, keyLength, &nbClefs, userData);
    clefsByThreads(msg, tailleMsg, keyLength, &nbClefs, userData, functor, c2c3);
    time_t tpsFin = time(NULL);

    printf("Temps génération des %lu clefs : %f\n", nbClefs, difftime(tpsFin, tpsDepart));

    char *textLog = malloc(strlen(file_in) + strlen("text : ; number of keys : ; time : \n") + sizeof(unsigned long) + sizeof(double) + 1);
    pError(textLog, "Erreur allocation memoire text log file", 1);
    snprintf(textLog, sizeof(textLog), "text : %s ; number of keys : %lu ; time : %f\n", file_in, nbClefs, difftime(tpsFin, tpsDepart));

    if (logFile) {
        FILE *log = fopen(logFile, "a+");
        pError(log, "Erreur ouverture fichier log", 1);
        fprintf(log, textLog, strlen(textLog));

        fclose(log);
    } else {
        printf("%s", textLog);
    }
    
    free((void *) msg);
    free(textLog);
    return;
}

void appelCaracteresCandidats(char *file_in, int keyLength) {
    off_t sizeMsg;
    char *msgLu = ouvreEtLitFichier(file_in, &sizeMsg);

    unsigned char **carCandParIndice = caracteresCandidatsParIndice(msgLu, sizeMsg, keyLength);

    for (int i = 0 ; i < keyLength ; ++i) {
        printf("Caracteres possibles clef[%d] : \"%s\"\n", i, carCandParIndice[i]);
    }

    free((void *) msgLu);
    return;
}

void clefsByThreads(char *msgCode, off_t tailleMsgCode, int len_key, unsigned long *nbClefs, void *uD, FunctorC1 functor, bool c2c3) {
    unsigned char **carCandParIndice = caracteresCandidatsParIndice(msgCode, tailleMsgCode, len_key);
    *nbClefs = nbClefsTotal(carCandParIndice, len_key);

    char *nbKeysToString = format_number_with_thousands_separator(*nbClefs);
    printf("Il y a %s clefs combinées à partir de :\n", nbKeysToString);
    for (int i = 0 ; i < len_key ; ++i) {
        printf("Caracteres possibles clef[%d] : \"%s\"\n", i, carCandParIndice[i]);
    }

    // nombre de threads selon la machine
    long nbThreadsMax = sysconf(_SC_NPROCESSORS_CONF);
    //long nbThreadsMax = 100;

    long nbThreadReel = 1;
    sPileIndCourFin **pilesTests = initialisePilesIndiceThreads(len_key, carCandParIndice, &nbThreadsMax, &nbThreadReel);

    unsigned long *nbClefTraiteeByThreads[nbThreadReel];
    unsigned long nbTotalClefsTraitees = 0;
    float pourcentage = 0;
    
    threadsInfosVolees tinfos[nbThreadReel];
    pthread_t thid[nbThreadReel];
    stC2_C3 **c2c3_UD = NULL;
    if (c2c3) {
        c2c3_UD = init_stC2C3_forThread((stC2_C3 *) uD, nbThreadReel);
    }
    
    for (int i = 0 ; i < nbThreadReel ; ++i) {
        if (c2c3) {
            tinfos[i] = creeInfoThreadPILES(pilesTests[i], carCandParIndice, functor, len_key, (void *) (c2c3_UD[i]));
        } else {
            tinfos[i] = creeInfoThreadPILES(pilesTests[i], carCandParIndice, functor, len_key, uD);
        }
        pthread_create(&(thid[i]), NULL, clefsThreadPiles, &(tinfos[i]));
    }
    for (int i = 0 ; i < nbThreadReel ; ++i) {
        pthread_join(thid[i], (void *) &(nbClefTraiteeByThreads[i]));

        nbTotalClefsTraitees += *(nbClefTraiteeByThreads[i]);
        pourcentage = ((float) nbTotalClefsTraitees / (float) *nbClefs) * 100;
        if (pourcentage != 100) {
            printf("\r%.2f%% des %lu clefs traitées...", pourcentage, *nbClefs);
        } else {
            printf("\r%.2f%% des %lu clefs traitées !", pourcentage, *nbClefs);
        }
        
        fflush(stdout);

        free(nbClefTraiteeByThreads[i]);
    }
    printf("\n");

    if (c2c3) {
        associeMaxTabs(c2c3_UD, (stC2_C3 *) uD, nbThreadReel);
        destroysInitedStC2C3(&c2c3_UD, nbThreadReel);
    }
    
    free(pilesTests); // dedans tout est déjà free
    // des que ce n'est plus utile (a la fin du traitement)
    // ca free le tableau
    return;   
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
*/
void clefsFinales(char *msgCode, off_t tailleMsgCode, int len_key, unsigned long *nbClefs, char *nameFileOut) {
    mutexEcritureClefsFichier = malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init(mutexEcritureClefsFichier, NULL);

    unsigned char **carCandParIndice = caracteresCandidatsParIndice(msgCode, tailleMsgCode, len_key);
    *nbClefs = nbClefsTotal(carCandParIndice, len_key);
    printf("nb clefs : %lu\n", *nbClefs);

    //int tailleSegment = 1;
    //int tailleSegment = ceil((double) strlen((const char *) carCandParIndice[0]) / 8);
    int tailleSegment = ceil((double) sqrt(strlen((const char *) carCandParIndice[0])));
    int nbSegment = strlen((const char *) carCandParIndice[0]) / tailleSegment;
    /*
    int fileOutDescriptor = open(nameFileOut, O_WRONLY | O_CREAT, 0644);
    if (fileOutDescriptor == -1) {
        printf("%s : ", nameFileOut);
        fflush(stdout);
        pError(NULL, "Erreur ouverture fichier", 1);
    }
    */

    FunctorC1 f = doNothing;
    void *userData = (void *) &nameFileOut;
    sTableauIndCourantEtMax tabs[nbSegment];
    pthread_t thid[nbSegment];
    threadInfoClefVolee infoThreads[nbSegment];
    for (int i = 0 ; i < nbSegment ; ++i) {
        tabs[i] = initialiseTableauxIndiceThreadLIGNE0(len_key, carCandParIndice, i, nbSegment, tailleSegment);
        infoThreads[i] = creeInfoThread(tabs[i], carCandParIndice, f, len_key, userData);
        pthread_create(&thid[i], NULL, clefsThread, &(infoThreads[i]));
    }
    for (int i = 0 ; i < nbSegment ; ++i) {
        pthread_join(thid[i], NULL);
    }

    pthread_mutex_destroy(mutexEcritureClefsFichier);
    return;
}

/*
unsigned char **clefFinalesTree(char *msgCode, unsigned long tailleMsgCode, int len_key, unsigned long *nbClefs) {
    unsigned char **carCandParIndice = caracteresCandidatsParIndice((unsigned char *) msgCode, tailleMsgCode, len_key);

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

void noThreadClefFinaleIndTAB(char *msgCode, unsigned long tailleMsgCode, int len_key, unsigned long *nbClefs, char *nameFileOut) {
    unsigned char **carCandParIndice = caracteresCandidatsParIndice((unsigned char *) msgCode, tailleMsgCode, len_key);
    sTableauIndCourantEtMax tabs = initialiseTableauxIndice(len_key, carCandParIndice);

    unsigned char *curKey;
    FunctorC1 f = clefTrouve;
    void *userData = (void *)nameFileOut;

    while (tabs.tableauIndCour != NULL) {
        curKey = getKeyFromTab(tabs.tableauIndCour, carCandParIndice, len_key);
        functorOnKey(curKey, f, userData);
        free(curKey);

        tabs = prochaineClefSelonTableau(tabs, len_key);
    }
    
    *nbClefs = nbClefsTotal(carCandParIndice, len_key);
    return;
}

*/


unsigned long nbClefsTotal(unsigned char **carCandParIndice, int len_key) {
    unsigned long nbClefs = 1;
    for (int i = 0 ; i < len_key ; ++i) {
        nbClefs *= strlen((const char *) carCandParIndice[i]);
    }
    return nbClefs;
}

void cpyChaine(unsigned char *dest, unsigned char *from) {
    strcpy((char *) dest, (const char *) from);
}

void freeDoubleArray(unsigned char ***arr, unsigned long len) {
    if (!(arr && *arr)) {
        return;
    }
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

// mutex sur l'écriture
void ecritClef(unsigned char *clef, void *fileOutDescriptor) {
    int fd = *((int *) fileOutDescriptor);
    if (pthread_mutex_lock(mutexEcritureClefsFichier) != 0) {
        pError(NULL, "Erreur prendre jeton pour ecriture clef fichier", 1);
    }
    if (write(fd, strcat((char *) clef, "\n"), strlen((const char *) clef)) < 0) {
        pError(NULL, "Erreur écriture nouvelles clef fichier", 1);
    }
    if (pthread_mutex_unlock(mutexEcritureClefsFichier) != 0) {
        pError(NULL, "Erreur don jeton pour ecriture clef fichier", 1);
    }
}


/*
// clefs le buffer de lecture du fichier de clef fileIn
// il ne faut pas que nbClefs (la taille du buffer) soit trop grand
// sinon explosion de la RAM
void traitementClefsLues(unsigned char *clefs, int nbClefs, int tailleClef, unsigned char **carCandidats, int nbCarCand, int fileOutFD) {
    unsigned char *newKeys = malloc(nbClefs * nbCarCand * (tailleClef + 2));
    int indTMP;

    for (int i = 0 ; i < nbClefs ; ++i) {
        for (int j = 0 ; j < nbCarCand ; ++j) {
            indTMP = ((i * nbCarCand) + j) * (tailleClef + 2);
            strncpy(&(newKeys[indTMP]), &(clefs[i * tailleClef]), tailleClef);
            newKeys[indTMP];
        }
    }
    ecritClef(fileOutFD, newKeys, sizeof(newKeys));
    free(newKeys);
}
*/

/*
    au depart faut calculer le nb de clefs

    une fonction principale qui appelle f1 tant que 
    le retour de f2 est différent de NULL
    (if break (sinon augmente la premiere clef sans l'utiliser))
    
    elle incrémente la clef grace a f2

    c'est une fonction f1 qui applique une fonction f sur la clef courante 
    selon un tableau de int

    y'a une autre fonction f2 qui retourne le tableau suivant selon un tableau

    c'est un tableau de int representant l'indice de la derniere clef lu
    quand l'indice d'un tableau arrive a la taille du nb de car candidats
    ca augmente celui du dessus de 1, et on repart de 0 sur la ligne courante
    si c'est le premier qui est augmenté


    quant aux threads, il faut les faire sur une portion de chaque tableau

*/

/*
    [1,2]
    [3]
    [4,5]

    [ 
      0
      0
      0
    ]
    renvoit 
    [ 
      0
      0
      1
    ]
    renvoit
    [ 
      1
      0 (pas 1 puisque length([3]) == 1)
      0
    ]
    renvoit
    [ 
      1
      0
      1
    ]
    renvoit NULL
*/
sTableauIndCourantEtMax prochaineClefSelonTableau(sTableauIndCourantEtMax tabs, int tailleClef) {
    int ind = tailleClef - 1;
    int *tableau = tabs.tableauIndCour;
    int *nbCaractereCandidatParIndice = tabs.tableauIndMax;

    if (tableau[ind] < nbCaractereCandidatParIndice[ind]) {
        tableau[ind] += 1;
    }
    while ((tableau[ind] == nbCaractereCandidatParIndice[ind]) && (ind > 0)) {
        tableau[ind - 1] += 1;
        tableau[ind] = 0;
        --ind;
    }
    
    if (ind == 0 && tableau[0] == nbCaractereCandidatParIndice[0]) {
        tabs.tableauIndCour = NULL;
    }
    
    return tabs;
}

/*
    initialise le tableau d'indice courant et le tableau d'indice a ne pas depasser

    pour faire avec des threads il faudrait implanter une taille de segment
*/
sTableauIndCourantEtMax initialiseTableauxIndice(int tailleClef, unsigned char **carCand) {
    sTableauIndCourantEtMax tabs;

    tabs.tableauIndCour = malloc(tailleClef * sizeof(int));
    pError(tabs.tableauIndCour, "Erreur creation tableau d'indice", 1);
    tabs.tableauIndMax = malloc(tailleClef * sizeof(int));
    pError(tabs.tableauIndMax, "Erreur creation tableau d'indice max", 1);

    for (int i = 0 ; i < tailleClef ; ++i) {
        tabs.tableauIndCour[i] = 0;
        tabs.tableauIndMax[i] = strlen((const char *) carCand[i]);
    }

    return tabs;
}

/*
    chaque thread s'occupera d'un segment de la premiere ligne de carCandidats
    exemple:
    soit le tableau de caracteres candidats
    [1234,
    56,
    908]
    et la taille d'un segment sqrt()
    thread 1 s'occupe de 12 de la premiere ligne et du reste
    thread 2 s'occupe de 34 et du reste (de toutes les lignes en bas)

    la division des taches se fait selon la premiere ligne
    probleme : premiere ligne petite mais reste hyper grand bah y'aura pas beaucoup
    de threads donc ca prendra une eternité
*/
sTableauIndCourantEtMax initialiseTableauxIndiceThreadLIGNE0(int tailleClef, unsigned char **carCand, int indThread, int nbThreads, int tailleSegment) {
    sTableauIndCourantEtMax tabs;

    tabs.tableauIndCour = malloc(tailleClef * sizeof(int));
    pError(tabs.tableauIndCour, "Erreur creation tableau d'indice", 1);
    tabs.tableauIndMax = malloc(tailleClef * sizeof(int));
    pError(tabs.tableauIndMax, "Erreur creation tableau d'indice max", 1);

    for (int i = 1 ; i < tailleClef ; ++i) {
        tabs.tableauIndCour[i] = 0;
        tabs.tableauIndMax[i] = strlen((const char *) carCand[i]);
    }
    tabs.tableauIndCour[0] = indThread * tailleSegment;
    if (indThread == nbThreads - 1) {
        tabs.tableauIndMax[0] = strlen((const char *) carCand[0]);
    } else {
        tabs.tableauIndMax[0] = (indThread + 1) * tailleSegment;
    }
    
    return tabs;
}

threadInfoClefVolee creeInfoThread(sTableauIndCourantEtMax tabs, 
    unsigned char **carCandParIndice, FunctorC1 f, int len_key, void *userData) {
    threadInfoClefVolee ti;
    ti.tabs = tabs;
    ti.carCandParIndice = carCandParIndice;
    ti.f = f;
    ti.len_key = len_key;
    ti.userData = userData;

    return ti;
}

void *clefsThread(void *arg) {
    threadInfoClefVolee ti = *((threadInfoClefVolee *) arg);
    unsigned char *curKey;

    while ((ti.tabs).tableauIndCour != NULL) {
        curKey = getKeyFromTab((ti.tabs).tableauIndCour, ti.carCandParIndice, ti.len_key);
        functorOnKey(curKey, ti.f, ti.userData);
        free(curKey);

        ti.tabs = prochaineClefSelonTableau(ti.tabs, ti.len_key);
    }

    pthread_exit(NULL);
}

void functorOnKey(unsigned char *key, FunctorC1 f, void *userData) {
    f(key, userData);
}

unsigned char *getKeyFromTab(int *tableauInd, unsigned char **carCandidats, int len_key) {
    unsigned char *key = malloc(len_key + 1);
    for (int i = 0 ; i < len_key ; ++i) {
        key[i] = carCandidats[i][tableauInd[i]];
    }
    key[len_key] = '\0';

    return key;
}

void afficheClef(unsigned char *key, void *userData) {
    (void) userData;
    printf("%s\n", key);
}

void clefTrouve(unsigned char *curKey, void *actualKey) {
    const char *keyChar = (const char *) actualKey;
    if (strcmp((const char *) curKey, keyChar) == 0) {
        printf("Trouvé ! %s\n", curKey);
    }
}

void doNothing(unsigned char *none, void *userData) {
    (void) none;
    (void) userData;
}

// met dans msgUncrypted le message traduit
void translateMsg(unsigned char *key, void *msgAndTaille) {
    sMsgAndTaille *smt = (sMsgAndTaille *) msgAndTaille;
    smt -> msgUncrypted = encrypt_decrypt_xorMSG(smt -> msg, (char *) key, smt -> lenMsg);
}

void afficheTab(int *tab, int len_key) {
    for (int i = 0 ; i < len_key ; ++i) {
        printf("%d ", tab[i]);
    }
    printf("\n");
}

/*
    copies the original structure (the userData used in appelClefsFinales)
    into a (nbThreads - 1) length array.
*/
stC2_C3 **init_stC2C3_forThread(stC2_C3 *original, int nbThreads) {
    stC2_C3 **stTh = malloc(sizeof(stC2_C3 *) * nbThreads);
    pError(stTh, "Erreur allocation memoire", 1);
    for (int i = 0 ; i < nbThreads ; ++i) {
        stTh[i] = copySC2C3(original);
    }

    return stTh;
}

/*
    frees every cell of the array and the array itself
    except the first cell which contains the initial userData
    it is for the user to free.
*/
void destroysInitedStC2C3(stC2_C3 ***array, int nbThreads) {
    if (**array) {
        for (int i = 0 ; i < nbThreads ; ++i) {
            destroyStructC2_C3(&((*array)[i]));
        }
        free((void *) (*array));
    }
}

/*
    associates to toWhere the compiled best arrays
    of the nbThreads array
*/
void associeMaxTabs(stC2_C3 **array, stC2_C3 *toWhere, int nbThreads) {
    for (int i = 0 ; i < nbThreads ; ++i) {
        incrusteTab(array, toWhere, i);
    } 
}
/*
    transform 1000 to 1'000
    for readability for the number of keys
*/
char *format_number_with_thousands_separator(unsigned long number) {
    int nbElem = 20;
    char *result = malloc(nbElem);
    pError(result, "Erreur allocation memoire", 1);
    int i = 0, j = 0;
    int len = 0;
    char temp[nbElem];

    // Convertir le nombre en chaîne de caractères
    snprintf(temp, sizeof(temp), "%lu", number);

    len = strlen(temp);

    // Ajouter les séparateurs de milliers
    for (i = len - 1; i >= 0; i--) {
        result[j++] = temp[i];
        // Insérer un séparateur tous les 3 chiffres
        if ((len - i) % 3 == 0 && i != 0) {
            result[j++] = '\'';
        }
    }

    result[j] = '\0';  // Terminer la chaîne
    // Inverser la chaîne pour obtenir l'ordre correct
    for (i = 0; i < j / 2; i++) {
        char temp_char = result[i];
        result[i] = result[j - i - 1];
        result[j - i - 1] = temp_char;
    }

    return result;
}
