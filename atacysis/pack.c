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
#ifdef MVS
#include <mvapts.h>
MODULEID(%M%,%J%/%D%/%T%)
#endif /* MVS */

static char pack_c[] = 
	"$Header: /users/source/archives/atac.vcs/atacysis/RCS/pack.c,v 3.4 1994/04/04 10:25:49 jrh Exp $";
/*
*-----------------------------------------------$Log: pack.c,v $
*-----------------------------------------------Revision 3.4  1994/04/04 10:25:49  jrh
*-----------------------------------------------FROM_KEYS
*-----------------------------------------------
*Revision 3.4  94/04/04  10:25:49  jrh
*Add Release Copyright
*
*Revision 3.3  93/08/04  15:57:10  ewk
*Added MVS and solaris support.  Squelched some ANSI warnings.
*
* Revision 3.2  93/03/31  11:42:45  saul
*Void function should be int.
*
*Revision 3.1  93/03/26  11:11:46  saul
*Initial version.
*
*-----------------------------------------------end of log
*/
/*
* Pack:  Pack a queue of four byte non-negative numbers to save space.
* The following access functions are provided:
*	pk_create - create a pkPack
*	pk_append - append a number to the end of a pkPack
*	pk_take - take a number off the beggingin of a pkPack and return it.
*	pk_free - free the storage for a pkPack
*
* Internal representation:
*	pk_lValue is the last number in the queue.
*	pk_lCount is the number of consecutive occurances of pk_lValue.
*	pk_fValue is the first number in the queue if pk_fCount is non-zero.
*	pk_fCount is the number of consecutive occurances of pk_fValue.
*	pk_fValue and pk_fCount are used when taking values off the front
*	of the queue.
*	pk_first points to a pkLink containing a buffer of "runs" and a link to
*	the next pkLink.  If the high order bit of the first byte in a "run"
*	is 0, it is a run of 0's of length equal to the byte value.
*	If the high order bit is a 1, it is a run of non-zeros of length
*	equal to the value of the last five bits of the byte plus 1.
*	The non-zero value is represented by the next 1, 2, 3, or 4 bytes.
*	The number of bytes is one more than the value of the second and third
*	bits of the first byte.  The bytes of the non-zero value are in order
*	with low order first.  The last 4 bytes of lk_buf are not used unless
*	they are part of the value part of a run of non-zeros.  Unused bytes
*	are filled with 0 when starting a new pkLink.
*
*          bit 7 | bits 0-6 ||
*        ---------------------
*        ||   0  |  count   ||			count 0's
*        ---------------------
*
*          bit 7 | bits 6,5 | bits 0-4 || bits 0-7
*        -------------------------------------------
*        ||   1  |   0 0    |  count   ||  value  ||	count values
*        -------------------------------------------
*
*          bit 7 | bits 6,5 | bits 0-4 || bits 0-7  ||  bits 0-7
*        ---------------------------------------------------------
*        ||   1  |   0 1    |  count   || lo value  || hi value || 
*        ---------------------------------------------------------
*
*/
#include <stdio.h>
#include "portable.h"

#define BIT5	(1 << 5)
#define BIT7	(1 << 7)

#ifdef TEST
#define INIT_BUF_SIZE	10
#define ADD_BUF_SIZE	10
#else
#define INIT_BUF_SIZE	200
#define ADD_BUF_SIZE	16
#endif

typedef struct pkLink {
    struct pkLink	*lk_next;
    int			lk_bufsize;
    byte		lk_buf[1];
} pkLink;

typedef struct {
    pkLink		*pk_first;
    pkLink		*pk_last;
    byte		*pk_bufNext;
    int			pk_lCount;
    unsigned long	pk_lValue;
    byte		*pk_bufFirst;
    int			pk_fCount;
    unsigned long	pk_fValue;
} pkPack;

/* forward declarations */
void pk_free();
unsigned long int pk_take();
int pk_empty();
void pk_append();
pkPack *pk_create();
static char *extend();

/*
* extend: Add a pkLink at the end of pk and return pointer to it's lk_buf.
*/
static byte *
extend(pk)
pkPack		*pk;
{
    pkLink	*new;
    byte	*buf;
    byte	*end;
    int		bufsize;

    if (pk->pk_first == NULL) {
	bufsize = INIT_BUF_SIZE;
	new = (pkLink *)malloc(sizeof *new + bufsize - 1);
	pk->pk_first = new;
	pk->pk_bufFirst = new->lk_buf;	/* Okay before (new == NULL) check! */
    } else {
	end = pk->pk_last->lk_buf + pk->pk_last->lk_bufsize;
	for (buf = pk->pk_bufNext; buf < end; ++buf)
	    *buf++ = '\0';
	bufsize = ADD_BUF_SIZE;
	new = (pkLink *)malloc(sizeof *new + bufsize - 1);
	pk->pk_last->lk_next = new;
    }
    if (new == NULL) {
	fprintf(stderr, "can't malloc\n");
	exit(1);
    }
    new->lk_next = NULL;
    new->lk_bufsize = bufsize;
    pk->pk_last = new;
    pk->pk_bufNext = new->lk_buf;

    return new->lk_buf;
}

/*
* pk_create: Create an empty pkPack.
*/
pkPack *
pk_create()
{
    pkPack	*new;

    new = (pkPack *)malloc(sizeof *new);
    if (new == NULL) {
	fprintf(stderr, "can't malloc\n");
	exit(1);
    }

    new->pk_first = NULL;
    new->pk_last = NULL;
    new->pk_bufNext = NULL;
    new->pk_lCount = 0;
    new->pk_lValue = 0;
    new->pk_bufFirst = NULL;
    new->pk_fCount = 0;
    new->pk_fValue = 0;

    return new;
}

