#ifndef __CBC_H__
#define __CBC_H__

int encrypt_cbc(char *file_in, char *file_key, char *file_out, char *v_init, void *method);

int decrypt_cbc(char *file_in, char *file_key, char *file_out, char *v_init, void *method);

#endif
