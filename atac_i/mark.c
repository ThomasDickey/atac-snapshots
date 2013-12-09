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
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef MVS
#pragma csect (CODE, "mark$")
#include <mvapts.h>
MODULEID(%M%,%J%/%D%/%T%)
#include <string.h>
#include <stdlib.h>
#endif /* MVS */

static const char mark_c[] = "$Id: mark.c,v 3.11 2013/12/08 18:32:45 tom Exp $";
/*
* @Log: mark.c,v @
* Revision 3.10  2005/08/13 16:16:41  tom
* gcc warnings
*
* Revision 3.9  1997/12/09 00:53:23  tom
* cast dug->fname to cover up special marker() case
*
* Revision 3.8  1997/05/11 23:27:56  tom
* correct gcc warnings
*
* Revision 3.7  1997/05/10 23:19:23  tom
* absorb srcpos.h into error.h
*
* Revision 3.6  1996/11/13 00:42:22  tom
* change ident to 'const' to quiet gcc
* add forward-ref prototypes
*
* Revision 3.5  94/04/04  10:13:25  jrh
* Add Release Copyright
* 
* Revision 3.4  94/03/21  08:32:05  saul
* MVS support __offsetof as builtin (not handled by cpp)
* 
* Revision 3.3  93/08/09  12:25:54  saul
* comments added
* 
* Revision 3.2  1993/08/04  15:46:28  ewk
* Added MVS and solaris support.  Squelched some ANSI warnings.
*
* Revision 3.1  93/07/12  10:54:30  saul
* MVS MODULEID
* 
* Revision 3.0  92/11/06  07:45:23  saul
* propagate to version 3.0
* 
* Revision 2.4  92/11/02  11:37:50  saul
* remove unused variables
* 
* Revision 2.3  92/10/30  09:48:26  saul
* include portable.h
* 
* Revision 2.2  92/03/17  14:22:37  saul
* copyright
* 
* Revision 2.1  91/06/13  12:39:10  saul
* Propagate to version 2.0
* 
* Revision 1.1  91/06/12  20:25:46  saul
* Aug 1990 baseline
* 
*-----------------------------------------------end of log
*/
#if HAVE_STDLIB_H
#include <stdlib.h>
#endif
#include <stdio.h>
#include "portable.h"
#include "error.h"
#include "tnode.h"
#include "tree.h"
#include "sym.h"		/* for type info in cast stuff */
#include "dug.h"
#include "hook.h"

/* forward declarations */
static void marker(TNODE * node, int mtype, int blkno, VALTYPE * type, int tempno);
static int tempvar(TNODE * local_pos, int stmt_no, TNODE * n);
static void expr_marker(DUG * dug, BLOCK * b, TNODE * end, TNODE *
			local_pos, int *stmt_no);
static VALTYPE *cast_type(TNODE * n);
static void stmt_marker(TNODE * n, int blk, int mark_after);

#define CHECK_MALLOC(p) ((p) ? 1 : internal_error(NULL, "Out of memory"))

/*
* stmt_marker: Insert a HOOK for "blk" of the appropriate type at "node".
*	Normally the HOOK semantically preceedes the "end" of "node".
*	If mark_after, the HOOK must semantically follow the end of "node".
*/
static void
stmt_marker(TNODE * n,
	    int blk,
	    int mark_after)
{
    TNODE *p;
    TNODE *pp;

    /*
     * Marking INDATA_DCLS ?
     */
    if (n->genus == GEN_INDATA_DCLS) {	/* e.g. {int x; H; while ... */
	marker(n, HOOK_STMT_R, blk, NULL, 0);
	return;
    }

    /*
     * Marking a STMT in a STMT_LIST ?
     */
    p = PARENT(n);
    while (p->genus == GEN_HOOK ||
	   (p->genus == GEN_STMT &&
	    (p->species == STMT_LABEL ||
	     p->species == STMT_CASE ||
	     p->species == STMT_DEFAULT))) {
	p = PARENT(p);
    }
    if (p->genus == GEN_STMT_LIST) {
	if (mark_after)		/* e.g. int p; H; p = 0 || 1; */
	    marker(n, HOOK_STMT_R, blk, NULL, 0);
	else			/* e.g. H; 0; while ... */
	    marker(n, HOOK_STMT_L, blk, NULL, 0);
	return;
    }

    /*
     * Marking a STMT that is a COMPSTMT.
     */
    p = LASTCHILD(n);
    if (p != NULL && p->genus == GEN_COMPSTMT) {
	pp = CHILD0(p);
	if (pp && pp->genus != GEN_INDATA_DCLS) {
	    if (mark_after) {
		/* Can't happen.  mark_after is true only when
		 *  n->species == STMT_EXPR so LASTCHILD(n)->genus
		 *  can't be GEN_COMPSTMT.
		 */
		marker(pp, HOOK_STMT_R, blk, NULL, 0);
	    } else		/* e.g. if (e) { H; 2; } */
		marker(pp, HOOK_STMT_L, blk, NULL, 0);
	    return;
	}
    }

    /*
     * Others:
     */
    if (mark_after)
	marker(n, HOOK_STMT_R_B, blk, NULL, 0);
    else
	marker(n, HOOK_STMT_L_B, blk, NULL, 0);
}

