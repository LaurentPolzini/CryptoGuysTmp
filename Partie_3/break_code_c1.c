#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <math.h>
#include <unistd.h>
#include <fcntl.h>
#include "./crackage.h"
#include "./Pile.h"
#include "./break_code_c1.h"
#include "Tree.h"
#include "../utilitaire/utiL.h"
#include "caracteresCandidatsIndexKey.h"
#include "ThreadSegmentationTableauxIndex.h"

void appelClefsFinales(char *file_in, int keyLength, char *file_out);
void appelCaracteresCandidats(char *file_in, int keyLength);

pthread_mutex_t *mutexEcritureClefsFichier;
pthread_mutex_t *mutexLectureClefsFichier;

void cpyChaine(unsigned char *dest, unsigned char *from);

void ajouteCandidats(unsigned char ***clefs, unsigned long *nbClefs, int *tailleClefs, unsigned char *carCandidats, int nbCarCand);
void *ajouteCandidatsThreads(void *threadIntel);
void *ajouteCandidatsThreads2(void *threadIntel);

void ajouteCandidats2(char *nameFileIn, unsigned long *nbClefs, int *tailleClefs, unsigned char *carCandidats, int nbCarCand);

// pour free plus vite les clefs
void *threadFreeSegment(void *info);

unsigned long nbClefsTotal(unsigned char **carCandParIndice, int len_key);

typedef void(*FunctorC1)(unsigned char *, void *);

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

// Threads sur plusieurs lignes (jusque threadUntilI)
sPileIndCourFin *initialiseTableauxIndiceThreads(int tailleClef, unsigned char **carCand, int *nbThreads, int threadUntilI);

void functorOnKey(unsigned char *key, FunctorC1 f, void *userData);

void afficheClef(unsigned char *key, void *userData);
void clefTrouve(unsigned char *curKey, void *actualKey);
void ecritClef(unsigned char *clef, void *fileOutDescriptor);

unsigned char *getKeyFromTab(int *tableauInd, unsigned char **carCandidats, int len_key);

void afficheTab(int *tab, int len_key);
   
typedef struct {
    unsigned char *carCandidats;
    int nbCarCand;
    unsigned long debutSegment;
    unsigned long finSegment;
    unsigned long tailleclef;
    unsigned char **keys;
} threadInfoKey;

typedef struct {
    unsigned char *carCandidats;
    int nbCarCand;
    unsigned long debutSegment;
    unsigned long finSegment;
    unsigned long tailleclef;
    int fileInDescriptor;
    int fileOutDescriptor;
} threadInfoKeyFILE;

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

    while (tmp != NULL) {
        ti.piles = tmp;
        curKey = clefActuelle(ti.piles, ti.carCandParIndice, ti.len_key);
        functorOnKey(curKey, ti.f, ti.userData);
        free(curKey);

        tmp = prochaineClefSelonPile(ti.piles, ti.len_key);
    }
    freeSPiles(&(ti.piles), 1);

    pthread_exit(NULL);
}

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
char *ouvreEtLitFichier(char *file_in, off_t *sizeMessage) {
    int fdFile_In = open(file_in, O_RDONLY, 0644);
    if (fdFile_In == -1) {
        pError(NULL, "Erreur ouverture fichier d'entrée", 1);
    }
    long sizeBuffer = 512;
    char *msgLu = malloc(sizeBuffer);
    long curSizeBuff = sizeBuffer;

    ssize_t bytesRead = 0;
    ssize_t totalBytesRead = 0;
    while ((bytesRead = read(fdFile_In, msgLu + totalBytesRead, sizeBuffer)) > 0) {
        totalBytesRead += bytesRead;
        if (totalBytesRead + sizeBuffer > curSizeBuff) {
            curSizeBuff *= 2;
            char *temp = realloc(msgLu, curSizeBuff);
            if (!temp) {
                close(fdFile_In);
                pError(NULL, "Erreur allocation tableau temporaire de lecture du fichier", 1);
            }
            msgLu = temp;
        }        
    }
    msgLu[totalBytesRead] = '\0';
    *sizeMessage = (off_t) totalBytesRead;
    close(fdFile_In);

    return msgLu;
}

