/* $Id: strtab.h,v 3.2 1997/05/11 22:05:19 tom Exp $ */

#ifndef strtab_H
#define strtab_H

#include "table.h"

#define BUFSIZE		500

typedef long ID_TYPE;

typedef struct buffer {
	struct buffer	*next;
	long		buf[BUFSIZE/sizeof(long)];
} BUFFER;

typedef struct strtab {
	BUFFER 	*buf_list;	/* List of buffers to be freed */
	char	*buf_ptr;	/* Next available position. */
	int	size;		/* Number of bytes left in current buffer */
	                        /* May be negative (when padding goes over). */
	TABLE	*index;		/* Table to find duplicates. */
	void	*upfix;		/* Unique prefix table. */
} STRTAB;

/* forward declarations */
extern char *strtab_upfix P_(( STRTAB *strtab ));
extern void strtab_free P_(( STRTAB *strtab ));
extern char *strtab_insert P_(( STRTAB *strtab, char *str, ID_TYPE **id ));
extern STRTAB *strtab_create P_(( void ));

#endif /* strtab_H */