/*
* cast_type:  If node n is an ICON of value 0, it's HOOK may need a
*	type cast to permit comparison with or assignment to a pointer
*	(or to be parallel with a pointer in a ?: expression).  (Compilers
*	allow direct comparison of pointers with 0, but may not allow it
*	when a HOOK is inserted; e.g. "p = (HOOK(...), NULL);".)  This
*	routine returns the type of the cast needed.  If none is needed,
*	0 is returned.
*/
static VALTYPE *
cast_type(TNODE * n)
{
    TNODE *p;
    VALTYPE type;
    VALTYPE *type_ptr;

    if (n->genus != GEN_EXPR)
	internal_error(n->srcpos, "cast_type: unexpected genus %d",
		       n->genus);
    if (n->species != EXPR_ICON)
	return 0;
    if (LASTCHILD(n) == NULL)
	internal_error(n->srcpos, "cast_type: EXPR missing child");
    if (evalIConstExpr(n) != 0)
	return 0;
    p = PARENT(n);
    while (p->genus == GEN_HOOK)
	p = PARENT(p);
    if (p->genus == GEN_EXPR) {
	if (p->species == EXPR_BINOP) {
	    switch (CHILD1(p)->species) {
	    case BINOP_ASGN:
		p = CHILD0(p);
		break;
	    case BINOP_EQ:
	    case BINOP_GTEQ:
	    case BINOP_LTEQ:
	    case BINOP_NEQ:
	    case BINOP_LT:
	    case BINOP_GT:
		if (n == CHILD2(p))
		    p = CHILD0(p);
		else
		    p = CHILD2(p);
		break;
	    default:
		return 0;
	    }
	} else if (p->species == EXPR_QCOLON) {
	    if (n == CHILD2(p))
		p = CHILD1(p);
	    else if (n == CHILD1(p))
		p = CHILD2(p);
	    else
		return 0;
	} else
	    return 0;
    } else
	return 0;

    expr_type(p, &type);
    if ((type.base & BTB_INT) && (type.qual == 0))
	return 0;

    type_ptr = (VALTYPE *) malloc(sizeof *type_ptr);
    CHECK_MALLOC(type_ptr);
    copy_type(&type, type_ptr);
    return type_ptr;
}