void clefsByThreads(char *msgCode, off_t tailleMsgCode, int len_key, unsigned long *nbClefs, void *uD);

int break_code_c1(char *file_in, int keyLength, char *file_out) {
    (void) file_out;
    /*
    off_t tailleMsg;
    char *msg = ouvreEtLitFichier(file_in, &tailleMsg);
    */

    //testThreadSeg(msg, tailleMsg, keyLength, (void *) file_out);
    appelCaracteresCandidats(file_in, keyLength);
    //appelClefsFinales(file_in, keyLength, file_out);
    
    return 1;
}

/*
    File out peut etre la clef qu'on cherche aussi
    c'est userData quoi
*/
void appelClefsFinales(char *file_in, int keyLength, char *file_out) {
    off_t tailleMsg;

    char *msg = ouvreEtLitFichier(file_in, &tailleMsg);

    unsigned long nbClefs = 0;

    time_t tpsDepart = time(NULL);
    //clefsFinales(msg, tailleMsg, keyLength, &nbClefs, file_out);
    clefsByThreads(msg, tailleMsg, keyLength, &nbClefs, (void *) file_out);
    time_t tpsFin = time(NULL);

    printf("Temps creation des %lu clefs : %f\n", nbClefs, difftime(tpsFin, tpsDepart));

    FILE *log = fopen("logKeys.txt", "a+");
    fprintf(log, "key : %s ; number of keys : %lu ; time : %f\n", file_out, nbClefs, difftime(tpsFin, tpsDepart));
    fclose(log);

    free((void *) msg);
    return;
}

void appelCaracteresCandidats(char *file_in, int keyLength) {
    off_t sizeMsg;
    char *msgLu = ouvreEtLitFichier(file_in, &sizeMsg);

    unsigned char **carCandParIndice = caracteresCandidatsParIndice(msgLu, sizeMsg, keyLength);

    printf("Il y a %lu clefs; combinées à partir de :\n", nbClefsTotal(carCandParIndice, keyLength));
    for (int i = 0 ; i < keyLength ; ++i) {
        printf("Caracteres possibles clef[%d] : \"%s\"\n", i, carCandParIndice[i]);
    }

    free((void *) msgLu);
    return;
}

