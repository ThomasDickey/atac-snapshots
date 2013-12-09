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

static const char tree_c[] = "$Id: tree.c,v 3.10 2013/12/09 00:19:10 tom Exp $";
/*
* @Log: tree.c,v @
* Revision 3.7  1997/05/11 20:26:01  tom
* moved prototypes to tnode.h
*
* Revision 3.6  1997/05/10 23:20:36  tom
* absorb srcpos.h into error.h
*
* Revision 3.5  1996/11/13 00:40:33  tom
* change ident to 'const' to quiet gcc
* add forward-ref prototypes
*
* Revision 3.4  94/04/04  10:15:10  jrh
* Add Release Copyright
* 
* Revision 3.3  93/08/04  15:48:45  ewk
* Added MVS and solaris support.  Squelched some ANSI warnings.
* 
* Revision 3.2  93/07/12  11:49:37  saul
* MVS MODULEID
* MVS tFindDefValue ==> tFindVDef for uniqueness
* 
* Revision 3.1  93/07/09  14:47:01  saul
* __DATE__ changed to __STDC__
* 
* Revision 3.0  92/11/06  07:45:06  saul
* propagate to version 3.0
* 
* Revision 2.7  92/10/30  09:49:21  saul
* include portable.h
* 
* Revision 2.6  92/09/22  15:21:08  saul
* ANSI preprocessor doesn't expand macros in strings.  Use #ifdef.
* 
* Revision 2.5  92/07/10  12:39:40  saul
* added tFind routines
* 
* Revision 2.4  92/03/17  14:23:09  saul
* copyright
* 
* Revision 2.3  91/10/23  13:21:46  saul
* Handle "*const volatile".
* 
* Revision 2.2  91/06/13  12:58:15  saul
* add tree printing stuff that was in main.c (parser.c)
* 
* Revision 2.1  91/06/13  12:39:28  saul
* Propagate to version 2.0
* 
* Revision 1.1  91/06/12  20:25:54  saul
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
#include "sym.h"
#include "tree.h"
#include "hook.h"		/* for tree_print of GEN_HOOK */

#define CHECK_MALLOC(p) ((p)?1:internal_error(NULL, "Out of memory\n"))

TNODE *
tmkleaf(
	   int genus,
	   int species,
	   SRCPOS * srcpos,
	   char *text)
{
    TNODE *t;

    t = (TNODE *) malloc(sizeof *t);
    CHECK_MALLOC(t);
    t->genus = genus;
    t->species = species;
    t->srcpos[LEFT_SRCPOS].file = srcpos[LEFT_SRCPOS].file;
    t->srcpos[LEFT_SRCPOS].line = srcpos[LEFT_SRCPOS].line;
    t->srcpos[LEFT_SRCPOS].col = srcpos[LEFT_SRCPOS].col;
    t->srcpos[RIGHT_SRCPOS].file = srcpos[RIGHT_SRCPOS].file;
    t->srcpos[RIGHT_SRCPOS].line = srcpos[RIGHT_SRCPOS].line;
    t->srcpos[RIGHT_SRCPOS].col = srcpos[RIGHT_SRCPOS].col;
    t->text = text;
    t->sym.symtab = NULL;
    t->sym.sym = NULL;

    t->up = NULL;
    t->down = NULL;
    t->over = t;

    return t;
}

TNODE *
tmknode(
	   int genus,
	   int species,
	   TNODE * Child0,
	   TNODE * Child1)
{
    TNODE *t;

    t = (TNODE *) malloc(sizeof *t);
    CHECK_MALLOC(t);
    t->genus = genus;
    t->species = species;
    t->up = NULL;
    t->over = t;
    t->text = NULL;
    t->sym.symtab = NULL;
    t->sym.sym = NULL;

    if (Child0 == NULL) {
	t->srcpos[LEFT_SRCPOS].file = -1;
	t->srcpos[LEFT_SRCPOS].line = 0;
	t->srcpos[LEFT_SRCPOS].col = 0;
	t->srcpos[RIGHT_SRCPOS].file = -1;
	t->srcpos[RIGHT_SRCPOS].line = 0;
	t->srcpos[RIGHT_SRCPOS].col = 0;
	t->down = NULL;
	return t;
    }

    t->srcpos[LEFT_SRCPOS].file = Child0->srcpos[LEFT_SRCPOS].file;
    t->srcpos[LEFT_SRCPOS].line = Child0->srcpos[LEFT_SRCPOS].line;
    t->srcpos[LEFT_SRCPOS].col = Child0->srcpos[LEFT_SRCPOS].col;

    Child0->up = t;

    if (Child1) {
	Child1->up = t;
	Child0->over = Child1;
	Child1->over = Child0;
	t->down = Child1;
	t->srcpos[RIGHT_SRCPOS].file =
	    Child1->srcpos[RIGHT_SRCPOS].file;
	t->srcpos[RIGHT_SRCPOS].line =
	    Child1->srcpos[RIGHT_SRCPOS].line;
	t->srcpos[RIGHT_SRCPOS].col =
	    Child1->srcpos[RIGHT_SRCPOS].col;
    } else {
	t->down = Child0;
	t->srcpos[RIGHT_SRCPOS].file =
	    Child0->srcpos[RIGHT_SRCPOS].file;
	t->srcpos[RIGHT_SRCPOS].line =
	    Child0->srcpos[RIGHT_SRCPOS].line;
	t->srcpos[RIGHT_SRCPOS].col =
	    Child0->srcpos[RIGHT_SRCPOS].col;
    }

    return t;
}

