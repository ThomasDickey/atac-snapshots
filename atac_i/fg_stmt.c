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
#include <mvapts.h>
MODULEID(%M%,%J%/%D%/%T%)
#endif /* MVS */

static const char fg_stmt_c[] = 
	"$Header: /users/source/archives/atac.vcs/atac_i/RCS/fg_stmt.c,v 3.7 1997/05/11 19:57:16 tom Exp $";
/*
* $Log: fg_stmt.c,v $
* Revision 3.7  1997/05/11 19:57:16  tom
* split-out flowgraph.h, compile-clean
*
* Revision 3.6  1997/05/10 23:14:27  tom
* absorb srcpos.h into error.h
*
* Revision 3.5  1996/11/12 23:54:02  tom
* change ident to 'const' to quiet gcc
* add forward-ref prototypes
*
* Revision 3.4  1995/12/27 23:24:56  tom
* don't use NULL for int value!
*
* Revision 3.3  94/04/04  10:12:58  jrh
* Add Release Copyright
* 
* Revision 3.2  93/08/04  15:45:40  ewk
* Added MVS and solaris support.  Squelched some ANSI warnings.
* 
* Revision 3.1  93/07/12  10:23:53  saul
* MVS MODULEID
* MVS make fg_stmt_goto static to avoid 7 char unique
* 
* Revision 3.0  92/11/06  07:45:16  saul
* propagate to version 3.0
* 
* Revision 2.5  92/10/30  09:48:17  saul
* include portable.h
* 
* Revision 2.4  92/07/10  11:56:26  saul
* New args for dug_du & dug_branch for new display
* 
* Revision 2.3  92/04/07  09:00:22  saul
* remove ATACYSIS #ifdefs
* 
* Revision 2.2  92/03/17  14:22:28  saul
* copyright
* 
* Revision 2.1  91/06/13  12:39:06  saul
* Propagate to version 2.0
* 
 * Revision 1.1  91/06/12  20:25:42  saul
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
#include "sym.h"
#include "dug.h"
#include "flowgraph.h"

#define CHECK_MALLOC(p) ((p)?1:internal_error(NULL, "Out of memory\n"))

extern SYM decis_sym;	/* from fg_module.c */

/* forward declarations */
static void fg_for P_(( DUG *dug, TNODE *expr1, TNODE *expr2, TNODE *expr3, TNODE *stmt, BLOCK *sblk, BLOCK *swblk, BLOCK **endblk, BLOCK **dblk ));
static void fg_stmt_goto P_(( DUG *dug, GOTOLIST **gotolist, BLOCK * sblk ));
static void fg_stmt_label P_(( DUG *dug, GOTOLIST **gotolist, BLOCK * sblk ));

