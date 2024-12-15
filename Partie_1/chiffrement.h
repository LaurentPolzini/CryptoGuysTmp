#ifndef __CHIFFREMENT_H__
#define __CHIFFREMENT_H__

#include <stdbool.h>
#include <sys/types.h>

char* gen_key(int n, char* key, bool mask);
void remove_padding(char *file_out);

void appel_chiffrement(char* namefile_in, char* namefile_out, char* key, char* methode, char* v_init);

char* encrypt_decrypt_xor(char* file_in, char* key, char* file_out);
char* encrypt_decrypt_xorMSG(char* msg, char* key, off_t tailleMsg);

int encrypt_cbc(char *file_in, char *file_key, char *file_out, char *v_init);
int decrypt_cbc(char *file_in, char *file_key, char *file_out, char *v_init);

// file_out might be NULL
char* encrypt_mask(char* file_in, char* key, char* file_out);
char* decrypt_mask(char* file_in, char* key, char* file_out);

#endif
