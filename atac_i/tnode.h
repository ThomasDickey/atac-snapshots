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
	"$Header: /users/source/archives/atac.vcs/atac_i/RCS/tnode.h,v 3.8 1997/12/08 22:16:46 tom Exp $";
/*
* $Log: tnode.h,v $
* Revision 3.8  1997/12/08 22:16:46  tom
* add prototypes for parse(), deparse()
*
* Revision 3.7  1997/05/11 23:58:01  tom
* add prototypes for sym.c, and include srcpos.h
*
* Revision 3.6  1997/05/11 22:27:02  tom
* add prototypes for type.c
*
* Revision 3.5  1997/05/11 20:26:53  tom
* include sym.h to declare CONST_VALUE
*
* Revision 3.4  1997/05/11 18:31:38  tom
* use prototypes for srcpos.c
*
* Revision 3.3  1997/05/10 20:15:05  tom
* prototype for print_tree
*
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
#include "srcpos.h"
#include "sym.h"

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

/* Pgram.y */
extern int parse P_(( FILE *srcfile, TNODE **tree, char **uprefix ));

/* dparse.c */
extern void deparse P_(( TNODE *n, FILE *f, char *hookname, char *prefix ));

/* srcpos.c */
extern char *srcfname P_(( int findex));
extern int srcfstamp P_(( int findex));
extern int store_filename P_(( char *s));
extern void node_isrcpos P_(( TNODE *node, int left, FILE *f));
extern void node_srcpos P_(( TNODE *node, int left, FILE *f));
extern void print_srcpos P_(( SRCPOS *srcpos, FILE *f));

/* tree.c */
extern TNODE *tFindDef P_(( TNODE *n ));
extern TNODE *tFindPred P_(( TNODE *n ));
extern TNODE *tFindSwitch P_(( TNODE *n ));
extern TNODE *tlist_add P_(( TNODE *list, TNODE *next ));
extern TNODE *tlist_ladd P_(( TNODE *list, TNODE *next ));
extern TNODE *tmkleaf P_(( int genus, int species, SRCPOS *srcpos, char *text ));
extern TNODE *tmknode P_(( int genus, int species, TNODE *child0, TNODE *child1 ));
extern TNODE *tsrc_pos P_(( TNODE *node, SRCPOS *begin, SRCPOS *end ));
extern int print_tree P_(( TNODE *node, int id, int parent, int level ));
extern void tFindVDef P_(( TNODE *n, CONST_VALUE *value ));
extern void tfreenode P_(( TNODE *node ));

/* sym.c */
extern SYM *sym_find P_(( char *name, NAMETYPE nametype, SYMLIST *symtab ));
extern void dump_sym P_(( TNODE *node, char *prefix ));
extern void do_sym P_(( TNODE *node ));

/* tnode.c */
TNODE *child0 P_(( TNODE *n ));
TNODE *child1 P_(( TNODE *n ));
TNODE *child2 P_(( TNODE *n ));
TNODE *child3 P_(( TNODE *n ));
TNODE *child4 P_(( TNODE *n ));
TNODE *tnext P_(( TNODE *n ));

/* type.c */
extern void clear_type P_(( VALTYPE *type ));
extern void copy_type P_(( VALTYPE *from, VALTYPE *to ));
extern void expr_type P_(( TNODE *n, VALTYPE *type ));
extern void int_promote P_(( VALTYPE *type ));

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
