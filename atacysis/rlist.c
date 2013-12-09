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

#if HAVE_CONFIG_H
#include <config.h>
#endif

#if HAVE_STDLIB_H
#include <stdlib.h>
#endif

#include <stdio.h>

#include "portable.h"
#include "rlist.h"

static char const rlist_c[] = "$Id: rlist.c,v 3.5 2013/12/08 20:07:35 tom Exp $";
/*
* @Log: rlist.c,v @
* Revision 3.4  1995/12/29 21:24:41  tom
* adjust headers, prototyped for autoconfig
*
* Revision 3.3  94/04/04  13:51:02  saul
* Fix binary copyright.
* 
* Revision 3.2  94/04/04  10:26:14  jrh
* Add Release Copyright
* 
* Revision 3.1  93/08/04  15:58:14  ewk
* Added MVS and solaris support.  Squelched some ANSI warnings.
* 
* Revision 3.0  92/11/06  07:47:58  saul
* propagate to version 3.0
* 
* Revision 2.4  92/10/30  09:55:35  saul
* include portable.h
* 
* Revision 2.3  92/05/01  12:55:47  saul
* Bug caused inappropriate highlighting of covered code near uncovered code.
* 
* Revision 2.2  92/03/17  15:27:13  saul
* copyright
* 
* Revision 2.1  91/06/19  13:10:07  saul
* Propagate to version 2.0
* 
* Revision 1.1  91/06/12  16:58:34  saul
* Aug 1990 baseline
* 
*-----------------------------------------------end of log
*/

RLIST **
rlist_create(void)
{
    RLIST **head;

    head = (RLIST **) malloc(sizeof(RLIST **));
    *head = NULL;

    return head;
}

void
rlist_free(RLIST ** head)
{
    RLIST *p;
    RLIST *next;

    if (head == NULL)
	return;

    for (p = *head; p; p = next) {
	next = p->next;
	free(p);
    }

    free(head);
}

/*
* rlist_put:  Insert the range [line1/col1 through line2/col2] in
*	the ordered list "head".  If ranges overlap, combine them.
*/
void
rlist_put(RLIST ** head,
	  int line1,
	  int col1,
	  int line2,
	  int col2)
{
    RLIST *p;
    RLIST *q;
    RLIST **next;
    RLIST *new;
    int sLine;
    int sCol;
    int eLine;
    int eCol;

    if (head == NULL)
	return;

    /*
     * Put start before end.
     */
    if (line1 > line2 || (line1 == line2 && col1 > col2)) {
	eLine = line1;
	eCol = col1;
	sLine = line2;
	sCol = col2;
    } else {
	sLine = line1;
	sCol = col1;
	eLine = line2;
	eCol = col2;
    }

    /*
     * Find place to insert range.
     */
    next = head;
    for (p = *next; p; p = p->next) {
	if (eLine > p->sLine || (eLine == p->sLine && eCol >= p->sCol - 1)) {
	    /*
	     *  Disjoint range?
	     */
	    if (sLine > p->eLine || (sLine == p->eLine && sCol > p->eCol + 1))
		break;

	    /*
	     * Extend end of range if necessary.
	     */
	    if (eLine > p->eLine || (eLine == p->eLine && eCol > p->eCol)) {
		p->eLine = eLine;
		p->eCol = eCol;
	    }

	    /*
	     *  Check for overlap with preceeding ranges.
	     */
	    for (q = p->next; q; q = q->next) {
		if (sLine < q->eLine ||
		    (sLine == q->eLine && sCol <= q->eCol + 1)) {
		    /*
		     * Combine *q and *p and free *q.
		     */
		    p->sLine = q->sLine;
		    p->sCol = q->sCol;
		    p->next = q->next;
		    free(q);
		    q = p;
		} else
		    break;
	    }

	    /*
	     * Extend begining of range if necessary.
	     */
	    if (sLine < p->sLine || (sLine == p->sLine && sCol < p->sCol)) {
		p->sLine = sLine;
		p->sCol = sCol;
	    }
	    return;
	}
	next = &p->next;
    }

    /*
     * Insert disjoint range.
     */
    new = (RLIST *) malloc(sizeof *new);
    new->eLine = eLine;
    new->eCol = eCol;
    new->sLine = sLine;
    new->sCol = sCol;
    new->next = p;
    *next = new;
}

void
rlist_reverse(RLIST ** head)
{
    RLIST *p;
    RLIST *next;
    RLIST *prev;

    if (head == NULL)
	return;

    prev = NULL;
    for (p = *head; p; p = next) {
	next = p->next;
	p->next = prev;
	prev = p;
    }
    *head = prev;
}