TNODE *
tlist_add(
	     TNODE * list,
	     TNODE * next)
{
    if (list->down == NULL) {
	list->srcpos[LEFT_SRCPOS].file = next->srcpos[LEFT_SRCPOS].file;
	list->srcpos[LEFT_SRCPOS].line = next->srcpos[LEFT_SRCPOS].line;
	list->srcpos[LEFT_SRCPOS].col = next->srcpos[LEFT_SRCPOS].col;
	/* assume: next->over == next */
    } else {
	next->over = list->down->over;
	list->down->over = next;
    }
    next->up = list;
    list->down = next;
    list->srcpos[RIGHT_SRCPOS].file = next->srcpos[RIGHT_SRCPOS].file;
    list->srcpos[RIGHT_SRCPOS].line = next->srcpos[RIGHT_SRCPOS].line;
    list->srcpos[RIGHT_SRCPOS].col = next->srcpos[RIGHT_SRCPOS].col;

    return list;
}

TNODE *
tlist_ladd(
	      TNODE * list,
	      TNODE * next)
{
    if (list->down == NULL) {
	list->srcpos[RIGHT_SRCPOS].file =
	    next->srcpos[RIGHT_SRCPOS].file;
	list->srcpos[RIGHT_SRCPOS].line =
	    next->srcpos[RIGHT_SRCPOS].line;
	list->srcpos[RIGHT_SRCPOS].col =
	    next->srcpos[RIGHT_SRCPOS].col;
	list->down = next;
	/* assume: next->over == next */
    } else {
	next->over = list->down->over;
	list->down->over = next;
    }
    next->up = list;
    list->srcpos[LEFT_SRCPOS].file = next->srcpos[LEFT_SRCPOS].file;
    list->srcpos[LEFT_SRCPOS].line = next->srcpos[LEFT_SRCPOS].line;
    list->srcpos[LEFT_SRCPOS].col = next->srcpos[LEFT_SRCPOS].col;

    return list;
}

TNODE *
tsrc_pos(
	    TNODE * node,
	    SRCPOS * begin,
	    SRCPOS * end)
{
    if (begin) {
	node->srcpos[LEFT_SRCPOS].file = begin[LEFT_SRCPOS].file;
	node->srcpos[LEFT_SRCPOS].line = begin[LEFT_SRCPOS].line;
	node->srcpos[LEFT_SRCPOS].col = begin[LEFT_SRCPOS].col;
    }

    if (end) {
	node->srcpos[RIGHT_SRCPOS].file = end[RIGHT_SRCPOS].file;
	node->srcpos[RIGHT_SRCPOS].line = end[RIGHT_SRCPOS].line;
	node->srcpos[RIGHT_SRCPOS].col = end[RIGHT_SRCPOS].col;
    }

    return node;
}

void
tfreenode(
	     TNODE * node)
{
    if (node == NULL)
	return;
    free(node);
}

#define GENSTR(s) case s: return #s;

