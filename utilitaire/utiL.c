#include <stdio.h>
#include "./utiL.h"
#include <stdbool.h>
#include <string.h>

bool argContainsHelp(int argc, char* argv[]) {
    int i = 0;
    bool found = false;
    bool isNotHelp = true;

    while ( i != argc && isNotHelp ) {
        isNotHelp = ( strstr(argv[i], "-h") == NULL ); // not equal to -h
        isNotHelp = ( isNotHelp && (strstr(argv[i], "--help") == NULL) ); // not equal to --help
        ++i;
    }

    return found;
}

