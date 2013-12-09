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

static const char reglist_c[] = "$Id: reglist.c,v 3.6 2013/12/08 17:46:23 tom Exp $";
/*
* @Log: reglist.c,v @
* Revision 3.5  1997/05/11 23:06:28  tom
* split-out reglist.h
*
* Revision 3.4  1996/11/13 00:41:37  tom
* change ident to 'const' to quiet gcc
* add forward-ref prototypes
*
* Revision 3.3  94/04/04  10:13:52  jrh
* Add Release Copyright
*
* Revision 3.2  93/08/04  15:47:27  ewk
* Added MVS and solaris support.  Squelched some ANSI warnings.
*
* Revision 3.1  93/07/12  11:14:02  saul
* MVS MODULEID
* MVS reglist ==> reglst for 8 char uniqueness
*
* Revision 3.0  92/11/06  07:46:13  saul
* propagate to version 3.0
*
* Revision 2.3  92/11/02  11:38:49  saul
* remove unused variables
*
* Revision 2.2  92/10/30  09:48:39  saul
* include portable.h
*
* Revision 2.1  92/07/10  13:37:54  saul
* new
*
*-----------------------------------------------end of log
*/
#if HAVE_STDLIB_H
#include <stdlib.h>
#endif
#include <stdio.h>
#include "portable.h"
#include "reglist.h"

/* forward declarations */
static void tree_free(REGNODE * tree);

REGLST *
reglst_create(void)
{
    REGLST *r;

    r = (REGLST *) malloc(sizeof *r);
    if (r == NULL)
	return NULL;		/* out of memory */

    r->tree = NULL;
    r->idno = 0;

    return r;
}

static void
tree_free(REGNODE * tree)
{
    if (tree) {
	tree_free(tree->left);
	tree_free(tree->right);
	free(tree);
    }
}

void
reglst_free(REGLST * reglst)
{
    if (reglst == NULL)
	return;			/* no reglst */

    tree_free(reglst->tree);
    free(reglst);
}

int
reglst_insert(REGLST * reglst,
	      void *data)
{
    REGNODE *n;
    REGNODE **next;

    if (reglst == NULL)
	return -1;		/* no reglst */

    next = &reglst->tree;
    for (n = reglst->tree; n != NULL; n = *next) {
	if (data < n->data)
	    next = &n->left;
	else if (data > n->data)
	    next = &n->right;
	else
	    return n->idno;	/* found */
    }

    n = (REGNODE *) malloc(sizeof *n);
    if (n == NULL)
	return -1;		/* out of memory */

    *next = n;
    n->data = data;
    n->idno = reglst->idno++;
    n->left = NULL;
    n->right = NULL;
    return n->idno;
}