void
fg_stmt(arg_n, dug, arg_sblk, bblk, cblk, swblk, endblk, dblk)
TNODE	*arg_n;
DUG	*dug;
BLOCK *	arg_sblk;	/* block preceeding or containing stmt */
BLOCK *	bblk;		/* block reached by break stmt */
BLOCK *	cblk;		/* block reached by continue stmt */
BLOCK *	swblk;		/* block containing switch to case label */
BLOCK *	*endblk;	/* return block at end of stmt */
BLOCK *	*dblk;		/* return block with "default:" label */
{
	BLOCK *		sblk;
	BLOCK *		end;
	TNODE *		n;
	int		more;
	CONST_VALUE	value;

	sblk = arg_sblk;
	n = arg_n;

	/* Note: sblk may be NULL; e.g. code reached only by GOTO or SWITCH */

	end = sblk;

	if (n->genus != GEN_STMT) {
		internal_error(n->srcpos,
			"*******fg_stmt: unexpected genus: %d", n->genus);
		*endblk = end;
		return;
	}

	/*
	* First strip off all kinds of labels.
	*/
	switch (n->species)
	{
	case STMT_LABEL:
	case STMT_CASE:
	case STMT_DEFAULT:
		end = dug_newblk(dug);
		dug_branch(dug, sblk, end, COND_UNCONDITIONAL, 0, NULL);
		more = 1;
		while(more) {
			switch (n->species)
			{
			case STMT_LABEL:
				if (CHILD0(n)->sym.sym)
					fg_stmt_label(dug,
						(GOTOLIST **)
						    &CHILD0(n)->sym.sym->type.label,
						end);
				n = CHILD1(n);
				break;
			case STMT_CASE:
				evalConstExpr(CHILD0(n), &value);
				if (value.type != CONST_VT) 
				    semantic_error(n->srcpos,
					"case value not constant expression");
				dug_branch(dug, swblk, end, COND_SWITCH,
					   value.fraction, CHILD0(n));
				n = CHILD1(n);
				break;
			case STMT_DEFAULT:
				if (dblk == NULL) semantic_error(n->srcpos,
					"default not in switch.");
				else *dblk = end;
				dug_branch(dug, swblk, end,
					   COND_SWITCH_DEFAULT, 0, NULL);
				n = CHILD0(n);
				break;
			default:
				more = 0;
			}
		}
		sblk = end;
	}


	/*
	* Statements.
	*/

	switch (n->species)
	{
	case STMT_EMPTY:
		/* A block may consist of an empty statement. */
		dug_startblk(dug, sblk, n);
		dug_endblk(dug, end, n);
		break;
	case STMT_COMPSTMT:
		/* startblk() called at GEN_COMPSTMT */
		fg_body(CHILD0(n), dug, sblk, bblk, cblk, swblk, &end, dblk);
		if (end == sblk) dug_endblk(dug, end, n);
		break;
	case STMT_EXPR:
		dug_startblk(dug, sblk, n);
		fg_expr(CHILD0(n), dug, sblk, &end, NULL, NULL,
			VAR_VOID, VAR_VOID);
		if (end == sblk) dug_endblk(dug, end, n);
		break;
	case STMT_IF_ELSE:
	case STMT_IF:
		{
			TNODE		*p;
			TNODE		*eNode;
			SUBGRAPH	e1;	/* expr 1 */
			SUBGRAPH	s1;	/* stmt 1 */
			SUBGRAPH	s2;	/* stmt 2 */
			BLOCK *		Tsblk;
			BLOCK *		Fsblk;

			dug_startblk(dug, sblk, n);
			eNode = CHILD0(n);
			fg_expr(eNode, dug, sblk, &e1.e, &Tsblk, &Fsblk,
				VAR_PUSE, VAR_VOID);
			dug_du(dug, &decis_sym, e1.e, VAR_PUSE, eNode);
			p = CHILD1(n);
			s1.s = dug_newblk(dug);
			fg_stmt(p, dug, s1.s, bblk, cblk, swblk, &s1.e, dblk);
			dug_branch(dug, e1.e, s1.s, COND_BOOLEAN, 1, eNode);
			dug_branch(dug, Tsblk, s1.s, COND_UNCONDITIONAL, 0,
				   NULL);
			end = dug_newblk(dug);
			dug_branch(dug, s1.e, end, COND_UNCONDITIONAL,0,NULL);
			if (n->species == STMT_IF) {
				dug_branch(dug, e1.e, end, COND_BOOLEAN, 0,
					   eNode);
				dug_branch(dug, Fsblk, end, COND_UNCONDITIONAL,
					   0, NULL);
			} else {
				p = CHILD2(n);
				s2.s = dug_newblk(dug);
				fg_stmt(p, dug, s2.s, bblk, cblk, swblk, &s2.e,
					dblk);
				dug_branch(dug, e1.e, s2.s, COND_BOOLEAN, 0,
					   eNode);
				dug_branch(dug, Fsblk, s2.s, COND_UNCONDITIONAL,
					   0, NULL);
				dug_branch(dug, s2.e, end, COND_UNCONDITIONAL,
					   0, NULL);
			}
			break;
		}
	case STMT_WHILE:
		{
			TNODE		*p;
			SUBGRAPH	e1;	/* expr 1 */
			SUBGRAPH	s1;	/* stmt 1 */
			BLOCK *		Tend;
			BLOCK *		Fend;

			p = CHILD0(n);
			e1.s = dug_newblk(dug);
			dug_branch(dug, sblk, e1.s, COND_UNCONDITIONAL,0,NULL);
			dug_startblk(dug, e1.s, n);
			fg_expr(p, dug, e1.s, &e1.e, &Tend, &Fend,
				VAR_PUSE, VAR_VOID);
			dug_du(dug, &decis_sym, e1.e, VAR_PUSE, p);
			end = dug_newblk(dug);
			dug_branch(dug, e1.e, end, COND_BOOLEAN, 0, p);
			dug_branch(dug, Fend, end, COND_UNCONDITIONAL, 0, NULL);
			p = CHILD1(n);
			s1.s = dug_newblk(dug);
			fg_stmt(p, dug, s1.s, end, e1.s, swblk, &s1.e, dblk);
			dug_branch(dug, e1.e, s1.s, COND_BOOLEAN, 1, CHILD0(n));
			dug_branch(dug, Tend, s1.s, COND_UNCONDITIONAL, 0,NULL);
			dug_branch(dug, s1.e, e1.s, COND_UNCONDITIONAL,0,NULL);
			break;
		}
	case STMT_DO:
		{
			TNODE		*p;
			SUBGRAPH	e1;	/* expr 1 */
			SUBGRAPH	s1;	/* stmt 1 */
			BLOCK *		Tend;
			BLOCK *		Fend;

			/*
			* (Can't do startblk() here because first stmt of s1
			* could be: while ... which would leave an empty block.)
			*/
			end = dug_newblk(dug);
			p = CHILD0(n);
			s1.s = dug_newblk(dug);
			dug_branch(dug, sblk, s1.s, COND_UNCONDITIONAL,0,NULL);
			e1.s = dug_newblk(dug);
			fg_stmt(p, dug, s1.s, end, e1.s, swblk, &s1.e, dblk);
			p = CHILD1(n);
			dug_branch(dug, s1.e, e1.s, COND_UNCONDITIONAL,0,NULL);
			fg_expr(p, dug, e1.s, &e1.e, &Tend, &Fend,
				VAR_PUSE, VAR_VOID);
			dug_du(dug, &decis_sym, e1.e, VAR_PUSE, p);

			dug_branch(dug, e1.e, s1.s, COND_BOOLEAN, 1, p);
			dug_branch(dug, Tend, s1.s, COND_UNCONDITIONAL, 0,NULL);
			dug_branch(dug, e1.e, end, COND_BOOLEAN, 0, p);
			dug_branch(dug, Fend, end, COND_UNCONDITIONAL, 0, NULL);
			break;
		}
	case STMT_FOR_EEES:	/* for (E;E;E) S; */
		fg_for(dug, CHILD0(n), CHILD1(n), CHILD2(n), CHILD3(n), sblk,
			swblk, &end, dblk);
		break;
	case STMT_FOR_EEE_:	/* for (E;E;E) ; */
		fg_for(dug, CHILD0(n), CHILD1(n), CHILD2(n), NULL, sblk, swblk,
			&end, dblk);
		break;
	case STMT_FOR_EE_S:	/* for (E;E; ) S; */
		fg_for(dug, CHILD0(n), CHILD1(n), NULL, CHILD2(n), sblk, swblk,
			&end, dblk);
		break;
	case STMT_FOR_EE__:	/* for (E;E; ) ; */
		fg_for(dug, CHILD0(n), CHILD1(n), NULL, NULL, sblk, swblk, &end,
			dblk);
		break;
	case STMT_FOR_E_ES:	/* for (E; ;E) S; */
		fg_for(dug, CHILD0(n), NULL, CHILD1(n), CHILD2(n), sblk, swblk,
			&end, dblk);
		break;
	case STMT_FOR_E_E_:	/* for (E; ;E) ; */
		fg_for(dug, CHILD0(n), NULL, CHILD1(n), NULL, sblk, swblk, &end,
			dblk);
		break;
	case STMT_FOR_E__S:	/* for (E; ; ) S; */
		fg_for(dug, CHILD0(n), NULL, NULL, CHILD1(n), sblk, swblk, &end,
			dblk);
		break;
	case STMT_FOR_E___:	/* for (E; ; ) ; */
		fg_for(dug, CHILD0(n), NULL, NULL, NULL, sblk, swblk, &end,
			dblk);
		break;
	case STMT_FOR__EES:	/* for ( ;E;E) S; */
		fg_for(dug, NULL, CHILD0(n), CHILD1(n), CHILD2(n), sblk, swblk,
			&end, dblk);
		break;
	case STMT_FOR__EE_:	/* for ( ;E;E) ; */
		fg_for(dug, NULL, CHILD0(n), CHILD1(n), NULL, sblk, swblk, &end,
			dblk);
		break;
	case STMT_FOR__E_S:	/* for ( ;E; ) S; */
		fg_for(dug, NULL, CHILD0(n), NULL, CHILD1(n), sblk, swblk, &end,
			dblk);
		break;
	case STMT_FOR__E__:	/* for ( ;E; )  ; */
		fg_for(dug, NULL, CHILD0(n), NULL, NULL, sblk, swblk, &end,
			dblk);
		break;
	case STMT_FOR___ES:	/* for ( ; ;E) S; */
		fg_for(dug, NULL, NULL, CHILD0(n), CHILD1(n), sblk, swblk, &end,
			dblk);
		break;
	case STMT_FOR___E_:	/* for ( ; ;E)  ; */
		fg_for(dug, NULL, NULL, CHILD0(n), NULL, sblk, swblk, &end,
			dblk);
		break;
	case STMT_FOR____S:	/* for ( ; ; ) S; */
		fg_for(dug, NULL, NULL, NULL, CHILD0(n), sblk, swblk, &end,
			dblk);
		break;
	case STMT_SWITCH:
		{
			BLOCK *		newdblk;
			SUBGRAPH	e1;	/* expr 1 */
			SUBGRAPH	s1;	/* stmt 1 */

			dug_startblk(dug, sblk, n);
			end = dug_newblk(dug);
			fg_expr(CHILD0(n), dug, sblk, &e1.e, NULL, NULL,
				VAR_PUSE, VAR_VOID);
			dug_du(dug, &decis_sym, e1.e, VAR_PUSE, CHILD0(n));
			newdblk = NULL_BLK;
			fg_stmt(CHILD1(n), dug, NULL_BLK, end, cblk, e1.e,
				&s1.e, &newdblk);
			/*
			* sblk is passed as swblk.
			* Each "case" will define an edge from swblk to itself.
			* "Default" will define an edge from swblk to itself.
			* "Default" will define an edge from s1.e to itself.
			* If newdblk comes back NULL, we define an edge from
			* swblk to end.
			*/
			if (newdblk == NULL_BLK)
				dug_branch(dug, e1.e, end, COND_SWITCH_DEFAULT,
					   0, NULL);
			dug_branch(dug, s1.e, end, COND_UNCONDITIONAL,0,NULL);
			break;
		}
	case STMT_BREAK:
		dug_startblk(dug, sblk, n);
		dug_endblk(dug, sblk, n);
		end = NULL_BLK;
		if (bblk) dug_branch(dug, sblk, bblk, COND_UNCONDITIONAL,
				     0, NULL);
		else semantic_error(n->srcpos,
			"break not in switch, for, while, or do.");
		break;
	case STMT_CONTINUE:
		dug_startblk(dug, sblk, n);
		dug_endblk(dug, sblk, n);
		end = NULL_BLK;
		if (cblk) dug_branch(dug, sblk, cblk, COND_UNCONDITIONAL, 0,
				     NULL);
		else semantic_error(n->srcpos,
			"continue not in for, while, or do.");
		break;
	case STMT_RETURN_EXPR:
		{
			TNODE		*p;

			dug_startblk(dug, sblk, n);
			p = CHILD0(n);
			fg_expr(p, dug, sblk, &end, NULL, NULL,
				VAR_CUSE, VAR_VOID);
			/*
			* If the return expression ends a block, no block is
			* created for the return itself (e.g. return f();).
			* Predicate uses that end with the return are identified
			* by block 0. Block coverage will not show failure to
			* return (e.g. return exit();).
			*/
			if (end == sblk) dug_endblk(dug, end, n);
			end = NULL_BLK;
			break;
		}
	case STMT_RETURN:
		dug_startblk(dug, sblk, n);
		dug_endblk(dug, sblk, n);
		end = NULL_BLK;
		break;
	case STMT_GOTO:
		dug_startblk(dug, sblk, n);
		fg_stmt_goto(dug, (GOTOLIST **)&CHILD0(n)->sym.sym->type.label,
			     sblk);
		dug_endblk(dug, sblk, n);
		end = NULL_BLK;
		break;
	case STMT_LABEL:
	case STMT_CASE:
	case STMT_DEFAULT:
	default:
		internal_error(n->srcpos, "****fg_stmt: unknown stmt species.");
		break;
	}

	*endblk = end;
	return;
}