static const char *
genstr(int token)
{
    static char buf[20];

    switch (token) {
	GENSTR(GEN_MODULE)
	    GENSTR(GEN_MODULE_ITEM)
	    GENSTR(GEN_FUNCTION)
	    GENSTR(GEN_FUNC_SPEC)
	    GENSTR(GEN_CLASSTYPES)
	    GENSTR(GEN_CLASSTYPE)
	    GENSTR(GEN_PARAM_DCLS)
	    GENSTR(GEN_PARAM_DCL)
	    GENSTR(GEN_PARAM_DEFS)
	    GENSTR(GEN_STARS)
	    GENSTR(GEN_STAR)
	    GENSTR(GEN_STMT_LIST)
	    GENSTR(GEN_ENUM_DCL)
	    GENSTR(GEN_ENUM_REF)
	    GENSTR(GEN_MOE_LIST)
	    GENSTR(GEN_MOE)
	    GENSTR(GEN_STRUCT_DCL)
	    GENSTR(GEN_STRUCT_REF)
	    GENSTR(GEN_MEM_LIST)
	    GENSTR(GEN_MEMBER)
	    GENSTR(GEN_MEM_DCLS)
	    GENSTR(GEN_MEM_DCL)
	    GENSTR(GEN_NAMES)
	    GENSTR(GEN_INIT_DCL)
	    GENSTR(GEN_INDATA_DCLS)
	    GENSTR(GEN_INDATA_DCL)
	    GENSTR(GEN_DATA_SPECS)
	    GENSTR(GEN_DATA_SPEC)
	    GENSTR(GEN_DATA_ITEMS)
	    GENSTR(GEN_DATA_ITEM)
	    GENSTR(GEN_INIT_LIST)
	    GENSTR(GEN_INIT_ITEM)
	    GENSTR(GEN_INITIALIZER)
	    GENSTR(GEN_COMPSTMT)
	    GENSTR(GEN_STMT)
	    GENSTR(GEN_EXP_LIST)
	    GENSTR(GEN_EXPR)
	    GENSTR(GEN_FUNC_LP)
	    GENSTR(GEN_CAST_TYPE)
	    GENSTR(GEN_NULL_DCL)
	    GENSTR(GEN_BINOP)
	    GENSTR(GEN_INCOP)
	    GENSTR(GEN_UNOP)
	    GENSTR(GEN_ANSI_PARAMS)
	    GENSTR(GEN_ANSI_PARAM)
	    GENSTR(GEN_QUALS)
	    GENSTR(GEN_QUAL)
	    GENSTR(GEN_FCON)
	    GENSTR(GEN_ICON)
	    GENSTR(GEN_STRING)
	    GENSTR(GEN_NAME)
	    GENSTR(GEN_TNAME)
	    GENSTR(GEN_FNAME)
    default:
	sprintf(buf, "%d", token);
	return buf;
    }
}

int
print_tree(
	      TNODE * node,
	      int id,
	      int parent,
	      int level)
{
    TNODE *p;
    int next;
    int i;

    if (node == NULL)
	return id;

    printf("%3.3d/%3.3d:", id, parent);
    for (i = 0; i < level; ++i) {
	putchar('|');
	putchar(' ');
    }
    printf("%s.%d", genstr(node->genus), node->species);
    if (node->text)
	printf(" <%s>\n", node->text);
    else
	putchar('\n');

    if (node->down == NULL)
	return id + 1;

    p = node->down;

    next = id + 1;
    do {
	p = p->over;
	next = print_tree(p, next, id, level + 1);
    } while (p != node->down);

    return next;
}

/*
* tFindDef:  Given a GEN_NAME node for a variable that is getting a data-flow
*	def, return the parse node representing the assignment:
*
*	o assignments (=, op=) - return the assignment expression.
*	o increment (++x, --x, x++, x--) - return the increment expression.
*	o declaration - return whole declaration stmt.
*	o ansi parameter - return type, qualifiers, and name.
*	o non-ansi parmeter - return just the name.
*/
TNODE *
tFindDef(
	    TNODE * n)
{
    TNODE *p;

    if (n == NULL)
	return NULL;

    for (p = n; p; p = PARENT(p)) {
	switch (p->genus) {
	case GEN_EXPR:
	    if (p->species == EXPR_INCOP)
		return p;
	    if (p->species == EXPR_UNOP) {
		if (CHILD0(p)->species == UNOP_INC)
		    return p;
		if (CHILD0(p)->species == UNOP_DEC)
		    return p;
	    } else if (p->species == EXPR_BINOP) {
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
		    return p;
		}
	    }
	    break;
	case GEN_INIT_DCL:
	case GEN_INDATA_DCL:
	case GEN_ANSI_PARAM:
	case GEN_PARAM_DCL:
	    return p;
	case GEN_NAMES:	/* non-ANSI style parameter list */
	    return n;
	}
    }
    internal_error(n->srcpos, "tFindDef: can't find def");
    /*NOTREACHED */
}

