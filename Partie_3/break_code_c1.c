#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <math.h>
#include <sys/resource.h>
#include "./crackage.h"
#include "mutex.h"

#include "./Pile.h" // pour les indices de debut et fin de segments
#include "caracteresCandidatsIndexKey.h" // trouve les caracteres candidats
#include "ThreadSegmentationTableauxIndex.h" // découpage des segments de caracteres candidats
#include "../Partie_1/chiffrement.h" // encrypt_decrypt_xor

#include "./break_code_c1.h"
#include "./break_code_c2_c3.h" // copier les structures pour les threads

#include "../utilitaire/utiL.h" // tout ce qui est pError...

#include "autres_methodes/c1_stockingKeys.h"

#include "tests_crackage.h"

double keysPerSec = 1e9 / 1200; // aux dernieres nouvelles le programme fait 1 milliard de clef en 20 minutes
// la traduction de l'entiereté du message par chaque clef est tres long
// il faudrait changer dans le message uniquement les caracteres modifiés
// par la nouvelle clef (généralement a un caractere de difference avec la precedente)
// la recherche de mot dans le dictionnaire doit aussi etre bien long

// besoin d'un mutex si on doit ecrire une clef dans un fichier
pthread_mutex_t *mutexEcritureClefsFichier;


void clefsByThreads(char *msgCode, off_t tailleMsgCode, int len_key, unsigned long *nbClefs, void *uD, FunctorC1 functor, bool c2c3);
void *clefsThreadPiles(void *arg);

/*
    creation / destruction de copies de stC2_C3
    => chaque threads a sa structure
*/
void destroysInitedStC2C3(stC2_C3 ***array, int nbThreads);
stC2_C3 **init_stC2C3_forThread(stC2_C3 *original, int nbThreads);

// compile les meilleurs scores de array dans toWhere
void associeMaxTabs(struct stC2_C3 **array, struct stC2_C3 *toWhere, int nbThreads);

// evite de cast a chaque fois (strcpy)
void cpyChaine(unsigned char *dest, unsigned char *from);

// estime le temps de génération des clefs
// en prenant comme temps 1 milliard de clef en 20 minutes
// avec c3
char *estimation_temps(unsigned long nbClefs);


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

/*
    genere juste les clefs sans rien faire dessus
*/
int break_code_c1(char *file_in, int keyLength, char *logFile) {

    appelClefsFinales(file_in, keyLength, NULL, doNothing, logFile, false, NULL);

    // génération des clefs par un tableau de stockage
    // montre vite les limites : ram explose
    //appelClefsParCombo(file_in, keyLength, logFile);

    return 0;
}

/*
    creation des informations pour les threads
    sPileIndCourFin = piles indices debut / fin / courants
    carCandParIndice les caracteres permettant de generer les clefs
    f un functor sur la clef générée et userData
    userData : le 2eme parametre du functor
    len_key la taille des clefs (le nombre de lignes de carCandParIndice)
*/
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


/*
    appelle la fonction caracteres candidats
    (les caracteres possibles pour former les clefs)
*/
void appelCaracteresCandidats(char *file_in, int keyLength) {
    off_t sizeMsg;
    char *msgLu = ouvreEtLitFichier(file_in, &sizeMsg);

    unsigned char **carCandParIndice = caracteresCandidatsParIndice(msgLu, sizeMsg, keyLength);

    for (int i = 0 ; i < keyLength ; ++i) {
        printf("Caracteres possibles clef[%d] : \"%s\"\n", i, carCandParIndice[i]);
    }

    free((void *) msgLu);
	freeTabs((void ***) &carCandParIndice, keyLength);
    return;
}

