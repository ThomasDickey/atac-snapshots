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
static const char tnode_h[] = "$Id: tnode.h,v 3.9 2013/12/08 22:04:36 tom Exp $";
/*
* @Log: tnode.h,v @
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
    int genus;
    int species;
    SRCPOS srcpos[2];		/* LEFT_SRCPOS, RIGHT_SRCPOS */
    struct tnode *up;
    struct tnode *down;
    struct tnode *over;
    char *text;
    union {
	struct symlist *symtab;
	struct sym *sym;
	struct {
	    short blkno;
	    short tempno;
	    struct valtype *type;
	} hook;
    } sym;
} TNODE;

/* Pgram.y */
extern int parse(FILE *srcfile, TNODE ** tree, char **uprefix);

/* dparse.c */
extern void deparse(TNODE * n, FILE *f, char *hookname, char *prefix);

/* srcpos.c */
extern char *srcfname(int findex);
extern int srcfstamp(int findex);
extern int store_filename(char *s);
extern void node_isrcpos(TNODE * node, int left, FILE *f);
extern void node_srcpos(TNODE * node, int left, FILE *f);
extern void print_srcpos(SRCPOS * srcpos, FILE *f);

/* tree.c */
extern TNODE *tFindDef(TNODE * n);
extern TNODE *tFindPred(TNODE * n);
extern TNODE *tFindSwitch(TNODE * n);
extern TNODE *tlist_add(TNODE * list, TNODE * next);
extern TNODE *tlist_ladd(TNODE * list, TNODE * next);
extern TNODE *tmkleaf(int genus, int species, SRCPOS * srcpos, char *text);
extern TNODE *tmknode(int genus, int species, TNODE * child0, TNODE * child1);
extern TNODE *tsrc_pos(TNODE * node, SRCPOS * begin, SRCPOS * end);
extern int print_tree(TNODE * node, int id, int parent, int level);
extern void tFindVDef(TNODE * n, CONST_VALUE * value);
extern void tfreenode(TNODE * node);

/* sym.c */
extern SYM *sym_find(char *name, NAMETYPE nametype, SYMLIST * symtab);
extern void dump_sym(TNODE * node, char *prefix);
extern void do_sym(TNODE * node);

/* tnode.c */
TNODE *child0(TNODE * n);
TNODE *child1(TNODE * n);
TNODE *child2(TNODE * n);
TNODE *child3(TNODE * n);
TNODE *child4(TNODE * n);
TNODE *tnext(TNODE * n);

/* type.c */
extern void clear_type(VALTYPE * type);
extern void copy_type(VALTYPE * from, VALTYPE * to);
extern void expr_type(TNODE * n, VALTYPE * type);
extern void int_promote(VALTYPE * type);

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
