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

static char reglist_c[] = 
	"$Header: /users/source/archives/atac.vcs/atac_i/RCS/reglist.c,v 3.3 1994/04/04 10:13:52 jrh Exp $";
/*
*-----------------------------------------------$Log: reglist.c,v $
*-----------------------------------------------Revision 3.3  1994/04/04 10:13:52  jrh
*-----------------------------------------------FROM_KEYS
*-----------------------------------------------
*Revision 3.3  94/04/04  10:13:52  jrh
*Add Release Copyright
*
*Revision 3.2  93/08/04  15:47:27  ewk
*Added MVS and solaris support.  Squelched some ANSI warnings.
*
ist.c
*Revision 3.1  93/07/12  11:14:02  saul
*MVS MODULEID
*MVS reglist ==> reglst for 8 char uniqueness
*
*Revision 3.0  92/11/06  07:46:13  saul
*propagate to version 3.0
*
*Revision 2.3  92/11/02  11:38:49  saul
*remove unused variables
*
*Revision 2.2  92/10/30  09:48:39  saul
*include portable.h
*
*Revision 2.1  92/07/10  13:37:54  saul
*new
*
*-----------------------------------------------end of log
*/
#include <stdio.h>
#include "portable.h"

typedef struct node {
	void		*data;
	int		idno;
	struct node	*left;
	struct node	*right;
} NODE;

typedef struct {
	NODE	*tree;
	int	idno;
} REGLST;

/* forward declarations */
int reglst_insert();
void reglst_free();
static void tree_free();
REGLST *reglst_create();

REGLST *
reglst_create()
{
	REGLST *r;

	r = (REGLST *)malloc(sizeof *r);
	if (r == NULL) return NULL;		/* out of memory */

	r->tree = NULL;
	r->idno = 0;

	return r;
}

static void
tree_free(tree)
NODE	*tree;
{
	if (tree) {
		tree_free(tree->left);
		tree_free(tree->right);
		free(tree);
	}
}

void
reglst_free(reglst)
REGLST	*reglst;
{
	if (reglst == NULL) return;		/* no reglst */

	tree_free(reglst->tree);
	free(reglst);
}

int
reglst_insert(reglst, data)
REGLST	*reglst;
void	*data;
{
	NODE	*n;
	NODE	**next;

	if (reglst == NULL) return -1;		/* no reglst */

	next = &reglst->tree;
	for (n = reglst->tree; n != NULL; n = *next) {
		if (data < n->data) next = &n->left;
		else
		if (data > n->data) next = &n->right;
		else return n->idno;		/* found */
	}

	n = (NODE *)malloc(sizeof *n);
	if (n == NULL) return -1;	/* out of memory */

	*next = n;
	n->data = data;
	n->idno = reglst->idno++;
	n->left = NULL;
	n->right = NULL;
	return n->idno;
}