static void
fg_stmt_label(dug, gotolist, sblk)
DUG *		dug;
GOTOLIST **	gotolist;
BLOCK *		sblk;
{
	GOTOLIST	*g;
	GOTOLIST	*tmpg;

	/*
	* Add an edge to each goto for this label already 
	* encountered.  
	*/
	g = *gotolist;
	if (g == NULL) {
		g = (GOTOLIST *)malloc(sizeof *g);
		CHECK_MALLOC(g);
		g->block = sblk;
		g->next = NULL;
		*gotolist = g;
	} else {
		g->block = sblk;
		tmpg = g;
		g = g->next;
		tmpg->next = NULL;
		while (g != NULL) {
			dug_branch(dug, g->block, sblk, COND_UNCONDITIONAL, 0,
				   NULL);
			tmpg = g;
			g = g->next;
			free(tmpg);
		}
	}
}

static void
fg_stmt_goto(dug, gotolist, sblk)
DUG *		dug;
GOTOLIST **	gotolist;
BLOCK *		sblk;
{
	GOTOLIST	*g;
	GOTOLIST	*tmpg;

	/*
	* If the label has not been seen yet,
	* the first entry on the list contains
	* NULL_BLK, and the remaining entries each contain a block
	* number that needs an edge to the label.  After the label has
	* been seen the list is reduced to one entry with the number
	* of the block containing the label.
	*/
	g = *gotolist;
	if (g == NULL) {
		g = (GOTOLIST *)malloc(sizeof *g);
		CHECK_MALLOC(g);
		g->block = NULL_BLK;
		g->next = (GOTOLIST *)malloc(sizeof *g);
		CHECK_MALLOC(g->next);
		g->next->block = sblk;
		g->next->next = NULL;
		*gotolist = g;
	}
	else if (g->next != NULL) {
		tmpg = g->next;
		g->next = (GOTOLIST *)malloc(sizeof *g);
		CHECK_MALLOC(g->next);
		g->next->block = sblk;
		g->next->next = tmpg;
	} else {
		dug_branch(dug, sblk, g->block, COND_UNCONDITIONAL, 0, NULL);
	}
}

