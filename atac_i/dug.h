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
#ifndef dug_H
#define dug_H
static const char dug_h[] = "$Id: dug.h,v 3.9 2013/12/08 22:07:57 tom Exp $";
/*
* @Log: dug.h,v @
* Revision 3.8  1997/12/08 23:45:36  tom
* add macros ID_SYM and VAR_ID
*
* Revision 3.7  1997/05/11 23:49:33  tom
* add prototypes for mark.c, paths.c, alldu.c
*
* Revision 3.6  1997/05/11 21:25:52  tom
* correct prototypes for du_use, du_use_type
*
* Revision 3.5  1997/05/11 19:01:29  tom
* move prototypes for dug.c here (first cut)
*
* Revision 3.4  1997/05/11 16:47:18  tom
* include list.h, to define LIST-type rather than dummy
*
* Revision 3.3  1996/11/13 00:28:32  tom
* change ident to 'const' to quiet gcc
*
* Revision 3.2  1995/12/27 23:23:54  tom
* don't use NULL for int value!
*
* Revision 3.1  94/04/04  10:12:31  jrh
* Add Release Copyright
* 
* Revision 3.0  92/11/06  07:45:59  saul
* propagate to version 3.0
* 
* Revision 2.5  92/10/28  08:55:49  saul
* enum's removed for portability
* 
* Revision 2.4  92/07/15  10:30:16  saul
* parse_pos bug
* 
* Revision 2.3  92/07/10  13:57:24  saul
* new COND_TYPE; new BRANCH structure; DU moved here
* 
* Revision 2.2  92/03/17  14:22:23  saul
* copyright
* 
* Revision 2.1  91/06/13  12:39:02  saul
* Propagate to version 2.0
* 
* Revision 1.1  91/06/12  20:25:38  saul
* Aug 1990 baseline
* 
*-----------------------------------------------end of log
*/
#include "tnode.h"
#include "list.h"

typedef struct block {
    int magic;
    int block_id;
    LIST *branches;
    int to_count;
    LIST *du_list;
    void *parse_start;
    void *parse_end;
    struct block *visited;
} BLOCK;

typedef struct {
    int magic;
    char *fname;
    int count;
    LIST *block_list;
    int nvar;
    struct varsym *vartab;
} DUG;

typedef struct {
    SYM *symbol;
    int var_id;			/* available after var_clean() */
    int ref_type;
    struct tnode *usePos;
    struct tnode *defPos;
} DU;

/* convert between DU.var_id and SYM */
#define ID_SYM(v) ((SYM *)((v) + 1))
#define VAR_ID(s) ((int)(s) - 1)

typedef int COND_TYPE;
#define COND_UNCONDITIONAL  ((COND_TYPE) 0)
#define COND_BOOLEAN	    ((COND_TYPE) 1)	/* e.g. if (a == b) ... */
#define COND_CHAR	    ((COND_TYPE) 2)	/* e.g. if (c) ... */
#define COND_INT	    ((COND_TYPE) 3)	/* e.g. if (i) ... */
#define COND_PTR	    ((COND_TYPE) 4)	/* e.g. if (p) ... */
#define COND_ENUM	    ((COND_TYPE) 5)	/* e.g. if (e) ... */
#define COND_SWITCH	    ((COND_TYPE) 6)	/* switch (i) ... case ... */
#define COND_SWITCH_DEFAULT ((COND_TYPE) 7)	/* switch (i) ... default:... */

/*
* For COND_SWITCH_DEFAULT, BRANCH.value is 0.  For COND_SWITCH,
* BRANCH.value is the constant value for the concerned "case" and "node"
* points to the GEN_EXPR for the "case".  For others, BRANCH.value is 0
* for "false" and 1 for "true", and "node" is the GEN_EXPR for the
* conditional.
*/
typedef struct branch {
    int magic;
    BLOCK *to;
    COND_TYPE condType;
    long value;
    void *node;
} BRANCH;

#define VAR_VOID	0	/* expression value is not used */
#define VAR_DEF		1	/* expression is an lvalue */
#define VAR_CUSE	2	/* expression is part of a computation */
#define VAR_PUSE	4	/* expression participates in branch decision */
#define VAR_DREF	8	/* expression is dereferenced as a pointer */

#define NULL_BLK	0

/* alldu.c */
void alldu(DUG * dug);

/* dug.c */
extern BLOCK *dug_newblk(DUG * dug);
extern DU *du_use(DUG * dug, BLOCK * node, LIST ** n);
extern DU *du_use_type(DUG * dug, BLOCK * node, SYM * symbol, int mode);
extern DUG *dug_create(void);
extern int dug_blocks(DUG * dug, FILE *f);
extern int dug_branch(DUG * dug, BLOCK * from, BLOCK * bTo, COND_TYPE condType, long value, void *node);
extern int dug_cyclomatic(DUG * dug);
extern int dug_du(DUG * dug, SYM * symbol, BLOCK * block, int ref_type, struct tnode *parse_pos);
extern int dug_endblk(DUG * dug, BLOCK * blk, struct tnode *parse_end);
extern int dug_fname(DUG * dug, char *fname);
extern int dug_free(DUG * dug);
extern int dug_startblk(DUG * dug, BLOCK * blk, struct tnode *parse_start);
extern void dug_clean(DUG * dug);
extern void dug_du_combine(DUG * dug, BLOCK * first, BLOCK * second);
extern void dug_dump(DUG * dug);
extern void dug_tables(DUG * dug, int funcno, char *prefix, FILE *f);
extern void dug_var_table(DUG * dug, FILE *f);

/* mark.c */
extern void dug_mark(DUG * dug);

/* paths.c */
extern void paths(DUG * dug, FILE *f, int feasableOnly);

#endif /* dug_H */