static void
expr_marker(DUG * dug,
	    BLOCK * b,
	    TNODE * end,
	    TNODE * local_pos,
	    int *stmt_no)
{
    int mark_after;
    TNODE *p;
    VALTYPE *type;
    int more;

    mark_after = 0;

    /*
     * Try to push the node that marks the end of the block up the
     * syntax tree.  As long as the end node is the first part of it's
     * parent to be evaluated, it can be pushed up.  (The mark will
     * be evaluated before the node that is marked.)
     */
    p = end;
    do {
	more = 0;
	end = p;
	p = PARENT(end);
	if (p == NULL)
	    internal_error(end->srcpos, "missing parent");
	switch (p->genus) {
	case GEN_HOOK:
	    /*
	     * cast_type() gets confused if we pass it a GEN_HOOK.
	     */
	    break;;
	case GEN_EXPR:
	    switch (p->species) {
	    case EXPR_UNOP:
	    case EXPR_INCOP:
	    case EXPR_LSTAR:
	    case EXPR_LARROW:
	    case EXPR_LDOT:
	    case EXPR_LFCALL0:
	    case EXPR_CAST:
	    case EXPR_SIZEOF:
#ifdef MVS
	    case EXPR_OFFSET:
#endif
	    case EXPR_INHERIT:
		more = 1;
		continue;
	    case EXPR_QCOLON:
	    case EXPR_COMMA:
	    case EXPR_LARRAY:
		if (CHILD0(p) == end)
		    more = 1;
		continue;
	    case EXPR_BINOP:
		switch (CHILD1(p)->species) {
		case BINOP_ASGN:
		case BINOP_APLUS:
		case BINOP_AMINUS:
		case BINOP_AMUL:
		case BINOP_ADIV:
		case BINOP_AMOD:
		case BINOP_ALS:
		case BINOP_ARS:
		case BINOP_AAND:
		case BINOP_AOR:
		case BINOP_AER:
		    if (LASTCHILD(p) == end)
			more = 1;
		    else {
			/*
			 * Marking after so
			 * don't push up any
			 * more.
			 */
			end = p;
			mark_after = 1;
		    }
		    break;
		default:
		    if (CHILD0(p) == end)
			more = 1;
		    break;
		}
		continue;
	    case EXPR_LFCALL1:
		/*
		 * If the expression is part of the FUNC_LP,
		 * or the FUNC_LP is a FUNC_NAME_LP and the
		 * expression is part of the EXP_LIST, okay to
		 * continue.  Otherwise, go back down to the
		 * first EXPR in the EXP_LIST.
		 */
		if (end->genus == GEN_FUNC_LP)
		    more = 1;
		else if (CHILD0(p)->species == FUNC_NAME_LP)
		    more = 1;
		else
		    end = CHILD0(end);
		continue;
	    default:
		break;
	    }
	    continue;
	case GEN_EXP_LIST:
	    if (CHILD0(p) == end)
		more = 1;
	    continue;
	case GEN_FUNC_LP:
	    more = 1;
	    continue;
	}
    } while (more);

    p = PARENT(end);
    if (p->genus == GEN_STMT && p->species == STMT_EXPR) {
	/*
	 * STMT marker is more reliable than EXPR marker.
	 */
	stmt_marker(p, b->block_id, mark_after);
	++*stmt_no;
	return;
    }

    if (mark_after) {
	marker(end, HOOK_EXPR_R, b->block_id, NULL,
	       tempvar(local_pos, *stmt_no, end));
    } else {
	type = cast_type(end);
	if (type)
	    marker(end, HOOK_EXPR_CAST, b->block_id, type, 0);
	else
	    marker(end, HOOK_EXPR_L, b->block_id, NULL, 0);
    }
}

/*
* dug_mark: Insert HOOK node into parse tree at semantic end of each node
*	in "dug".
*/
void
dug_mark(dug)
     DUG *dug;
{
    LIST *t;
    BLOCK *b;
    TNODE *start;
    TNODE *end;
    TNODE *local_pos;
    int stmt_no;		/* Controls reuse of temp vars */

    t = NULL;
    if (LIST_NEXT(dug->block_list, &t, &b) == 0) {
	internal_error(NULL, "dug_mark: no start block");
	return;
    }
    start = b->parse_start;
    local_pos = start;
    marker(start, HOOK_START, 0, (VALTYPE *) (dug->fname), 0);
    stmt_no = 0;

    while (LIST_NEXT(dug->block_list, &t, &b)) {
	start = (TNODE *) b->parse_start;
	end = (TNODE *) b->parse_end;
	if (start == NULL)
	    continue;
	if (end == NULL)
	    continue;

	switch (end->genus) {
	case GEN_EXPR:
	    expr_marker(dug, b, end, local_pos, &stmt_no);
	    break;
	case GEN_STMT:
	case GEN_COMPSTMT:
	case GEN_INDATA_DCLS:
	    stmt_marker(end, b->block_id, 0);
	    ++stmt_no;
	    break;
	default:
	    internal_error(end->srcpos,
			   "marker: unexpected genus %d", end->genus);
	}
    }
}

