#ifndef __DH_GEN_GROUP_H__
#define __DH_GEN_GROUP_H__

typedef struct {
    int g;
    int p;
} gen_group;

// file_out might be NULL
gen_group generateSophieGermain(char *file_out);

#endif