void clefsByThreads(char *msgCode, off_t tailleMsgCode, int len_key, unsigned long *nbClefs, void *uD) {
    unsigned char **carCandParIndice = caracteresCandidatsParIndice(msgCode, tailleMsgCode, len_key);
    for (int i = 0 ; i < len_key ; ++i) {
        printf("Caracteres possibles clef[%d] : \"%s\"\n", i, carCandParIndice[i]);
    }
    (void) uD;
    int nbThreadsMax = 8;
    int nbThreadReel = 1;
    sPileIndCourFin **pilesTests = initialisePilesIndiceThreads(len_key, carCandParIndice, &nbThreadsMax, &nbThreadReel);
    *nbClefs = nbClefsTotal(carCandParIndice, len_key);
    printf("On a %d threads pour %lu clefs\n", nbThreadReel, *nbClefs);

    FunctorC1 f = clefTrouve;
    
    threadsInfosVolees tinfos[nbThreadReel];
    pthread_t thid[nbThreadReel];

    for (int i = 0 ; i < nbThreadReel ; ++i) {
        tinfos[i] = creeInfoThreadPILES(pilesTests[i], carCandParIndice, f, len_key, uD);
        pthread_create(&(thid[i]), NULL, clefsThreadPiles, &(tinfos[i]));
    }
    for (int i = 0 ; i < nbThreadReel ; ++i) {
        pthread_join(thid[i], NULL);
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
        pError(NULL, "Erreur ouverture fichier", 1);
    }
    */

    FunctorC1 f = clefTrouve;
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
void clefsFinales2(char *msgCode, int len_key, unsigned long *nbClefs, char *nameFileOut) {
    unsigned char **carCandParIndice = caracteresCandidatsParIndice((unsigned char *) msgCode, len_key);

    // ecriture clefs par fichier
    mutexEcritureClefsFichier = malloc(sizeof(pthread_mutex_t));
    pError(mutexEcritureClefsFichier, "Erreur création mutex ecriture clefs", 1);
    mutexLectureClefsFichier = malloc(sizeof(pthread_mutex_t));
    pError(mutexLectureClefsFichier, "Erreur création mutex lecture clefs", 1);

    int tailleClefTMP = 0;
    for (int i = 0 ; i < len_key ; ++i) {
        ajouteCandidats2(nameFileOut, &nbClefs, &tailleClefTMP, carCandParIndice[i], (int) strlen((const char *) carCandParIndice[i]));
    }
}
*/
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


unsigned char **clefFinalesParCombinaisons(char *msgCode, unsigned long tailleMsgCode, int len_key, unsigned long *nbClefs) {
    unsigned char **carCandParIndice = caracteresCandidatsParIndice((unsigned char *) msgCode, tailleMsgCode, len_key);
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

    *nbClefs = nbClefsTMP;
    return clefs;
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
/*
threadInfoKey createThreadInfo(unsigned char *carCandidats, int nbCarCand, unsigned long debutSegment,
    unsigned long finSegment, unsigned long tailleclef, int fileInDescriptor, int fileOutDescriptor) {
        threadInfoKey ti;
        ti.carCandidats = carCandidats;
        ti.nbCarCand = nbCarCand;
        ti.debutSegment = debutSegment;
        ti.finSegment = finSegment;
        ti.tailleclef = tailleclef;
        ti.fileInDescriptor = fileInDescriptor;
        ti.fileOutDescriptor = fileOutDescriptor;

        return ti;
}
*/
/*
    Dans clefs les clefs pour l'instant 
    soit les caracteres candidats 
    [123]
    [45]

    on ajoute [123] ce qui fait les clefs ["1", "2", "3"]
    puis on ajoute [45] ce qui donne ["14", "15", "24", "25", "34", "35"]
    
    on ajoute a chaque clefs actuels les nouveaux caracteres candidats
        -> double boucle

    a la fin de tous les appels de carCandidats on a dans ***clefs toutes les clefs
*/
/*
void ajouteCandidats(unsigned char ***clefs, unsigned long *nbClefs, int *tailleClefs, unsigned char *carCandidats, int nbCarCand) {
    // initialise les nouvelles infos : 
    // nouveaux nb de clefs et nouveau tableau de clefs en consequence
    unsigned long newNbClefs = (*nbClefs) * nbCarCand;
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

        ti[i] = createThreadInfo(carCandidats, nbCarCand, *clefs, i * tailleSegment, fin, newKeys, *tailleClefs, -1, -1);
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

// par file (meme chose que ajouteCandidats mais avec un fichier sinon RAM explose)
void ajouteCandidats2(char *nameFileIn, unsigned long *nbClefs, int *tailleClefs, unsigned char *carCandidats, int nbCarCand) {
    int fileInFileDescriptor = open(nameFileIn, O_RDONLY);
    if (fileInFileDescriptor == -1) {
        pError(NULL, "Erreur ouverture fichier d'écriture des clefs candidates", 1);
    }

    int fileOutFileDescriptor = open("tmp.txt", O_WRONLY);
    if (fileOutFileDescriptor == -1) {
        pError(NULL, "Erreur création fichier temporaire d'écriture des clefs", 1);
    }

    // nb de threads a creer
    unsigned long nbSegment = (unsigned long) sqrt(*nbClefs);
    // la portion dont le thread va s'occuper
    unsigned long tailleSegment = *nbClefs / nbSegment + (*nbClefs % nbSegment != 0 ? 1 : 0);
    
    // initialise des tableaux pour les threads
    pthread_t *thid = malloc(nbSegment * sizeof(pthread_t));
    pError(thid, "Erreur creation tableau thread clefs", 1);
    threadInfoKey *ti = malloc(nbSegment * sizeof(threadInfoKey));
    pError(ti, "Erreur creation tableau informations pour threads clefs", 1);

    unsigned long fin;
    
    for (unsigned long i = 0 ; i < nbSegment ; ++i) {
        // attrape bien les dernieres informations
        // si la division n'est pas entiere
        fin = (i + 1) * tailleSegment > *nbClefs ? *nbClefs : (i + 1) * tailleSegment;

        ti[i] = createThreadInfo(carCandidats, nbCarCand, i * tailleSegment, fin,
             *tailleClefs, fileInFileDescriptor, fileOutFileDescriptor);
        pthread_create(&thid[i], NULL, ajouteCandidatsThreads2, (void *) &(ti[i]));
    }
    for (unsigned long i = 0 ; i < nbSegment ; ++i) {
        pthread_join(thid[i], NULL);
    }
    
    *nbClefs = (*nbClefs) * nbCarCand;
    *tailleClefs += 1;

    free(thid);
    free(ti);

    // DELETE fileIn
    // et renommer fileOut en fileIn
    // plus rapide que copier tout fileOut
    close(fileOut);
    close(fileTMP);

    return;
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
/*
void *ajouteCandidatsThreads(void *threadIntel) {
    threadInfoKey ti = *((threadInfoKey *) threadIntel);

    unsigned long indTmp;

    for (unsigned long i = ti.debutSegment ; i < ti.finSegment ; ++i) {
        for (int j = 0 ; j < ti.nbCarCand ; ++j) {
            indTmp = (i * ti.nbCarCand) + j;
            ti.keys[indTmp] = malloc(ti.tailleclef + 2); // 2 : nouveau caractere et \0
            pError(ti.keys[indTmp], "Erreur creation nouvelle clef", 1);

            // copie le 1 dans les 2 premieres cases
            strcpy((char *) ti.keys[indTmp], (const char *) (ti.clef)[i]);
            // met le 4 et le 5 dans les cases 1 et 2 (fin 0 et 1 quoi)
            ti.keys[indTmp][ti.tailleclef] = ti.carCandidats[j];
            ti.keys[indTmp][ti.tailleclef + 1] = '\0';
        }
    }

    pthread_exit(NULL);
}

void *ajouteCandidatsThreads2(void *threadIntel) {
    threadInfoKey ti = *((threadInfoKey *) threadIntel);

    unsigned long indTmp;

    int tailleBuffFile = 1024;
    unsigned char keys[tailleBuffFile * (ti.tailleclef + 1)];
    // lit tout d'un coup, boucle sur le buffer lu
    // toutes les (tailleClefs + 1) j'ajoute le nouveau caractere
    // ecrit tout le buffer dans le nouveau fichier
    // ajoute le caractere " " a la fin de chaque clef

    if (lseek(ti.fileInDescriptor, (off_t) ti.debutSegment, SEEK_SET) == -1) {
        pError(NULL, "Erreur deplacement curseur dans fichier", 1);
    }
    ssize_t bytesRead;
    // nombre d'octet à lire par thread
    ssize_t nbBytesMAXToRead = (ti.finSegment - ti.debutSegment) * ti.tailleclef;
    ssize_t nbBytesReadTotal = 0;
    while (nbBytesReadTotal < nbBytesMAXToRead) {
        if ((bytesRead = read(ti.fileInDescriptor, keys, tailleBuffFile)) < 0) {
            pError(NULL, "Erreur lecture fichier", 1);
        }
        nbBytesReadTotal += bytesRead;
        // lseek sur ti.debut + bytesRead (a cause des autres threads)
    }

    for (unsigned long i = ti.debutSegment ; i < ti.finSegment ; ++i) {
        for (int j = 0 ; j < ti.nbCarCand ; ++j) {
            indTmp = ((i * ti.nbCarCand) + j) % tailleBuffFile;

            // copie le 1 dans les 2 premieres cases
            strcpy((char *) newKeys[indTmp], (const char *) (ti.clef)[i]);
            // met le 4 et le 5 dans les cases 1 et 2 (fin 0 et 1 quoi)
            newKeys[indTmp][ti.tailleclef] = ti.carCandidats[j];
            newKeys[indTmp][ti.tailleclef + 1] = '\0';

            if (indTmp == tailleBuffFile - 1) {
                ecritClef(ti.fileOut, newKeys, tailleBuffFile);
            }
        }
    }
    
    pthread_exit(NULL);
}

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
            newKeys[indTMP]
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

void afficheTab(int *tab, int len_key) {
    pthread_mutex_lock(mutexEcritureClefsFichier);
    for (int i = 0 ; i < len_key ; ++i) {
        printf("%d ", tab[i]);
    }
    printf("\n");
    pthread_mutex_unlock(mutexEcritureClefsFichier);
}