#define N_REUSE_TEMP 15

static int
tempvar(local_pos, stmt_no, n)
     TNODE *local_pos;
     int stmt_no;
     TNODE *n;
{
    struct reuse {
	VALTYPE *type;
	int tempno;
	int stmt_no;
    };
    static struct reuse reuse[N_REUSE_TEMP];
    static int n_reuse = 0;
    static TNODE *prev_pos = NULL;
    static int lasttmp;
    struct reuse temp;
    VALTYPE type;
    VALTYPE *r;
    int i;

    if (local_pos != prev_pos) {
	lasttmp = 0;
	n_reuse = 0;
	prev_pos = local_pos;
    }

    expr_type(n, &type);

    /*
     * Find a temp of the same type that was not yet used in this stmt; 
     * Reuse it.
     */
    for (i = 0; i < n_reuse; ++i) {
	if (reuse[i].stmt_no == stmt_no)
	    continue;
	r = reuse[i].type;
	if (r->base != type.base)
	    continue;
	if (r->qual != type.qual)
	    continue;
	if (r->tag != type.tag)
	    continue;
	if (r->dimensions != type.dimensions)
	    continue;
	if (r->param_list != type.param_list)
	    continue;
	break;
    }

    if (i == n_reuse) {
	/*
	 * Temp not found, create one.
	 */
	if (n_reuse == N_REUSE_TEMP) {
	    /*
	     * No more room. Drop least recently used.
	     */
	    for (i = 0; i < n_reuse - 1; ++i) {
		reuse[i].type = reuse[i + 1].type;
		reuse[i].stmt_no = reuse[i + 1].stmt_no;
		reuse[i].tempno = reuse[i + 1].tempno;
	    }
	} else
	    ++n_reuse;
	r = (VALTYPE *) malloc(sizeof *r);
	CHECK_MALLOC(r);
	copy_type(&type, r);
	marker(local_pos, HOOK_TEMP, 0, r, ++lasttmp);
	reuse[i].type = r;
	reuse[i].tempno = lasttmp;;
    } else {
	/*
	 * Temp found.  Move it to the most recently used position.
	 */
	temp.type = reuse[i].type;
	temp.tempno = reuse[i].tempno;
	for (i = 0; i < n_reuse - 1; ++i) {
	    reuse[i].type = reuse[i + 1].type;
	    reuse[i].stmt_no = reuse[i + 1].stmt_no;
	    reuse[i].tempno = reuse[i + 1].tempno;
	}
	reuse[i].type = temp.type;
	reuse[i].tempno = temp.tempno;
    }
    reuse[i].stmt_no = stmt_no;

    return reuse[i].tempno;
}

/*
* marker:  Insert a parse tree "HOOK" of type mtype with fields blkno,
*	type, tempno, in place of node.  Put "node" under HOOK.
*	(HOOK has no src position.)
*/
static void
marker(node, mtype, blkno, type, tempno)
     TNODE *node;
     int mtype;
     int blkno;
     VALTYPE *type;
     int tempno;
{
    TNODE *hook;
    TNODE *p;

    hook = (TNODE *) malloc(sizeof *hook);
    CHECK_MALLOC(hook);

    hook->genus = GEN_HOOK;
    hook->species = mtype;
    hook->down = node;
    hook->up = node->up;

    if (node->up->down == node)
	node->up->down = hook;
    node->up = hook;

    for (p = node; p->over != node; p = p->over) ;
    if (p == node) {
	hook->over = hook;
    } else {
	hook->over = node->over;
	p->over = hook;
	node->over = node;
    }

    hook->sym.symtab = NULL;
    hook->sym.sym = NULL;
    hook->srcpos[LEFT_SRCPOS].file = -1;
    hook->srcpos[LEFT_SRCPOS].line = 0;
    hook->srcpos[LEFT_SRCPOS].col = 0;
    hook->srcpos[RIGHT_SRCPOS].file = -1;
    hook->srcpos[RIGHT_SRCPOS].line = 0;
    hook->srcpos[RIGHT_SRCPOS].col = 0;
    hook->text = NULL;

    hook->sym.hook.blkno = blkno;
    hook->sym.hook.type = type;
    hook->sym.hook.tempno = tempno;
}
