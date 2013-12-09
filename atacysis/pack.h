#ifndef pack_H
#define pack_H

#include "portable.h"

typedef struct pkLink {
    struct pkLink *lk_next;
    int lk_bufsize;
    byte lk_buf[1];
} pkLink;

typedef struct {
    pkLink *pk_first;
    pkLink *pk_last;
    byte *pk_bufNext;
    int pk_lCount;
    unsigned long pk_lValue;
    byte *pk_bufFirst;
    int pk_fCount;
    unsigned long pk_fValue;
} pkPack;

extern pkPack *pk_create(void);
extern void pk_append(pkPack * pk, unsigned long n);
extern boolean pk_empty(pkPack * pk);
extern unsigned long pk_take(pkPack * pk);
extern void pk_free(pkPack * pk);

#endif /* pack_H */
