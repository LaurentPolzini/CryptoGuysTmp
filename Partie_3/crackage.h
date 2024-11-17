#ifndef __CRACKAGE_H__
#define __CRACKAGE_H__

#include "break_code_c1.h"
#include "break_code_c2.h"

int break_code_c1(char *file_in, int keyLength, char *logFile);

int break_code_c2(char *file_in, char *dict_file_in, char *score_out, int keyLength, char *logFile);

int break_code_c3(char *file_in, char *dict_file_in, char *score_out);

char *ouvreEtLitFichier(char *file_in, off_t *sizeMessage);

#endif