/*
* tFindVDef:  Given a GEN_NAME node for a variable that is getting a
*	data-flow return the value being assigned in *value.
*/
void
tFindVDef(
	     TNODE * n,
	     CONST_VALUE * value)
{
    TNODE *p;

    value->type = CONST_VT_UNDETERMINED;

    p = tFindDef(n);
    if (p == NULL)
	return;

    if (p->genus == GEN_EXPR && p->species == EXPR_BINOP &&
	CHILD1(p)->species == BINOP_ASGN) {
	evalConstExpr(CHILD2(p), value);
	return;
    }

    if (p->genus == GEN_INDATA_DCL) {
	for (p = n; p; p = PARENT(p)) {
	    if (p->genus == GEN_DATA_SPEC) {
		if (p->species != DATA_SPEC_INIT)
		    return;
		p = CHILD1(p);
		if (p->genus != GEN_INITIALIZER)
		    internal_error(p->srcpos,
				   "tFindVDef: can't find INITIALIZER");
		if (p->species != INITIALIZER_EXPR)
		    return;
		evalConstExpr(CHILD0(p), value);
		return;
	    }
	}
	internal_error(n->srcpos, "tFindVDef: can't find DATA_SPEC");
    }

    return;
}

/*
* tFindPred:  Given a GEN_NAME node for a variable that is in a data-flow
*	predicate, return the parse node representing the predicate.
*	Given a GEN_EXPR for a node that is a predicate, return the
*	right most part of the predicate (e.g. "a && (b=(c>d))"
*	returns "c>d").
*/
TNODE *
tFindPred(
	     TNODE * n)
{
    TNODE *p;
    TNODE *prev;
    int species;

    /*
     * EXPR handling.
     */
    if (n->genus == GEN_EXPR) {
	p = n;
	while (1) {
	    species = p->species;
	    if (p->genus == GEN_HOOK)
		p = CHILD0(p);
	    else if (p->genus != GEN_EXPR)
		internal_error(p->srcpos,
			       "tFindPred1: expected GEN_EXPR; %s\n",
			       genstr(p->genus));
	    else if (species == EXPR_INHERIT)
		p = CHILD0(p);
	    else if (species == EXPR_INCOP)
		p = CHILD0(p);
	    else if (species == EXPR_COMMA)
		p = CHILD1(p);
	    else if (species == EXPR_BINOP) {
		species = CHILD1(p)->species;
		if (species == BINOP_ASGN)
		    p = CHILD2(p);
		else if (species == BINOP_ANDAND)
		    p = CHILD2(p);
		else if (species == BINOP_OROR)
		    p = CHILD2(p);
		else
		    return p;
	    } else
		return p;
	}			/* while */
    }

    /*
     * NAME handling.
     */
    prev = n;
    for (p = n; p; prev = p, p = PARENT(p)) {
	switch (p->genus) {
	case GEN_HOOK:
	    continue;
	case GEN_EXPR:
	    species = p->species;
	    if (species == EXPR_QCOLON) {
		if (CHILD0(p) == prev)
		    break;
	    } else if (species == EXPR_BINOP) {
		species = CHILD1(p)->species;
		if (species == BINOP_ANDAND || species == BINOP_OROR)
		    break;
	    }
	    continue;
	case GEN_STMT:
	    break;
	default:
	    continue;
	}
	p = prev;
	/*
	 * Found predicate parse node.  Go back down over irrelevant stuff.
	 */
	while (1) {
	    species = p->species;
	    if (p->genus == GEN_HOOK)
		p = CHILD0(p);
	    else if (p->genus != GEN_EXPR)
		internal_error(p->srcpos,
			       "tFindPred2: expected GEN_EXPR; %s\n",
			       genstr(p->genus));
	    else if (species == EXPR_INHERIT)
		p = CHILD0(p);
	    else if (species == EXPR_INCOP)
		p = CHILD0(p);
	    else if (species == EXPR_COMMA)
		p = CHILD1(p);
	    else if (species == EXPR_BINOP &&
		     CHILD1(p)->species == BINOP_ASGN)
		p = CHILD2(p);
	    else
		return p;
	}			/* while */
    }				/* for */
    internal_error(n->srcpos, "tFindPred: can't find Pred\n");
    /*NOTREACHED */
}

/*
* tFindSwitch:  Given any node syntacticly inside a SWITCH statement, return
*	a pointer to the switch expression node.  If not found, return NULL;
*/
TNODE *
tFindSwitch(
	       TNODE * n)
{
    TNODE *p;

    for (p = n; p; p = PARENT(p)) {
	if (p->genus == GEN_STMT && p->species == STMT_SWITCH)
	    return CHILD0(p);
    }

    return NULL;
}
