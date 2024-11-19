#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include <fcntl.h>
#include "./utiL.h"

bool argContainsHelp(int argc, char* argv[]) {
    int i = 0;
    bool helpPresent = false;

    while ( i != argc && !helpPresent ) {
        helpPresent = ( strstr(argv[i], "-h") != NULL ); // not equal to -h
        helpPresent = ( helpPresent || (strstr(argv[i], "--help") != NULL) ); // not equal to --help
        ++i;
    }

    return helpPresent;
}

void pError(void *ptr, char *msg, int exitStatus) {
    if (!ptr) {
        perror(msg);
        exit(exitStatus);
    }
}

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
