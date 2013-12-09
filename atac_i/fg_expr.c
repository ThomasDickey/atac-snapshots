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
#pragma csect (CODE, "fg_expr$")
#include <mvapts.h>
MODULEID(%M%,%J%/%D%/%T%)
#include <string.h>
#include <stdlib.h>
#endif /* MVS */

static const char fg_expr_c[] = "$Id: fg_expr.c,v 3.11 2013/12/08 18:12:11 tom Exp $";
/*
* @Log: fg_expr.c,v @
* Revision 3.10  1997/12/09 00:13:03  tom
* moved externs to header file
*
* Revision 3.9  1997/05/11 19:48:01  tom
* split-out flowgraph.h, compile-clean
*
* Revision 3.8  1997/05/11 19:01:45  tom
* make this compile clean against dug.h with all prototypes from dug.c
*
* Revision 3.7  1997/05/10 23:18:24  tom
* absorb srcpos.h into error.h
*
* Revision 3.6  1996/11/12 23:59:34  tom
* change ident to 'const' to quiet gcc
* add forward-ref prototypes
*
* Revision 3.5  1995/12/27 23:25:31  tom
* don't use NULL for int value!
*
* Revision 3.4  94/04/04  10:12:44  jrh
* Add Release Copyright
* 
* Revision 3.3  94/03/21  08:30:54  saul
* MVS support __offsetof as builtin (not handled by cpp)
* 
* Revision 3.2  93/08/04  15:45:05  ewk
* Added MVS and solaris support.  Squelched some ANSI warnings.
* 
* Revision 3.1  93/07/12  10:22:32  saul
* MVS MODULEID
* 
* Revision 3.0  92/11/06  07:45:12  saul
* propagate to version 3.0
* 
* Revision 2.6  92/11/02  11:36:48  saul
* remove unused variables
* 
* Revision 2.5  92/10/30  09:48:12  saul
* include portable.h
* 
* Revision 2.4  92/07/10  11:57:36  saul
* New args for dug_du & dug_branch for new display; enhanced header comment
* 
* Revision 2.3  92/04/07  09:00:18  saul
* remove ATACYSIS #ifdefs
* 
* Revision 2.2  92/03/17  14:22:25  saul
* copyright
* 
* Revision 2.1  91/06/13  12:39:03  saul
* Propagate to version 2.0
* 
 * Revision 1.1  91/06/12  20:25:40  saul
 * Aug 1990 baseline
 * 
*-----------------------------------------------end of log
*/
#include <stdio.h>
#include "portable.h"
#include "error.h"
#include "tnode.h"
#include "tree.h"
#include "sym.h"
#include "flowgraph.h"

/* forward declarations */
static void fg_asgn(TNODE * n, DUG * dug, BLOCK * sblk, BLOCK ** endblk,
		    int var_lhs, int ptr_lhs, int var_rhs, int ptr_rhs);

