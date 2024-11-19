#ifndef __XOR_H__
#define __XOR_H__

#include <unistd.h>

char* encrypt_decrypt_xor(char* file_in, char* key, char* file_out);
char* encrypt_decrypt_xorMSG(char* msg, char* key, off_t tailleMsg);
char* encrypt_mask(char* file_in, char* key, char* file_out);
char* decrypt_mask(char* file_in, char* key, char* file_out);
char* gen_key(int n, char* key);
void remove_padding(char *file_out);
int encrypt_cbc(char *file_in, char *file_key, char *file_out, char *v_init, void *method);
int decrypt_cbc(char *file_in, char *file_key, char *file_out, char *v_init, void *method);
char* charger_cle_depuis_fichier(const char* nom_fichier);
#endif