static void
fg_for(dug, expr1, expr2, expr3, stmt, sblk, swblk, endblk, dblk)
DUG	*dug;
TNODE	*expr1;
TNODE	*expr2;
TNODE	*expr3;
TNODE	*stmt;
BLOCK *	sblk; 
BLOCK *	swblk;
BLOCK **endblk;
BLOCK **dblk;
{
	SUBGRAPH	e1;	/* expr 1 */
	SUBGRAPH	e2;	/* expr 2 */
	SUBGRAPH	e3;	/* expr 3 */
	SUBGRAPH	s1;	/* stmt 1 */
	BLOCK *		end;	/* block following stmt */
	BLOCK *		cont;	/* target of continue */
	BLOCK *		Tend;
	BLOCK *		Fend;

	/*
	* ... sblk for (expr1; expr2; expr3) stmt; end ...
	*
	* expr1 is part of sblk. Normally the following edges are needed:
	*	end of expr1 ==> start of stmt
	*	end of stmt ==> start of expr3
	*	end of expr3 ==> start of expr2
	*	end of expr2 ==> start of stmt
	*	end of expr2 ==> end
	*	any continue statement in stmt ==> start of expr3.
	*	any break statement in stmt ==> end
	* New nodes are always created for the start of expr2, and end.
	* If expr2 is missing, the node for expr2 is a dummy and is not
	* marked in the code;  the edge from expr2 to end is ommitted.
	* Nodes for expr2 and stmt must not be created if they are empty
	* because P-USEs of variables in expr2 will require a path to
	* the nodes following the end of expr2 which must not be dummy nodes.
	* If stmt is missing, the edge from the end of expr2 goes directly
	* to the start of expr3.  If expr3 is missing the edge from the end
	* of stmt goes directly to the start of expr2, and the edge from
	* any continue statement in stmt goes directly to the start of expr2.
	* If both are missing, the edge from the end of expr2 goes to the
	* start of expr2.
	*/
	end = dug_newblk(dug);
	if (expr1) {
		dug_startblk(dug, sblk, PARENT(expr1));
		fg_expr(expr1, dug, sblk, &e1.e, NULL, NULL,
			VAR_VOID, VAR_VOID);
	} else e1.e = sblk;
	e2.s = dug_newblk(dug);
	dug_branch(dug, e1.e, e2.s, COND_UNCONDITIONAL, 0, NULL);
	if (expr2) {
		if (expr1 == NULL)
			dug_startblk(dug, e2.s, PARENT(expr2));
		fg_expr(expr2, dug, e2.s, &e2.e, &Tend, &Fend,
			VAR_PUSE, VAR_VOID);
		dug_du(dug, &decis_sym, e2.e, VAR_PUSE, expr2);
		dug_branch(dug, e2.e, end, COND_BOOLEAN, 0, expr2);
		dug_branch(dug, Fend, end, COND_UNCONDITIONAL, 0, NULL);
	} else {
		e2.e = e2.s;
		Tend = 0;
	}
	if (expr3) {
		e3.s = dug_newblk(dug);
		cont = e3.s;
	} else cont = e2.s;
	if (stmt) {
		s1.s = dug_newblk(dug);
		if (expr2) dug_branch(dug, e2.e, s1.s, COND_BOOLEAN, 1, expr2);
		else dug_branch(dug, e2.e, s1.s, COND_UNCONDITIONAL, 0, NULL);
		dug_branch(dug, Tend, s1.s, COND_UNCONDITIONAL, 0, NULL);
		Tend = 0;
		fg_stmt(stmt, dug, s1.s, end, cont, swblk, &s1.e, dblk);
	} else s1.e = e2.e;
	if (expr3) {
		fg_expr(expr3, dug, e3.s, &e3.e, NULL, NULL, VAR_VOID,VAR_VOID);
		if (stmt) {		/* for (*; *; e3) s; */
		    if ((s1.e == s1.s) && (e3.e == e3.s)) {
			/*
			* Don't worry that cont = e3.s because if it were
			* used in fg_stmt, s1.e would not equal s1.s.
			*/
			dug_du_combine(dug, s1.s, e3.s);
			dug_startblk(dug, s1.s, NULL);
			dug_startblk(dug, s1.s, expr3);
			dug_endblk(dug, s1.s, stmt);
			dug_branch(dug, s1.e, e2.s, COND_UNCONDITIONAL,0,NULL);
		    } else {
			dug_branch(dug, s1.e, e3.s, COND_UNCONDITIONAL, 0,NULL);
			dug_branch(dug, e3.e, e2.s, COND_UNCONDITIONAL, 0,NULL);
		    }
		} else {		/* for (*;  *; e3) ; */
		    if (expr2) {	/* for (*; e2; e3) ; */
			dug_branch(dug, e2.e, e3.s, COND_BOOLEAN, 1, expr2);
			dug_branch(dug, Tend, e3.s, COND_UNCONDITIONAL, 0,NULL);
		    }
		    dug_branch(dug, e3.e, e2.s, COND_UNCONDITIONAL, 0, NULL);
		}
	} else dug_branch(dug, s1.e, e2.s, COND_UNCONDITIONAL, 0, NULL);

	*endblk = end;
	return;
}
