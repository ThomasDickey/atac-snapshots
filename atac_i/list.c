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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

static const char list_c[] = "$Id: list.c,v 3.11 2013/12/09 00:17:33 tom Exp $";
/*
* @Log: list.c,v @
* Revision 3.9  2005/08/14 13:45:53  tom
* gcc warning (implicit type)
*
* Revision 3.8  1997/05/11 20:55:27  tom
* rename DATA to LIST_DATATYPE
*
* Revision 3.7  1997/05/11 16:38:02  tom
* rename type from LINK to LIST, for consistency
*
* Revision 3.5  1996/11/13 00:58:59  tom
* change ident to 'const' to quiet gcc
* add forward-ref prototypes
*
* Revision 3.4  94/04/04  10:13:19  jrh
* Add Release Copyright
* 
* Revision 3.3  93/08/09  13:11:35  ewk
* Print the contents of "data" rather than "*data" at line 255.
* 
* Revision 3.2  93/08/04  15:46:16  ewk
* Added MVS and solaris support.  Squelched some ANSI warnings.
* 
* Revision 3.1  93/07/12  10:53:56  saul
* MVS MODULEID
* 
* Revision 3.0  92/11/06  07:45:53  saul
* propagate to version 3.0
* 
* Revision 2.3  92/10/30  09:48:24  saul
* include portable.h
* 
* Revision 2.2  92/03/17  14:22:33  saul
* copyright
* 
* Revision 2.1  91/06/13  12:39:09  saul
* Propagate to version 2.0
* 
* Revision 1.1  91/06/12  20:25:45  saul
* Aug 1990 baseline
* 
*-----------------------------------------------end of log
*/
#include <stdio.h>
#include <portable.h>

#if HAVE_STDLIB_H
#include <stdlib.h>
#endif

#include "list.h"

#ifdef DEBUG
static char MAGIC[1];		/* Guaranteed unique address */
#endif

LIST *
list_create(void)
{
    LIST *head;			/* points to list pointer */

    head = (LIST *) malloc(sizeof *head);
    if (head == NULL)
	return NULL;
#ifdef DEBUG
    head->magic = MAGIC;
#endif
    head->data = NULL;		/* not used */
    head->next = head;
    head->prev = head;
    return head;
}

int
list_free(LIST * head,
	  DataFree datafree)
{
    LIST *link;
    LIST *next;

#ifdef DEBUG
    if (head->magic != MAGIC)
	return 0;
#endif

    for (link = head->next; link != head; link = next) {
	next = link->next;
	if (datafree)
	    (*datafree) (link->data);
#ifdef DEBUG
	link->magic = NULL;
#endif
	free(link);
    }
#ifdef DEBUG
    head->magic = NULL;
#endif
    free(head);
    return 1;
}

int
list_delete(LIST * head,
	    LIST ** old)
{
    LIST *d;
    LIST *p;

#ifdef DEBUG
    if (head->magic != MAGIC)
	return 0;
#endif
    if (old == NULL)
	d = head->next;
    else {
	d = *old;
	if (d == NULL)
	    d = head->next;
    }
    if (d == NULL)
	return 0;
#ifdef DEBUG
    if (d->magic != MAGIC)
	return 0;
#endif

    p = d->prev;
    if (p == head)
	p = NULL;
    if (old)
	*old = p;

    d->prev->next = d->next;
    d->next->prev = d->prev;
#ifdef DEBUG
    d->magic = NULL;
#endif
    free(d);

    return 1;
}

int
list_put(LIST * head,
	 LIST_DATATYPE * data)
{
    LIST *new;

#ifdef DEBUG
    if (head->magic != MAGIC)
	return 0;
#endif

    new = (LIST *) malloc(sizeof *new);
    if (new == NULL)
	return 0;
#ifdef DEBUG
    new->magic = MAGIC;
#endif
    new->data = data;
    new->next = head;		/* Add to end of list */
    head->prev->next = new;
    new->prev = head->prev;	/* Add to beginning of backward list */
    head->prev = new;
    return 1;
}

int
list_next(LIST * head,
	  LIST ** prev,
	  LIST_DATATYPE ** data)
{
    LIST *p;

#ifdef DEBUG
    if (head->magic != MAGIC)
	return 0;
#endif

    if (prev && (p = *prev)) {
#ifdef DEBUG
	if (p->magic != MAGIC)
	    return 0;
#endif
	p = p->next;
    } else
	p = head->next;

    if (p == head)
	return 0;

    if (data)
	*data = p->data;
    if (prev)
	*prev = p;

    return 1;
}

int
list_prev(LIST * head,
	  LIST ** prev,
	  LIST_DATATYPE ** data)
{
    LIST *p;

#ifdef DEBUG
    if (head->magic != MAGIC)
	return 0;
#endif

    if (prev && (p = *prev)) {
#ifdef DEBUG
	if (p->magic != MAGIC)
	    return 0;
#endif
	p = p->prev;
    } else
	p = head->prev;

    if (p == head)
	return 0;

    if (data)
	*data = p->data;
    if (prev)
	*prev = p;

    return 1;
}

void
list_dump(LIST * head,
	  char *(*datadump) (LIST_DATATYPE *),
	  const char *label)
{
    LIST *link;
    LIST_DATATYPE *data;
    int i;
    int j;
    static int tab = -1;

    ++tab;
    for (j = 0; j < tab; ++j)
	putc('\t', stderr);
    fprintf(stderr, "--->");
    if (label)
	fprintf(stderr, "%s\n", label);
    else
	fprintf(stderr, " list_dump\n");
#ifdef DEBUG
    if (head->magic != MAGIC) {
	for (j = 0; j < tab; ++j)
	    putc('\t', stderr);
	fprintf(stderr, "error: corrupted list\n");
    } else {
#endif
	i = 0;
	for (link = NULL; list_next(head, &link, &data);) {
	    for (j = 0; j < tab; ++j)
		putc('\t', stderr);
	    fprintf(stderr, "%d:\t", i++);
	    if (datadump)
		(*datadump) (data);
	    else if (data)
		fprintf(stderr, "%p\n", data);
	}
#ifdef DEBUG
    }
#endif
    for (j = 0; j < tab; ++j)
	putc('\t', stderr);
    fprintf(stderr, "<---");
    if (label)
	fprintf(stderr, "%s\n", label);
    else
	fprintf(stderr, " list_dump\n");
    --tab;
}
