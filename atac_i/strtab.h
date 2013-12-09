/* $Id: strtab.h,v 3.3 2013/12/08 22:05:51 tom Exp $ */

#ifndef strtab_H
#define strtab_H

#include "table.h"

#define BUFSIZE		500

typedef long ID_TYPE;

typedef struct buffer {
    struct buffer *next;
    long buf[BUFSIZE / sizeof(long)];
} BUFFER;

typedef struct strtab {
    BUFFER *buf_list;		/* List of buffers to be freed */
    char *buf_ptr;		/* Next available position. */
    int size;			/* Number of bytes left in current buffer */
    /* May be negative (when padding goes over). */
    TABLE *index;		/* Table to find duplicates. */
    void *upfix;		/* Unique prefix table. */
} STRTAB;

/* forward declarations */
extern char *strtab_upfix(STRTAB * strtab);
extern void strtab_free(STRTAB * strtab);
extern char *strtab_insert(STRTAB * strtab, char *str, ID_TYPE ** id);
extern STRTAB *strtab_create(void);

#endif /* strtab_H */
