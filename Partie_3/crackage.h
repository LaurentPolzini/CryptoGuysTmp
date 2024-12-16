#ifndef __CRACKAGE_H__
#define __CRACKAGE_H__

/* 
    method = freq / mask
    in : crypté
    out : 
*/
void appel_crackage(char *method, char *in, int len_key, char *dico, char *logName, char *scoresName, char *crypte2, char *clair1, char *out_clair2);

int break_code_c1(char *file_in, int keyLength, char *logFile);

int break_code_c2(char *file_in, float *stats, char *score_out, int keyLength, char *logFileName);

int break_code_c3(char *file_in, char *dict_file_in, char *score_out, int keyLen, char *logFile);

// keyLen used as the maximum size (tests every size under keyLen)
int break_code_all_exact_len(char *file_in, char *dict_file_in, char *score_out, int keyLen, char *logFileName);
// keyLen used as the exact size
int break_code_all_max_len(char *file_in, char *dict_file_in, char *score_out, int keyLen, char *logFileName);

void crack_mask(const char* file_c1, const char* file_c2, const char* file_m1, const char* file_m2);

#endif
