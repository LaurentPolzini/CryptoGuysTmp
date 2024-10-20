#include <stdio.h>
#include <getopt.h>
#include "../Partie_1/mask.h"
#include "../Partie_1/xor.h"

/*
int main(int argc, char *argv[]) {
    if ( !((argc == 5) || (argc == 4)) ) {
        perror("Usage : ./crack_mask crypted1_file crypted2_file [test_file] decrypted_out_file");
    }
    char *crypted_1;
    char *crypted_2;
    char *clearByUser;
    char *clearResultOut;
    
    crypted_1 = argv[1];
    crypted_2 = argv[2];

    if (argc == 5) {
        clearByUser = argv[3];
        clearResultOut = argv[4];
    } else {
        clearByUser = NULL;
        clearResultOut = argv[3];
    }
    (void) crypted_1, (void) crypted_2, (void) clearByUser, (void) clearResultOut;


    return 0;
}
*/
