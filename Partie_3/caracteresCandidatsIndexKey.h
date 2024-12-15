#ifndef __CARACTERESCANDIDATSINDEXKEY_H__
#define __CARACTERESCANDIDATSINDEXKEY_H__

#include <stdbool.h>
#include <string.h>
#include <sys/types.h>

bool estCaractereValideASCII(unsigned char charDechiffre);
bool estCaractereValideUTF8(unsigned char charDechiffre);
bool estCaractereValideACCENT(unsigned char charDechiffre);

unsigned char **caracteresCandidatsParIndice(char *msgCode, off_t tailleMsgCode, int len_key);

unsigned char *caracteresCandidatIndKey(char *msgCode, off_t tailleMsgCode, int indice, int len_key);

unsigned char *caracteresPossibles(unsigned char *charSet, unsigned char carChiffre);

void affiche_caracteres_candidats(unsigned char **carCand, int lenKey);

#endif
