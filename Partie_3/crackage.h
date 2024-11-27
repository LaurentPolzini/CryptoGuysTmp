#ifndef __CRACKAGE_H__
#define __CRACKAGE_H__

#include "break_code_c1.h"
#include "break_code_c2.h"

int break_code_c1(char *file_in, int keyLength, char *logFile);

int break_code_c2_c3(char *file_in, char *dict_file_in, char *score_out, int keyLen, char *logFile);

void crack_mask(const char* file_c1, const char* file_c2, const char* file_m1, const char* file_m2);

#endif