/*
* pk_append: Append a long integer to pk.
*/
void
pk_append(pk, n)
pkPack		*pk;
unsigned long	n;
{
    pkLink		*last;
    byte		*buf;
    int			count;
    unsigned long	value;
    int			size;

    count = pk->pk_lCount;
    value = pk->pk_lValue;

    if (n == value) {
	++pk->pk_lCount;
	return;
    }

    if (count == 0) {
	pk->pk_lValue = n;
	pk->pk_lCount = 1;
	return;
    }
    
    last = pk->pk_last;

    /*
     * Find room for up to 5 bytes.  (Unpacker will not look for run beginning
     * beyond lk_buf[lk_bufsize - 4].)
     */
    buf = pk->pk_bufNext;
    if (last == NULL || buf >= last->lk_buf + last->lk_bufsize - 4) {
	pk->pk_bufNext = buf;
	buf = extend(pk);
	last = pk->pk_last;
    }

    if (value == 0) {
	while (count >= BIT7) {		/* e.g. >= 128 */
	    *buf++ = BIT7 - 1;
	    count -= BIT7 - 1;
	    if (buf >= last->lk_buf + last->lk_bufsize - 4) {
		pk->pk_bufNext = buf;
		buf = extend(pk);
		last = pk->pk_last;
	    }
	}
	*buf++ = count;
    } else {
	while (count > BIT5) {	/* e.g. 32 */
	    count -= BIT5;
	    size = 0;
	    do {
		*++buf = LOBYTE(value);
		value >>= 8;
		++size;
	    } while (value != 0);
	    buf[-size] = BIT7 | ((size - 1) << 5) | (BIT5 - 1);
	    ++buf;
	    value = pk->pk_lValue;
	    if (buf >= last->lk_buf + last->lk_bufsize - 4) {
		pk->pk_bufNext = buf;
		buf = extend(pk);
		last = pk->pk_last;
	    }
	}
	size = 0;
	do {
	    *++buf = LOBYTE(value);
	    value >>= 8;
	    ++size;
	} while (value != 0);
	buf[-size] = BIT7 | ((size - 1) << 5) | (count - 1);
	++buf;
    }
    pk->pk_bufNext = buf;

    pk->pk_lCount = 1;
    pk->pk_lValue = n;
}

/*
* pk_empty: return TRUE if the pkPack is empty.
*/
boolean
pk_empty(pk)
pkPack		*pk;
{
    if (pk->pk_lCount == 0)
	return TRUE;
    else return FALSE;
}

/*
* pk_take
*/
unsigned long
pk_take(pk)
pkPack		*pk;
{
    pkLink	*first;
    byte	*buf;
    int		b;
    int		size;
    int		count;
    int		shift;
    unsigned long	value;

    if (pk->pk_fCount != 0) {
	--pk->pk_fCount;
	return pk->pk_fValue;
    }

    first = pk->pk_first;
    if (first == NULL) {
	if (pk->pk_lCount != 0) {
	    --pk->pk_lCount;
	    return pk->pk_lValue;
	}
	else return 0;		/* empty. */
    }

    buf = pk->pk_bufFirst;
    b = BYTE(*buf++);
    if (b < BIT7) {		/* e.g. < 128 */
	count = b - 1;
	value = 0;
    } else {
	count = b & (BIT5 - 1);
	size = (b & (BIT7 - 1)) >> 5;
	shift = 8;
	value = BYTE(*buf++);
	while (size--) {
	    value |= BYTE(*buf++) << shift;
	    shift += 8;
	}
    }
    pk->pk_fCount = count;
    pk->pk_fValue = value;
    pk->pk_bufFirst = buf;

    /*
     * If we emptied a pkLink, free it.
     */
    if (first == pk->pk_last) {
	if (buf >= pk->pk_bufNext) {
	    pk->pk_first = NULL;
	    pk->pk_last = NULL;
	    pk->pk_bufNext = NULL;
	    pk->pk_bufFirst = NULL;
	    free(first);
	}
    }
    else if (buf >= first->lk_buf + first->lk_bufsize - 4) {
	pk->pk_first = first->lk_next;
	pk->pk_bufFirst = first->lk_next->lk_buf;
	free(first);
    }

    return value;
}

/*
* pk_free: Release memory used by pk.
*/
void
pk_free(pk)
pkPack	*pk;
{
    pkLink	*link;
    pkLink	*next;

    for (link = pk->pk_first; link != NULL; link = next) {
	next = link->lk_next;
	free(link);
    }

    free(pk);

    return;
}

#ifdef TEST
main(argc, argv)
int argc;
char *argv[];
{
    unsigned long	n;
    pkPack		*pk;
    int			i;

    pk = pk_create();

    if (argc == 2 && strcmp(argv[1], "-") == 0) {
	pk_append(pk, 10);
	n = pk_take(pk);
	printf("%lu\n", n);
	n = pk_take(pk);
	printf("%lu\n", n);
	pk_append(pk, 11);
	pk_append(pk, 12);
	pk_append(pk, 13);
	pk_free(pk);
	exit(0);
    }
	
    if (isatty(0)) printf("> ");
    while (scanf("%lu", &n) > 0)
	pk_append(pk, n);
    while (!pk_empty(pk)) {
	n = pk_take(pk);
	printf("%lu ", n);
    }
    printf("\n");
    pk_free(pk);
}
#endif /* TEST */