/*
* fg_expr:  Build on to the data flow graph "dug" for the parse sub-tree
*	rooted at n.   "sblk" is the ID of the flow graph block to begin at.
*
*	Return the last block of the flowgraph in *endblk.  The "last"
*	block is the last evaluated.  If the flow graph does not converge,
*	that is, there are two blocks that may be evaluated last depending
*	on the results of a predicate, as with  "a && b" and "a || b"
*	expressions, if Tendblk and Fendblk are NULL add an extra block to the
*	flowgraph and make it the target of the two last blocks.  The extra
*	block becomes the "last" block.  If the flow graph does not converge,
*	and Tendblk and Fendblk are not NULL the syntacticly later block
*	is considered "last".  In this case,  return  *Tendblk and *Fendblk as
*	the last blocks on the true and false branches respectively.  If
*	Tendblk and Fendblk are not NULL and the flow graph is convergent,
*	return  *Tendblk and *Fendblk as NULL.
*
*	"var_use" is a combination of VAR_DEF, VAR_CUSE, VAR_PUSE, or VAR_VOID,
*	indicating the data context of a variable in expression "n":
*		VAR_DEF -- assigned a value
*		VAR_CUSE -- used in a value computation
*		VAR_PUSE -- used in a predicate evaluation
*		VAR_VOID -- none of the others
*	Combinations may apply.  (For example, in "if (b++)" b is assigned,
*	used in a computation, and used in a predicate evalutation.)
*
*	"ptr_use" has the same possible values as "var_use".  It indicates
*	the data context of the pointer dereferenced variable in "n".
*	For example, in "if (p[i])" var_use is VAR_CUSE and ptr_use is VAR_PUSE
*	indicating that p is used in a computation (of p + i) and something
*	dereferenced off of p (in this case "*(p + i)") is used in a predicate
*	evalutation.
*/
void
fg_expr(TNODE * n,
	DUG * dug,
	BLOCK * sblk,
	BLOCK ** endblk,
	BLOCK ** Tendblk,
	BLOCK ** Fendblk,
	int var_use,
	int ptr_use)
{
    SUBGRAPH e1;		/* expr 1 */
    SUBGRAPH e2;		/* expr 2 */
    SUBGRAPH e3;		/* expr 3 */
    TNODE *p;
    BLOCK *end;
    BLOCK *tmp;

    if (sblk == 0) {
	/* unreachable code */
	*endblk = 0;
	return;
    }

    if (Tendblk)
	*Tendblk = 0;
    if (Fendblk)
	*Fendblk = 0;

    end = sblk;
    dug_startblk(dug, sblk, n);

    if (n->genus != GEN_EXPR) {
	internal_error(n->srcpos,
		       "*******fg_expr: unexpected genus: %d", n->genus);
	*endblk = end;
	return;
    }

    switch (n->species) {
    case EXPR_QCOLON:
	{
	    TNODE *p2;
	    BLOCK *Tend;
	    BLOCK *Fend;

	    end = dug_newblk(dug);
	    fg_expr(CHILD0(n), dug, sblk, &e1.e, &Tend, &Fend,
		    VAR_PUSE, VAR_VOID);
	    dug_du(dug, &decis_sym, e1.e, VAR_PUSE, CHILD0(n));
	    p2 = CHILD1(n);
	    e2.s = dug_newblk(dug);
	    fg_expr(p2, dug, e2.s, &e2.e, NULL, NULL,
		    var_use, ptr_use);
	    p2 = CHILD2(n);
	    e3.s = dug_newblk(dug);
	    fg_expr(p2, dug, e3.s, &e3.e, NULL, NULL,
		    var_use, ptr_use);
	    dug_branch(dug, e1.e, e2.s, COND_BOOLEAN, 1, CHILD0(n));
	    dug_branch(dug, e1.e, e3.s, COND_BOOLEAN, 0, CHILD0(n));
	    dug_branch(dug, Tend, e2.s, COND_UNCONDITIONAL, 0, NULL);
	    dug_branch(dug, Fend, e3.s, COND_UNCONDITIONAL, 0, NULL);
	    dug_branch(dug, e2.e, end, COND_UNCONDITIONAL, 0, NULL);
	    dug_branch(dug, e3.e, end, COND_UNCONDITIONAL, 0, NULL);
	    break;
	}
    case EXPR_COMMA:
	fg_expr(CHILD0(n), dug, sblk, &end, NULL, NULL,
		VAR_VOID, VAR_VOID);
	fg_expr(CHILD1(n), dug, end, &end, Tendblk, Fendblk,
		var_use, ptr_use);
	break;
    case EXPR_LARRAY:
	fg_expr(CHILD0(n), dug, sblk, &end, NULL, NULL,
		VAR_CUSE, var_use | ptr_use);
	fg_expr(CHILD1(n), dug, end, &end, NULL, NULL,
		VAR_CUSE, var_use | ptr_use);
	break;
    case EXPR_BINOP:
	p = CHILD1(n);
	switch (p->species) {
	case BINOP_ASGN:
	    fg_asgn(n, dug, sblk, &end, VAR_DEF, VAR_VOID,
		    var_use | VAR_CUSE, ptr_use);
	    break;
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
	    fg_asgn(n, dug, sblk, &end,
		    VAR_DEF | VAR_CUSE | var_use, ptr_use,
		    var_use | VAR_CUSE, ptr_use);
	    break;
	case BINOP_ANDAND:
	case BINOP_OROR:
	    {
		TNODE *p2;
		BLOCK *start;
		BLOCK *Tstart;
		BLOCK *Fstart;
		BLOCK *Tend;
		BLOCK *Fend;

		end = dug_newblk(dug);
		Tend = dug_newblk(dug);
		Fend = dug_newblk(dug);
		fg_expr(CHILD0(n), dug, sblk, &start, &Tstart,
			&Fstart, var_use | VAR_PUSE, ptr_use);
		dug_du(dug, &decis_sym, start, VAR_PUSE,
		       CHILD0(n));
		e2.s = dug_newblk(dug);
		if (CHILD1(n)->species == BINOP_ANDAND) {
		    dug_branch(dug, Fstart, Fend,
			       COND_UNCONDITIONAL, 0, NULL);
		    dug_branch(dug, Tstart, e2.s,
			       COND_UNCONDITIONAL, 0, NULL);
		    dug_branch(dug, start, Fend,
			       COND_BOOLEAN, 0, CHILD0(n));
		    dug_branch(dug, start, e2.s,
			       COND_BOOLEAN, 1, CHILD0(n));
		} else {
		    dug_branch(dug, Tstart, Tend,
			       COND_UNCONDITIONAL, 0, NULL);
		    dug_branch(dug, Fstart, e2.s,
			       COND_UNCONDITIONAL, 0, NULL);
		    dug_branch(dug, start, Tend,
			       COND_BOOLEAN, 1, CHILD0(n));
		    dug_branch(dug, start, e2.s,
			       COND_BOOLEAN, 0, CHILD0(n));
		}
		p2 = CHILD2(n);
		fg_expr(p2, dug, e2.s, &e2.e, &Tstart, &Fstart,
			var_use, ptr_use);
		dug_branch(dug, Tstart, Tend,
			   COND_UNCONDITIONAL, 0, NULL);
		dug_branch(dug, Fstart, Fend,
			   COND_UNCONDITIONAL, 0, NULL);
		dug_branch(dug, e2.e, end,
			   COND_UNCONDITIONAL, 0, NULL);
		if (Tendblk)
		    *Tendblk = Tend;
		else
		    dug_branch(dug, Tend, end,
			       COND_UNCONDITIONAL, 0, NULL);
		if (Fendblk)
		    *Fendblk = Fend;
		else
		    dug_branch(dug, Fend, end,
			       COND_UNCONDITIONAL, 0, NULL);
		break;
	    }
	default:
	    fg_expr(CHILD0(n), dug, sblk, &end, NULL, NULL,
		    var_use, ptr_use);
	    fg_expr(CHILD2(n), dug, end, &end, NULL, NULL,
		    var_use, ptr_use);
	    break;
	}
	break;
    case EXPR_LSTAR:
	fg_expr(CHILD1(n), dug, sblk, &end, NULL, NULL,
		VAR_CUSE, var_use | ptr_use);
	break;
    case EXPR_UNOP:
	{
	    TNODE *p2;

	    p2 = CHILD0(n);
	    switch (p2->species) {
	    case UNOP_AND:
		/*
		 * What should be the DEF/USE character of &x?
		 * *&x ? **&x ?
		 */
		fg_expr(CHILD1(n), dug, sblk, &end, NULL, NULL,
			VAR_VOID, VAR_VOID);
		break;
	    case UNOP_INC:
	    case UNOP_DEC:
		fg_expr(CHILD1(n), dug, sblk, &end, NULL, NULL,
			var_use | VAR_CUSE | VAR_DEF, ptr_use);
		break;
	    case UNOP_NOT:
		fg_expr(CHILD1(n), dug, sblk, &end, Fendblk,
			Tendblk, var_use, ptr_use);
		break;
	    default:
		fg_expr(CHILD1(n), dug, sblk, &end, NULL, NULL,
			var_use, ptr_use);
		break;
	    }
	    break;
	}
    case EXPR_CAST:
	fg_expr(CHILD1(n), dug, sblk, &end, Tendblk, Fendblk,
		var_use, ptr_use);
	break;
    case EXPR_INCOP:
	/*
	 * Would it be better to call x++ a use but not a definition?
	 */
	fg_expr(CHILD0(n), dug, sblk, &end, NULL, NULL,
		var_use | VAR_CUSE | VAR_DEF, ptr_use);
	break;
    case EXPR_LARROW:
    case EXPR_LDOT:
	/*
	 * Should differentiate uses of the same pointer with
	 * different field names. ?unknown?
	 */
	fg_expr(CHILD0(n), dug, sblk, &end, NULL, NULL,
		VAR_CUSE, var_use | ptr_use);
	break;
    case EXPR_INHERIT:
	fg_expr(CHILD0(n), dug, sblk, &end, Tendblk, Fendblk,
		var_use, ptr_use);
	break;
    case EXPR_LFCALL0:
    case EXPR_LFCALL1:
	/*
	 * The order of evaluation of the function expression and
	 * its arguments is undetermined.  We assume function expression
	 * first followed by arguments in order left to right.
	 */
	p = CHILD0(n);		/* FUNC_LP */
	p = CHILD0(p);		/* FNAME or EXPR */
	if (p->genus == GEN_EXPR) {
	    /*
	     * ?unknown? should ptr_use be VOID.
	     */
	    fg_expr(p, dug, end, &end, NULL, NULL,
		    var_use, ptr_use);
	}
	p = CHILD1(n);		/* EXP_LIST for LFCALL1 */
	if (p->genus == GEN_EXP_LIST) {
	    /*
	     * pass over EXP_LIST to EXPR
	     */
	    for (p = CHILD0(p); p != NULL; p = TNEXT(p)) {
		/*
		 * Should arguments be CUSE only? Are they too
		 * indirectly related to the predicate,
		 * definition, or dereference ?
		 */
		fg_expr(p, dug, end, &end, NULL, NULL,
			var_use | VAR_CUSE, ptr_use);
	    }
	}
	dug_endblk(dug, end, n);
	tmp = end;
	end = dug_newblk(dug);
	dug_branch(dug, tmp, end, COND_UNCONDITIONAL, 0, NULL);
	break;
    case EXPR_SIZEOF:
    case EXPR_SIZEOF_TYPE:
#ifdef MVS
    case EXPR_OFFSET:
#endif /* MVS */
    case EXPR_ICON:
    case EXPR_FCON:
    case EXPR_STRING:
	break;
    case EXPR_LNAME:
	/*
	 * Record the type of reference to this variable.
	 */
	{
	    SYM *sym;
	    unsigned long sym_type;
	    TNODE *p2;

	    /*
	     * If both references are VOID return.  (E.g. stmt: x;)
	     */
	    if ((var_use | ptr_use) == VAR_VOID)
		break;
	    p2 = CHILD0(n);
	    sym = p2->sym.sym;
	    if (sym == NULL)
		internal_error(n->srcpos,
			       "fg_expr missing symbol info for: %s",
			       p2->text ? p2->text : "?");
	    if (sym->nametype != VALSYM)	/* e.g. mem of enum */
		break;		/* not a variable */
	    /*
	     * Record direct reference to variables.
	     * (References to arrays and functions are ignored.)
	     * ?unknown? what about passing array name as argument?
	     */
	    sym_type = sym->type.valtype.qual;
	    if (var_use != VAR_VOID) {
		if (!QUAL_ISARRAY(sym_type) &&
		    !QUAL_ISFUNC(sym_type)) {
		    dug_du(dug, sym, sblk, var_use, p2);
		}
	    }

	    /*
	     * Record dereferences of pointers or arrays.
	     * Dereferences of other variables are direct
	     * references (e.g. integer in: *(array + integer) ).
	     */
	    if (ptr_use != VAR_VOID) {
		if (QUAL_ISPTR(sym_type) ||
		    QUAL_ISARRAY(sym_type)) {
		    dug_du(dug, sym, sblk,
			   VAR_DREF | ptr_use, p2);
		} else
		    dug_du(dug, sym, sblk, ptr_use, p2);
	    }
	    break;
	}
    default:
	internal_error(n->srcpos,
		       "*******fg_expr: unknown species: %d", n->species);
	break;
    }

    if (end == sblk)
	dug_endblk(dug, end, n);
    *endblk = end;
    return;
}