#if defined(TEST) || defined(TEST2)
static void
rlist_print(RLIST ** head)
{
    RLIST *p;

    if (head == NULL)
	return;

    for (p = *head; p; p = p->next)
	printf("[%d, %d] --> [%d, %d]\n", p->sLine, p->sCol, p->eLine, p->eCol);
}
#endif

int
rlist_get(RLIST ** head,
	  int *sLine,
	  int *sCol,
	  int *eLine,
	  int *eCol)
{
    RLIST *p;

    if (head == NULL)
	return 0;

    p = *head;

    if (p == NULL)
	return 0;

    *head = p->next;

    *sLine = p->sLine;
    *sCol = p->sCol;
    *eLine = p->eLine;
    *eCol = p->eCol;

    free(p);

    return 1;
}

#ifdef TEST
static char bellcoreCopyRight[] =
"Copyright (c) 1993 Bell Communications Research, Inc. (Bellcore)";
int
main(void)
{
    int sLine, sCol, eLine, eCol;
    RLIST **h;

    h = rlist_create();
    while (scanf("%d %d %d %d", &sLine, &sCol, &eLine, &eCol) == 4)
	rlist_put(h, sLine, sCol, eLine, eCol);
    printf("--- forward ---\n");
    rlist_print(h);
    printf("--- reverse ---\n");
    rlist_reverse(h);
    rlist_print(h);
    rlist_free(h);
}
#endif

#ifdef TEST2
static char bellcoreCopyRight[] =
"Copyright (c) 1993 Bell Communications Research, Inc. (Bellcore)";
int
main(void)
{
    RLIST **h;

    rlist_put(0, 0, 0, 0, 0);

    h = rlist_create();

    rlist_put(h, 31, 1, 33, 1);
    rlist_put(h, 43, 1, 45, 1);
    rlist_put(h, 50, 1, 53, 1);
    rlist_put(h, 10, 1, 13, 1);
    rlist_put(h, 22, 1, 25, 1);
    rlist_put(h, 60, 1, 65, 1);
    rlist_put(h, 72, 1, 75, 1);
    rlist_put(h, 80, 1, 82, 1);
    rlist_put(h, 90, 1, 92, 1);
    rlist_put(h, 94, 1, 95, 1);
    rlist_put(h, 102, 1, 104, 1);
    rlist_put(h, 106, 1, 108, 1);
    rlist_put(h, 110, 1, 113, 1);
    rlist_put(h, 125, 1, 120, 1);
    rlist_put(h, 12, 1, 15, 1);
    rlist_put(h, 20, 1, 23, 1);
    rlist_put(h, 30, 1, 35, 1);
    rlist_put(h, 40, 1, 43, 1);
    rlist_put(h, 53, 1, 55, 1);
    rlist_put(h, 60, 1, 65, 1);
    rlist_put(h, 70, 1, 75, 1);
    rlist_put(h, 80, 1, 85, 1);
    rlist_put(h, 91, 1, 94, 1);
    rlist_put(h, 100, 1, 115, 1);

    printf("--- lines forward ---\n");
    rlist_print(h);

    rlist_reverse(h);
    printf("--- lines reverse ---\n");
    rlist_print(h);

    rlist_free(h);

    h = rlist_create();

    rlist_put(h, 1, 31, 1, 33);
    rlist_put(h, 1, 43, 1, 45);
    rlist_put(h, 1, 50, 1, 53);
    rlist_put(h, 1, 10, 1, 13);
    rlist_put(h, 1, 22, 1, 25);
    rlist_put(h, 1, 60, 1, 65);
    rlist_put(h, 1, 72, 1, 75);
    rlist_put(h, 1, 80, 1, 82);
    rlist_put(h, 1, 90, 1, 92);
    rlist_put(h, 1, 94, 1, 95);
    rlist_put(h, 1, 102, 1, 104);
    rlist_put(h, 1, 106, 1, 108);
    rlist_put(h, 1, 110, 1, 113);
    rlist_put(h, 1, 125, 1, 120);
    rlist_put(h, 1, 12, 1, 15);
    rlist_put(h, 1, 20, 1, 23);
    rlist_put(h, 1, 30, 1, 35);
    rlist_put(h, 1, 40, 1, 43);
    rlist_put(h, 1, 53, 1, 55);
    rlist_put(h, 1, 60, 1, 65);
    rlist_put(h, 1, 70, 1, 75);
    rlist_put(h, 1, 80, 1, 85);
    rlist_put(h, 1, 91, 1, 94);
    rlist_put(h, 1, 100, 1, 115);

    printf("--- columns forward ---\n");
    rlist_print(h);

    rlist_reverse(h);
    printf("--- columns reverse ---\n");
    rlist_print(h);

    rlist_free(h);
}
#endif