/*
    genere les clefs en faisant un functor sur les clefs
    cree des threads
*/
void appelClefsFinales(char *file_in, int keyLength, void *userData, FunctorC1 functor, char *logFile, bool c2c3, unsigned long *nbOfKeys) {
    off_t tailleMsg;
    char *msg = ouvreEtLitFichier(file_in, &tailleMsg);

    unsigned long nbKeys = 0;

    printf("Début de C1...\n");

    time_t tpsDepart = time(NULL);
    clefsByThreads(msg, tailleMsg, keyLength, &nbKeys, userData, functor, c2c3);
    time_t tpsFin = time(NULL);

    char *SlenKey = format_number_with_thousands_separator(nbKeys);
    char *StimeDiff = format_seconds_to_string(difftime(tpsFin, tpsDepart));

    size_t tailleAllouee = strlen("text from : ; number of keys : ; time : \n") + strlen(file_in) + strlen(SlenKey) + strlen(StimeDiff) + 1;
    char *textLog = malloc(tailleAllouee);
    pError(textLog, "Erreur allocation memoire text log file", 1);
    snprintf(textLog, tailleAllouee, "text from : %s ; number of keys : %s ; time : %s\n", file_in, SlenKey, StimeDiff);

    if (logFile) {
        FILE *log = fopen(logFile, "a");
        pError(log, "Erreur ouverture fichier log c1", 1);
        fprintf(log, "%s\n", textLog);

        fclose(log);
    } else {
        printf("%s\n", textLog);
    }
    
    free((void *) msg);
    free(textLog);
    free(SlenKey);
    free(StimeDiff);

    printf("Fin de C1\n");

    if (nbOfKeys) {
        *nbOfKeys = nbKeys;
    }
    
    return;
}

/*
    gere les threads
*/
void clefsByThreads(char *msgCode, off_t tailleMsgCode, int len_key, unsigned long *nbClefs, void *uD, FunctorC1 functor, bool c2c3) {
    
    unsigned char **carCandParIndice = caracteresCandidatsParIndice(msgCode, tailleMsgCode, len_key);
    
    affiche_caracteres_candidats(carCandParIndice, len_key);
    
    unsigned long nbClef = nbClefsTotal(carCandParIndice, len_key);

    if (nbClef == 0) {
        freeTabs((void ***) &carCandParIndice, len_key);
        printf("Il n'y a pas de clef : sortie du programme.\n");
        return;
    } else if (nbClefs) {
        *nbClefs = nbClef;
    }

    char *nbKeysToString = format_number_with_thousands_separator(nbClef);

    // nombre de threads selon la machine
    long nbThreadsMax = sysconf(_SC_NPROCESSORS_CONF);

    long nbThreadReel = 1;
    sPileIndCourFin **piles = initialisePilesIndiceThreads(len_key, carCandParIndice, &nbThreadsMax, &nbThreadReel);
    
    
    unsigned long *nbClefTraiteeByThreads[nbThreadReel];
    unsigned long nbTotalClefsTraitees = 0;
    int pourcentage = 0;
    
    threadsInfosVolees tinfos[nbThreadReel];
    pthread_t thid[nbThreadReel];
    stC2_C3 **c2c3_UD = NULL;
    char *tempsRestant = estimation_temps(nbClef);

    if (c2c3) {
        c2c3_UD = init_stC2C3_forThread((stC2_C3 *) uD, nbThreadReel);
    }
    printf("La génération des %s clefs devrait prendre %s\n", nbKeysToString, tempsRestant);
    free(tempsRestant);
    for (int i = 0 ; i < nbThreadReel ; ++i) {
        if (c2c3) {
            tinfos[i] = creeInfoThreadPILES(piles[i], carCandParIndice, functor, len_key, (void *) (c2c3_UD[i]));
        } else {
            tinfos[i] = creeInfoThreadPILES(piles[i], carCandParIndice, functor, len_key, uD);
        }

        pthread_create(&(thid[i]), NULL, clefsThreadPiles, &(tinfos[i]));
    }
    static int previous_length = 0;
    int current_length;

    for (int i = 0 ; i < nbThreadReel ; ++i) {
        pthread_join(thid[i], (void **) &(nbClefTraiteeByThreads[i]));

        nbTotalClefsTraitees += *(nbClefTraiteeByThreads[i]);
        pourcentage = (int) ((nbTotalClefsTraitees / nbClef) * 100);
        if (pourcentage != 100) {
            tempsRestant = estimation_temps(nbClef - nbTotalClefsTraitees);
            
            current_length = snprintf(NULL, 0, "%d%% des %s clefs traitées. Il reste %s", pourcentage, nbKeysToString, tempsRestant);
            
            // d'un coup le \r a arreté de fonctionné
            // je dois passé par cette statégie
            // il faut aussi que le shell soit assez grand 
            // pour afficher la phrase en entiere
            // sinon ca ne marche pas...
            printf("\r%*s\r", previous_length, "");
            printf("%d%% des %s clefs traitées. Il reste %s", pourcentage, nbKeysToString, tempsRestant);

            previous_length = current_length;
            free(tempsRestant);
        } else {
            printf("\r%*s\r", previous_length, "");
            printf("\r%d%% des %s clefs traitées !", pourcentage, nbKeysToString);
        }
        
        fflush(stdout);

        free(nbClefTraiteeByThreads[i]);
    }
	free(nbKeysToString);
    printf("\n");

    if (c2c3) {
        associeMaxTabs(c2c3_UD, (stC2_C3 *) uD, nbThreadReel);
        destroysInitedStC2C3(&c2c3_UD, nbThreadReel);
    }
    
    freeSPiles(&piles, nbThreadReel);
	freeTabs((void ***) &carCandParIndice, len_key);
    
    return;   
}

