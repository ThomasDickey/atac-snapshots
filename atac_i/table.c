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

static char table_c[] = 
	"$Header: /users/source/archives/atac.vcs/atac_i/RCS/table.c,v 3.3 1994/04/04 10:14:49 jrh Exp $";
/*
*-----------------------------------------------$Log: table.c,v $
*-----------------------------------------------Revision 3.3  1994/04/04 10:14:49  jrh
*-----------------------------------------------FROM_KEYS
*-----------------------------------------------
* Revision 3.3  94/04/04  10:14:49  jrh
* Add Release Copyright
* 
* Revision 3.2  93/08/04  15:48:23  ewk
* Added MVS and solaris support.  Squelched some ANSI warnings.
* 
* Revision 3.1  93/07/12  11:45:59  saul
* MVS MODULEID
* 
* Revision 3.0  92/11/06  07:45:55  saul
* propagate to version 3.0
* 
* Revision 2.4  92/11/02  11:38:27  saul
* test unused parameter to avoid lint warnings
* 
* Revision 2.3  92/10/30  09:49:16  saul
* include portable.h
* 
* Revision 2.2  92/03/17  14:23:06  saul
* copyright
* 
* Revision 2.1  91/06/13  12:39:26  saul
* Propagate to version 2.0
* 
 * Revision 1.1  91/06/12  20:25:53  saul
 * Aug 1990 baseline
 * 
*-----------------------------------------------end of log
*/
#include <stdio.h>
#include "portable.h"

typedef	int DATA;

typedef struct node {
	DATA		*data;
	struct node	*left;
	struct node	*right;
	struct node	*up;
} NODE;

typedef struct {
	NODE	*tree;
	int	(*cmp)();
} TABLE;

/* forward declarations */
DATA *table_insert();
DATA *table_next();
DATA *table_find();
static void tree_free();
void table_free();
TABLE *table_create();
int intcmp();

int
intcmp(a, b)	/* dummy integer compare routine */
int a;
int b;
{
	return a - b;
}

TABLE *
table_create(cmp)
int	(*cmp)();
{
	TABLE *r;

	r = (TABLE *)malloc(sizeof *r);
	if (r == NULL) return NULL;		/* out of memory */

	r->cmp = cmp;
	r->tree = NULL;

	return r;
}

void
table_free(table, datafree)
TABLE	*table;
void	(*datafree)();
{
	if (table == NULL) return;		/* no table */

	tree_free(table->tree, datafree);
	free(table);
}

static void
tree_free(tree, datafree)
NODE	*tree;
void	(*datafree)();
{
	if (tree) {
		tree_free(tree->left, datafree);
		tree_free(tree->right, datafree);
		if (datafree) (*datafree)(tree->data);
		free(tree);
	}
}

/*
* table_find:  If "node" is NULL find first entry matching "key".
*	Set "node" for use on subsequent calls.
*	If "node" is not NULL find next entry matching "key" after
*	entry at "node".
*	Matchtypes are EQ, GT, LT, GE, LE, NE (only EQ implemented).
*	Matchtype may be OR'ed with REVERSE to reverse the ordering
*	(not implemented).
*/
DATA *					/* return pointer to data found */
table_find(table, key, node, matchtype)
TABLE	*table;
DATA	*key;
NODE	**node;
int	matchtype;	/* not implemented */
{
	NODE	*n;
	NODE	*next;
	int	c;
	int	(*cmp)();
	DATA 	*table_next();

	if (matchtype != 0) return NULL;

	if (table == NULL) return NULL;		/* no table */

	cmp = table->cmp;

	if (node && *node) {
		n = *node;
		if (table_next(table, &n) == NULL)
			return NULL;		/* no next after node */
		if (cmp == NULL)
			c = (int)key - *n->data;
		else if (cmp == intcmp)
			c = (int)key - (int)n->data;
		else c = (*cmp)(key, n->data);
		if (c) return NULL;		/* no match */
		*node = n;
		return n->data;			/* match */
	}

	for (n = table->tree; n != NULL; n = next) {
		if (cmp)
			c = (*cmp)(key, n->data);
		else c = (int)key - *n->data;
		if (c < 0) next = n->left;
		else
		if (c > 0) next = n->right;
		else {
			if (node) *node = n;
			return n->data;
		}
	}
	return NULL;				/* not found */
}

DATA *					/* return pointer to data found */
table_next(table, node)
TABLE	*table;
NODE	**node;
{
	NODE	*n;

	if (table == NULL || node == NULL) return NULL;

	n = *node;

	if (n == NULL) {
		n = table->tree;
		if (n) while (n->left) n = n->left;
	}
	else if (n->right) {
		n = n->right;
		while (n->left) n = n->left;
	} else {
		while (n->up && n->up->right == n) n = n->up;
		n = n->up;
	}

	*node = n;
	if (n) return n->data;
	else return NULL;
}

DATA *
table_insert(table, data, duplicates)
TABLE	*table;
DATA	*data;
int	duplicates;
{
	NODE	*n;
	NODE	**next;
	NODE	*prev;
	int	c;
	int	(*cmp)();

	if (table == NULL) return NULL;			/* no table */

	cmp = table->cmp;

	prev = NULL;
	next = &table->tree;
	for (n = table->tree; n != NULL; n = *next) {
		if (cmp == NULL)
		 	c = *data - *n->data;
		else if (cmp == intcmp)
			c = (int)data - (int)n->data;
		else c = (*cmp)(data, n->data);
		if (c < 0) next = &n->left;
		else
		if (c > 0 || duplicates) next = &n->right;
		else
		return NULL;		/* duplicate */
		prev = n;
	}

	n = (NODE *)malloc(sizeof *n);
	if (n == NULL) return NULL;	/* out of memory */

	*next = n;
	n->data = data;
	n->left = NULL;
	n->right = NULL;
	n->up = prev;
	return data;
}
