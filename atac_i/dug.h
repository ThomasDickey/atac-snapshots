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
static const char dug_h[] = 
	"$Header: /users/source/archives/atac.vcs/atac_i/RCS/dug.h,v 3.3 1996/11/13 00:28:32 tom Exp $";
/*
* $Log: dug.h,v $
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
typedef char LIST;

typedef struct block {
	int	magic;
	int	block_id;
	LIST	*branches;
	int	to_count;
	LIST	*du_list;
	void*	parse_start;
	void*	parse_end;
	struct block	*visited;
} BLOCK;
	
typedef struct {
	int	magic;
	char	*fname;
	int	count;
	LIST	*block_list;
	int	nvar;
	struct varsym	*vartab;
} DUG;

typedef struct {
	SYM	*symbol;
	int	var_id;		/* available after var_clean() */
	int	ref_type;
	int	usePos;
	int	defPos;
} DU;

typedef int COND_TYPE;
#define COND_UNCONDITIONAL  ((COND_TYPE) 0)
#define COND_BOOLEAN	    ((COND_TYPE) 1)	/* e.g. if (a == b) ... */
#define COND_CHAR	    ((COND_TYPE) 2)	/* e.g. if (c) ... */
#define COND_INT	    ((COND_TYPE) 3)	/* e.g. if (i) ... */
#define COND_PTR	    ((COND_TYPE) 4)	/* e.g. if (p) ... */
#define COND_ENUM	    ((COND_TYPE) 5)	/* e.g. if (e) ... */
#define COND_SWITCH	    ((COND_TYPE) 6)	/* switch (i) ... case ... */
#define COND_SWITCH_DEFAULT ((COND_TYPE) 7)	/* switch (i) ... default:...*/

/*
* For COND_SWITCH_DEFAULT, BRANCH.value is 0.  For COND_SWITCH,
* BRANCH.value is the constant value for the concerned "case" and "node"
* points to the GEN_EXPR for the "case".  For others, BRANCH.value is 0
* for "false" and 1 for "true", and "node" is the GEN_EXPR for the
* conditional.
*/
typedef struct {
    	int		magic;
	BLOCK		*to;
	COND_TYPE	condType;
	long		value;
	void		*node;
} BRANCH;

#define VAR_VOID	0	/* expression value is not used */
#define VAR_DEF		1	/* expression is an lvalue */
#define VAR_CUSE	2	/* expression is part of a computation */
#define VAR_PUSE	4	/* expression participates in branch decision*/
#define VAR_DREF	8	/* expression is dereferenced as a pointer */

#define NULL_BLK	0

#endif /* dug_H */