/*
    fonction pour les threads
    charge une clef et fait un fonctor dessus
    charge les clefs selon le contenue de threadsInfosVolees -> piles
    pile debut -> pile fin : les indices des caracteres candidats (la clef a generer)
*/
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

    pthread_exit((void *) nbClefTraitee);
}



unsigned long nbClefsTotal(unsigned char **carCandParIndice, int len_key) {
    unsigned long nbClefs = 1;
    for (int i = 0 ; i < len_key ; ++i) {
        nbClefs *= strlen((const char *) carCandParIndice[i]);
    }
    return nbClefs;
}


// -------------------------------------------------------------------------------------------
/*
    Functors sur les clefs
*/
void functorOnKey(unsigned char *key, FunctorC1 f, void *userData) {
    f(key, userData);
}

// retourne une clef
// a partir de carCandidats et des indices correspondants dans tableauInd
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
    stocke les clefs créées pour voir leur unicité
*/
void stock_key(unsigned char *key, void *SstockKeys) {
    if (pthread_mutex_lock(&MUTEX_TEST) != 0) {
        pError(NULL, "Erreur prendre jeton tests crackages", 1);
    }

    insert_key((sstock_key *) SstockKeys, key);

    if (pthread_mutex_unlock(&MUTEX_TEST) != 0) {
        pError(NULL, "Erreur don jeton tests crackages", 1);
    }
}

// ------------------------------------------------------------------------------------
/*
    fonctions pour la creation de threads et les informations a passer
*/
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
            destruct_stC2_C3(&((*array)[i]));
        }
        free((void *) (*array));
    }
}

/*
    associates to toWhere the compiled best arrays
    of the nbThreads array
*/
void associeMaxTabs(struct stC2_C3 **array, struct stC2_C3 *toWhere, int nbThreads) {
    for (int i = 0 ; i < nbThreads ; ++i) {
        incrusteTab(array[i], toWhere);
    } 
}

// evite de faire les cast (char *) a chaque fois
void cpyChaine(unsigned char *dest, unsigned char *from) {
    strcpy((char *) dest, (const char *) from);
}



void afficheTab(int *tab, int len_key) {
    for (int i = 0 ; i < len_key ; ++i) {
        printf("%d ", tab[i]);
    }
    printf("\n");
}

// 1 milliard de clef en 20 minutes
char *estimation_temps(unsigned long nbClefs) {
    return format_seconds_to_string(nbClefs / keysPerSec);
}


/*
    l'idée ici c'est que garder les clefs dans un tableau c'est compliqué
    alors plutot qu'un tableau on passe par un fichier
    c'a vite été abandonné : technique de génération de clef pas assez efficace

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
