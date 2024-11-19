#ifndef __MASK_H__
#define __MASK_H__

// file_out might be NULL
char* encrypt_mask(char* file_in, char* key, char* file_out);
char* decrypt_mask(char* file_in, char* key, char* file_out);

#endif
