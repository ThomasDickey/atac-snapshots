/* $Id: flowgraph.h,v 3.1 1997/05/11 19:55:18 tom Exp $ */

#ifndef flowgraph_H
#define flowgraph_H

#include "dug.h"

typedef struct {
	BLOCK *	s;	/* start block */
	BLOCK *	e;	/* end block */
} SUBGRAPH;

typedef struct gotolist {
	BLOCK *	block;
	struct gotolist *next;
} GOTOLIST;

/* fg_expr.c */
extern void fg_expr P_(( TNODE *n, DUG *dug, BLOCK * sblk, BLOCK ** endblk, BLOCK ** Tendblk, BLOCK ** Fendblk, int var_use, int ptr_use ));

/* fg_module.c */
void fg_body P_(( TNODE *n, DUG *dug, BLOCK * sblk, BLOCK * bblk, BLOCK * cblk, BLOCK * swblk, BLOCK ** endblk, BLOCK ** dblk ));
void flowgraph P_(( TNODE *tree, FILE *outsrc, FILE *outtables, char *prefix ));

/* fg_stmt.c */
extern void fg_stmt P_(( TNODE *arg_n, DUG *dug, BLOCK * arg_sblk, BLOCK * bblk, BLOCK * cblk, BLOCK * swblk, BLOCK ** endblk, BLOCK ** dblk ));

#endif /* flowgraph_H */
