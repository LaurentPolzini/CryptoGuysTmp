#ifndef __C1_STOCKINGKEYS_H__
#define __C1_STOCKINGKEYS_H__

typedef struct {
    unsigned char *carCandidats;
    int nbCarCand;
    unsigned long debutSegment;
    unsigned long finSegment;
    unsigned long tailleclef;
    unsigned char **actual_keys;
    unsigned char **next_keys;

    int fileInDescriptor;
    int fileOutDescriptor;
} threadInfoKey;

void appelClefsParCombo(char *file_in, int keyLength, char *logFile);

void *ajouteCandidatsThreads(void *threadIntel);
void ajouteCandidats(unsigned char ***clefs, unsigned long *nbClefs, int *tailleClefs, unsigned char *carCandidats, int nbCarCand);
unsigned char **clefFinalesParCombinaisons(char *msgCode, unsigned long tailleMsgCode, int len_key, unsigned long *nbClefs);
threadInfoKey createThreadInfo(unsigned char *carCandidats, int nbCarCand, unsigned char **actualKeys, unsigned long debutSegment,
    unsigned long finSegment, unsigned char **new_keys, unsigned long tailleclef, int fileInDescriptor, int fileOutDescriptor);

// 2eme version : stockage RAM trop petit
// solution : par fichier
// probleme probable : trop lent
//void *ajouteCandidatsThreads2(void *threadIntel);

#endif
