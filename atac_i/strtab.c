/****************************************************************
*Copyright (c) 1993 Bell Communications Research, Inc. (Bellcore)
*
*Permission to use, copy, modify, and distribute this material
*for any purpose and without fee is hereby granted, provided
*that the above copyright notice and this permission notice
*appear in all copies, and that the name of Bellcore not be
*used in advertising or publicity pertaining to this
*material without the specific, prior written permission
*of an authorized representative of Bellcore.  BELLCORE
*MAKES NO REPRESENTATIONS ABOUT THE ACCURACY OR SUITABILITY
*OF THIS MATERIAL FOR ANY PURPOSE.  IT IS PROVIDED "AS IS",
*WITHOUT ANY EXPRESS OR IMPLIED WARRANTIES.
****************************************************************/
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef MVS
#include <mvapts.h>
MODULEID(%M%,%J%/%D%/%T%)
#endif /* MVS */

static const char strtab_c[] = "$Id: strtab.c,v 3.8 2013/12/08 18:08:17 tom Exp $";
/*
* @Log: strtab.c,v @
* Revision 3.7  1997/12/09 00:03:39  tom
* int/size_t fix
*
* Revision 3.6  1997/05/11 22:08:45  tom
* use ID_TYPE to correct prototype for strtab_insert()
*
* Revision 3.5  1997/05/11 00:16:56  tom
* split-out strtab.h
*
* Revision 3.4  1996/11/13 00:42:39  tom
* change ident to 'const' to quiet gcc
* add forward-ref prototypes
*
* Revision 3.3  94/04/04  10:14:28  jrh
* Add Release Copyright
* 
* Revision 3.2  93/08/04  15:47:57  ewk
* Added MVS and solaris support.  Squelched some ANSI warnings.
* 
* Revision 3.1  93/07/12  11:34:13  saul
* MVS MODULEID
* 
* Revision 3.0  92/11/06  07:45:04  saul
* propagate to version 3.0
* 
* Revision 2.8  92/10/30  09:48:48  saul
* include portable.h
* 
* Revision 2.7  92/09/16  07:35:45  saul
* Idtype stored with string.
* 
* Revision 2.6  92/04/07  08:20:05  saul
* supply default prefix
* 
* Revision 2.5  92/04/07  08:19:16  saul
* supply default prefix
* 
* Revision 2.4  92/04/07  07:40:39  saul
* unique prefix stuff
* 
* Revision 2.3  92/04/07  07:37:06  saul
* added unique prefix stuff
* 
* Revision 2.2  92/03/17  14:22:55  saul
* copyright
* 
* Revision 2.1  91/06/13  12:39:21  saul
* Propagate to version 2.0
* 
* Revision 1.1  91/06/12  20:25:51  saul
* Aug 1990 baseline
* 
*-----------------------------------------------end of log
*/
#if HAVE_STDLIB_H
#include <stdlib.h>
#endif
#include <stdio.h>
#include <string.h>
#include "portable.h"
#include "error.h"
#include "strtab.h"
#include "upfix.h"

#define CHECK_MALLOC(p) ((p) ? 1 : internal_error(NULL, "Out of memory\n"))

/*
* Strtab stores strings in dynamically allocated memory.  Duplicate strings
* share the same space.  There is no explicit limit on string length, but
* long strings tend to waste space at the end of a buffer.
*/
#define UPFIX_MAX_LEN	5
#define DEFAULT_PREFIX	"aTaC_"
#define IDSIZE	sizeof(long)

static size_t alignSize = 0;

/*
* strtab_create:  Return pointer to strtab; to be passed to strtab_insert()
*	and strtab_free().
*/
STRTAB *
strtab_create(void)
{
    STRTAB *strtab;

    /*
       * Set alignSize to one less thatn the least power of 2 greater or
       * equal to IDSIZE.
       * This is assumed to be the maximum required alignment for this type.
     */
    if (alignSize == 0) {	/* first time only */
	alignSize = 1;
	while (alignSize < IDSIZE)
	    alignSize <<= 1;
	--alignSize;
    }

    strtab = (STRTAB *) malloc(sizeof *strtab);
    CHECK_MALLOC(strtab);

    strtab->buf_list = NULL;
    strtab->buf_ptr = NULL;
    strtab->size = 0;
    strtab->index = (TABLE *) table_create(strcmp);
    if (strtab->index == NULL) {
	free(strtab);
	return (NULL);
    }
    strtab->upfix = (void *) upfix_init(UPFIX_MAX_LEN, NULL);
    CHECK_MALLOC(strtab->upfix);

    return strtab;
}

/*
* strtab_insert:  Copy string into table if not already present.  Return
*	pointer to string in table.
*/
char *
strtab_insert(STRTAB * strtab,
	      char *str,
	      ID_TYPE ** id)
{
    char *s;
    BUFFER *buf;
    int size;

    if (strtab->index == NULL)
	return NULL;

    s = (char *) table_find(strtab->index, str, 0, 0);
    if (s) {
	if (id)
	    *id = (ID_TYPE *) (s - IDSIZE);
	return s;
    }

    size = IDSIZE + strlen(str) + 1;
    if (strtab->size < size) {
	buf = (BUFFER *) malloc(sizeof *buf + size);
	CHECK_MALLOC(buf);
	buf->next = strtab->buf_list;
	strtab->buf_list = buf;
	strtab->buf_ptr = (char *) (buf->buf);
	strtab->size = size + sizeof buf->buf;
    }

    *(long *) strtab->buf_ptr = 0;
    strcpy(strtab->buf_ptr + IDSIZE, str);
    s = (char *) table_insert(strtab->index, strtab->buf_ptr + IDSIZE, 0);
    if (id)
	*id = (ID_TYPE *) (strtab->buf_ptr);
    size = (size + alignSize) & ~alignSize;
    strtab->buf_ptr += size;
    strtab->size -= size;
    upfix_exclude(strtab->upfix, str);

    return s;
}

void
strtab_free(STRTAB * strtab)
{
    BUFFER *p;
    BUFFER *next;

    for (p = strtab->buf_list; p != NULL; p = next) {
	next = p->next;
	free(p);
    }

    upfix_free(strtab->upfix);

    free(strtab);
}

char *
strtab_upfix(STRTAB * strtab)
{
    char *prefix;

    prefix = (char *) upfix(strtab->upfix);
    if (prefix == NULL)
	return DEFAULT_PREFIX;
    else
	return prefix;
}
