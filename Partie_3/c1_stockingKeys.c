#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <math.h>
#include "break_code_c1.h"
#include "../utilitaire/utiL.h"
#include "c1_stockingKeys.h"

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
*/
