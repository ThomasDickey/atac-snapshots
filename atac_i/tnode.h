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
#ifndef tnode_H
#define tnode_H
static const char tnode_h[] = 
	"$Header: /users/source/archives/atac.vcs/atac_i/RCS/tnode.h,v 3.2 1996/11/13 00:26:35 tom Exp $";
/*
* $Log: tnode.h,v $
* Revision 3.2  1996/11/13 00:26:35  tom
* change ident to 'const' to quiet gcc
* prototyped functions for tree.c, tnode.c
*
* Revision 3.1  94/04/04  10:15:02  jrh
* Add Release Copyright
* 
* Revision 3.0  92/11/06  07:45:42  saul
* propagate to version 3.0
* 
* Revision 2.3  92/09/22  15:24:11  saul
* Declare child() functions incase macros are suppressed.
* 
* Revision 2.2  92/03/17  14:23:08  saul
* copyright
* 
* Revision 2.1  91/06/13  12:39:27  saul
* Propagate to version 2.0
* 
* Revision 1.1  91/06/12  20:25:54  saul
* Aug 1990 baseline
* 
*-----------------------------------------------end of log
*/
#define LEFT_SRCPOS	0
#define RIGHT_SRCPOS	1

typedef struct tnode {
	int		genus;
	int		species;
	SRCPOS		srcpos[2];	/* LEFT_SRCPOS, RIGHT_SRCPOS */
	struct tnode 	*up;
	struct tnode 	*down;
	struct tnode 	*over;
	char		*text;
	union {
		struct symlist	*symtab;
		struct sym	*sym;
		struct {
					short blkno;
					short tempno;
					struct valtype *type;
		}		 hook;
	} sym;
} TNODE;

/* srcpos.c */
#if 0
extern char *srcfname P_(( int findex));
extern int srcfstamp P_(( int findex));
extern int store_filename P_(( char *s));
extern void node_isrcpos P_(( TNODE *node, int left, FILE *f));
extern void node_srcpos P_(( TNODE *node, int left, FILE *f));
extern void print_srcpos P_(( SRCPOS *srcpos, FILE *f));
#endif

/* tree.c */
TNODE *tlist_add P_(( TNODE *list, TNODE *next ));
TNODE *tmknode P_(( int genus, int species, TNODE *child0, TNODE *child1 ));
TNODE *tmkleaf P_(( int genus, int species, SRCPOS *srcpos, char *text ));

/* tnode.c */
TNODE *child0 P_(( TNODE *n ));
TNODE *child1 P_(( TNODE *n ));
TNODE *child2 P_(( TNODE *n ));
TNODE *child3 P_(( TNODE *n ));
TNODE *child4 P_(( TNODE *n ));
TNODE *tnext P_(( TNODE *n ));

/*
* Tree navigation macros.  WARNING: n should be a simple expr., no side effects.
*/
#ifndef PARENT
static TNODE *macro_n;
#define CHILD0(n)	(macro_n = (n)->down,	\
	macro_n ? macro_n->over : (TNODE *)NULL)
#define CHILD1(n)	(macro_n = (n)->down,	\
	macro_n ? macro_n->over->over : (TNODE *)NULL)
#define CHILD2(n)	(macro_n = (n)->down,	\
	macro_n ? macro_n->over->over->over : (TNODE *)NULL)
#define CHILD3(n)	(macro_n = (n)->down,	\
	macro_n ? macro_n->over->over->over->over : (TNODE *)NULL)
#define CHILD4(n)	(macro_n = (n)->down,	\
	macro_n ? macro_n->over->over->over->over->over : (TNODE *)NULL)
#define TNEXT(n)	(macro_n = (n),		\
	macro_n->up->down == macro_n ? (TNODE *)NULL : macro_n->over)
#define LASTCHILD(n)	((n)->down)
#define PARENT(n)	((n)->up)
#endif
#endif /* tnode_H */