static void
fg_asgn(TNODE * n,
	DUG * dug,
	BLOCK * sblk,
	BLOCK ** endblk,
	int var_lhs,
	int ptr_lhs,
	int var_rhs,
	int ptr_rhs)
{
    SUBGRAPH lhs;		/* left hand side */
    SUBGRAPH rhs;		/* right hand side */
    BLOCK *end;

    /*
       * RHS of assignment is analysed first because it
       * will be evaluated first.
     */
    rhs.s = dug_newblk(dug);
    fg_expr(CHILD2(n), dug, rhs.s, &rhs.e, NULL, NULL,
	    var_rhs, ptr_rhs);
    lhs.s = dug_newblk(dug);
    fg_expr(CHILD0(n), dug, lhs.s, &lhs.e, NULL, NULL,
	    var_lhs, ptr_lhs);
    /*
     * If no new blocks begin in RHS or LHS of assignment combine them.
     * First LHS is combined into RHS to keep def/use info in order.
     * Parse_start of the combined block is forced to be the whole
     * assignment in case sblk has no start yet.  Then the combined block
     * is combined into sblk.  (Parse_end doesn't matter because it will
     * be set to the whole assignment when it is all one block.
     */
    if ((rhs.e == rhs.s) && (lhs.e == lhs.s)) {
	dug_du_combine(dug, rhs.s, lhs.s);
	dug_startblk(dug, rhs.s, NULL);
	dug_startblk(dug, rhs.s, n);
	dug_du_combine(dug, sblk, rhs.s);
	end = sblk;
    } else {
	end = dug_newblk(dug);
	dug_branch(dug, sblk, rhs.s, COND_UNCONDITIONAL, 0, NULL);
	dug_branch(dug, rhs.e, lhs.s, COND_UNCONDITIONAL, 0, NULL);
	dug_branch(dug, lhs.e, end, COND_UNCONDITIONAL, 0, NULL);
    }
    *endblk = end;
    return;
}
