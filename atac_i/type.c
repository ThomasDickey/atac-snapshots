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
#pragma csect (CODE, "type$")
#include <mvapts.h>
MODULEID(%M%,%J%/%D%/%T%)
#include <string.h>
#include <stdlib.h>
#endif /* MVS */

static const char type_c[] = "$Id: type.c,v 3.9 2013/12/08 18:51:16 tom Exp $";
/*
* @Log: type.c,v @
* Revision 3.8  1997/05/11 22:16:52  tom
* moved prototypes to tnode.h
*
* Revision 3.7  1997/05/10 23:14:50  tom
* absorb srcpos.h into error.h
*
* Revision 3.6  1996/11/13 00:41:44  tom
* change ident to 'const' to quiet gcc
* add forward-ref prototypes
*
* Revision 3.5  94/04/04  10:15:23  jrh
* Add Release Copyright
* 
* Revision 3.4  94/03/21  08:26:21  saul
* MVS support __offsetof as builtin (not handled by cpp)
* 
* Revision 3.3  1993/12/15  11:32:53  saul
* evaluate "p+1" to pointer
*
* Revision 3.2  93/08/04  15:49:10  ewk
* Added MVS and solaris support.  Squelched some ANSI warnings.
* 
* Revision 3.1  93/07/12  11:50:43  saul
* MVS MODULEID
* 
* Revision 3.0  92/11/06  07:45:29  saul
* propagate to version 3.0
* 
* Revision 2.5  92/10/30  09:49:24  saul
* include portable.h
* 
* Revision 2.4  92/07/10  13:26:20  saul
* support constant expression evaluation; be more tolerant
* 
* Revision 2.3  92/05/08  08:02:15  saul
* fix wording in error message.
* 
* Revision 2.2  92/03/17  14:23:13  saul
* copyright
* 
* Revision 2.1  91/06/13  12:39:30  saul
* Propagate to version 2.0
* 
* Revision 1.1  91/06/12  20:25:56  saul
* Aug 1990 baseline
* 
*-----------------------------------------------end of log
*/
#include <stdio.h>
#include "portable.h"
#include "error.h"
#include "tnode.h"
#include "sym.h"
#include "tree.h"
#include "hook.h"

/* forward declarations */
static void usual_arith_conv(VALTYPE * op1, VALTYPE * op2, VALTYPE * type);

void
clear_type(VALTYPE * type)
{
    type->sclass = 0;
    type->base = 0;
    type->bits = -1;
    type->bit_alignment = 0;
    type->qual = 0;
    type->param_list = NULL;
    type->tag = NULL;
    type->dimensions = NULL;
}

void
copy_type(VALTYPE * from,
	  VALTYPE * to)
{
    to->sclass = from->sclass;
    to->base = from->base;
    to->bits = -1;
    to->bit_alignment = 0;
    to->qual = from->qual;
    to->param_list = from->param_list;
    to->tag = from->tag;
    to->dimensions = from->dimensions;
}

/*
* usual_arith_conv:  compute the "usual arithmetic conversions" for operations
*	involving op1 and op2 and put the type int type.
* 	WARNING: this function may modify op1 and op2.
*/
static void
usual_arith_conv(VALTYPE * op1,
		 VALTYPE * op2,
		 VALTYPE * type)
{
    /*
     * Note: there is no check that op1 and op2 are arith types.  They could
     * be pointers to arith types.
     */
    clear_type(type);
    if ((op1->base == BT_LONGDOUBLE) || (op2->base == BT_LONGDOUBLE)) {
	type->base = BT_LONGDOUBLE;
	return;
    }
    if ((op1->base == BT_DOUBLE) || (op2->base == BT_DOUBLE)) {
	type->base = BT_DOUBLE;
	return;
    }
    if ((op1->base == BT_FLOAT) || (op2->base == BT_FLOAT)) {
	type->base = BT_FLOAT;
	return;
    }

    int_promote(op1);
    int_promote(op2);
    if ((op1->base == BT_ULONG) || (op2->base == BT_ULONG))
	type->base = BT_ULONG;
    else if (((op1->base == BT_LONG) && (op2->base == BT_UINT)) ||
	     ((op1->base == BT_UINT) && (op2->base == BT_LONG))) {
	/* ? if the largest UNSIGNED INT will fit in a signed LONG, use signed LONG */
	type->base = BT_ULONG;
    } else if ((op1->base == BT_LONG) || (op2->base == BT_LONG))
	type->base = BT_LONG;
    else if ((op1->base == BT_UINT) || (op2->base == BT_UINT))
	type->base = BT_ULONG;
    else
	type->base = BT_INT;
}

void
int_promote(VALTYPE * type)
{
    if (type->qual)
	return;			/* not a simple type */
    if (type->base == BT_ENUM) {
	/* ? if ENUM has values that don't fit in an INT use UNSIGNED */
	type->base = BT_INT;
	return;
    }
    if (!(type->base & BTB_INT))
	return;			/* not an INT type */

    if (type->bits != -1) {
	/* ? if field has values that don't fit in an INT use UNSIGNED */
	type->bits = -1;
	type->base = BT_INT;
	return;
    }

    if ((type->base & BTB_CHAR_SIZE) || (type->base & BTB_SHORT_SIZE)) {
	/* ? if the largest UNSIGNED CHAR or UNSIGNED SHORT won't fit in an INT, */
	/* use UNSIGNED */
	type->base = BT_INT;
	return;
    }

    return;
}

void
expr_type(TNODE * n,
	  VALTYPE * type)
{
    VALTYPE op1;
    VALTYPE op2;
    TNODE *p;
    SYM *sym;

    if (n->genus == GEN_HOOK) {
	expr_type(CHILD0(n), type);
	return;
    }

    if (n->genus != GEN_EXPR)
	internal_error(n->srcpos, "type: unexpected genus: %d",
		       n->genus);

    switch (n->species) {
    case EXPR_QCOLON:
	/*
	 * "E ? A : B"
	 * A and B both arithmentic: usual conversions.
	 * A or B integral: must be something compared with NULL
	 *                    return the type of "something".
	 * A or B pointer to void: return the type of the other.
	 * Else: Assume they are both the same type and return 
	 *                    the type of the first.
	 */
	expr_type(CHILD1(n), &op1);
	expr_type(CHILD2(n), &op2);
	if (BT_ISARITH(op1.base) && op1.qual == 0
	    && BT_ISARITH(op2.base) && op2.qual == 0) {
	    usual_arith_conv(&op1, &op2, type);
	} else if ((op1.base & BTB_INT) && op1.qual == 0)
	    copy_type(&op2, type);
	else if ((op2.base & BTB_INT) && op1.qual == 0)
	    copy_type(&op1, type);
	else if (op1.base == BT_VOID && op1.qual == QUAL_PTR)
	    copy_type(&op2, type);
	else if (op2.base == BT_VOID && op2.qual == QUAL_PTR)
	    copy_type(&op1, type);
	else
	    copy_type(&op1, type);
	break;
    case EXPR_COMMA:
	expr_type(CHILD1(n), type);
	break;
    case EXPR_BINOP:
	switch (CHILD1(n)->species) {
	case BINOP_PLUS:
	    expr_type(CHILD0(n), &op1);
	    expr_type(CHILD2(n), &op2);
	    if (QUAL_ISPTR(op1.qual) || QUAL_ISARRAY(op1.qual)) {
		/* should check that the other is simple int */
		copy_type(&op1, type);
		break;
	    }
	    if (QUAL_ISPTR(op2.qual) || QUAL_ISARRAY(op2.qual)) {
		copy_type(&op2, type);
		break;
	    }
	    usual_arith_conv(&op1, &op2, type);
	    break;
	case BINOP_MINUS:
	    expr_type(CHILD0(n), &op1);
	    expr_type(CHILD2(n), &op2);
	    if (QUAL_ISPTR(op1.qual) || QUAL_ISARRAY(op1.qual)) {
		if (QUAL_ISPTR(op2.qual) || QUAL_ISARRAY(op2.qual)) {
		    clear_type(type);
		    type->base = BT_INT;
		} else
		    copy_type(&op1, type);
	    } else if (QUAL_ISPTR(op2.qual) || QUAL_ISARRAY(op2.qual)) {
		copy_type(&op2, type);
		break;
	    } else {
		usual_arith_conv(&op1, &op2, type);
	    }
	    break;
	case BINOP_MUL:
	case BINOP_DIV:
	case BINOP_MOD:
	case BINOP_AND:
	case BINOP_OR:
	case BINOP_ER:
	    expr_type(CHILD0(n), &op1);
	    expr_type(CHILD2(n), &op2);
	    usual_arith_conv(&op1, &op2, type);
	    break;
	case BINOP_LS:
	case BINOP_RS:
	    expr_type(CHILD0(n), type);
	    int_promote(type);
	    break;
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
	    expr_type(CHILD0(n), type);
	    if (type->base & BTB_INT)
		int_promote(type);
	    break;
	case BINOP_ANDAND:
	case BINOP_OROR:
	case BINOP_EQ:
	case BINOP_GTEQ:
	case BINOP_LTEQ:
	case BINOP_NEQ:
	case BINOP_LT:
	case BINOP_GT:
	    clear_type(type);
	    type->base = BT_INT;
	    break;
	default:
	    clear_type(type);
	    type->base = BT_UNKNOWN;
	    break;
	}
	break;
    case EXPR_UNOP:
	switch (CHILD0(n)->species) {
	case UNOP_AND:
	    expr_type(CHILD1(n), type);
	    if (QUAL_ISARRAY(type->qual))
		semantic_error(n->srcpos,
			       "address of array ignored");
	    else
		type->qual = (type->qual << QUAL_SHIFT) | QUAL_PTR;
	    break;
	case UNOP_MINUS:
	    expr_type(CHILD1(n), type);
	    if (type->base & BTB_INT)
		int_promote(type);
	    break;
	case UNOP_INC:
	case UNOP_DEC:
	    expr_type(CHILD1(n), type);
	    break;
	case UNOP_NOT:
	    clear_type(type);
	    type->base = BT_INT;
	    break;
	case UNOP_COMPL:
	    expr_type(CHILD1(n), type);
	    int_promote(type);
	    break;
	default:
	    clear_type(type);
	    type->base = BT_UNKNOWN;
	    break;
	}
	break;
    case EXPR_INCOP:
	expr_type(CHILD0(n), type);
	break;
    case EXPR_LSTAR:
	expr_type(CHILD1(n), type);
	p = CHILD0(n);
	for (p = CHILD0(p); p != NULL; p = TNEXT(p))
	    if (QUAL_ISPTR(type->qual) || QUAL_ISARRAY(type->qual))
		type->qual >>= QUAL_SHIFT;
	    else
		semantic_error(p->srcpos, "type error");
	break;
    case EXPR_LARRAY:
	expr_type(CHILD0(n), type);
	if (QUAL_ISPTR(type->qual) || QUAL_ISARRAY(type->qual))
	    type->qual >>= QUAL_SHIFT;
	else
	    semantic_error(n->srcpos, "type error");
	break;
    case EXPR_LARROW:
    case EXPR_LDOT:
	sym = CHILD1(n)->sym.sym;
	if (sym)
	    copy_type(&sym->type.valtype, type);
	else {
	    clear_type(type);
	    type->base = BT_UNKNOWN;
	}
	break;
    case EXPR_LFCALL1:
    case EXPR_LFCALL0:
	p = CHILD0(n);
	if (p->species == FUNC_EXPR_LP)
	    expr_type(CHILD0(p), type);
	else {
	    sym = CHILD0(p)->sym.sym;
	    if (sym)
		copy_type(&sym->type.valtype, type);
	    else {
		clear_type(type);
		type->base = BT_UNKNOWN;
	    }
	}
	while (QUAL_ISPTR(type->qual) || QUAL_ISARRAY(type->qual))
	    type->qual >>= QUAL_SHIFT;
	if (QUAL_ISFUNC(type->qual))
	    type->qual >>= QUAL_SHIFT;
	else
	    semantic_error(n->srcpos, "type error");
	break;
    case EXPR_CAST:
	p = CHILD0(n);
	if (p->sym.sym)		/* Casts are resolved in tree. */
	    copy_type(&p->sym.sym->type.valtype, type);
	else {			/* shouldn't happen */
	    clear_type(type);
	    type->base = BT_UNKNOWN;
	}
	break;
    case EXPR_SIZEOF:
#ifdef MVS
    case EXPR_OFFSET:
#endif
    case EXPR_SIZEOF_TYPE:
	clear_type(type);
	type->base = BT_INT;
	break;
    case EXPR_LNAME:
	sym = CHILD0(n)->sym.sym;
	if (sym && sym->nametype == VALSYM)
	    copy_type(&sym->type.valtype, type);
	else if (sym && sym->nametype == MEM_ENUM) {
	    clear_type(type);
	    type->base = BT_INT;
	} else {
	    clear_type(type);
	    type->base = BT_UNKNOWN;
	}
	break;
    case EXPR_ICON:
	clear_type(type);
	type->base = BT_INT;
	/* ?unknown? L and U suffixes should be considered as well as value size. */
	break;
    case EXPR_FCON:
	clear_type(type);
	type->base = BT_DOUBLE;
	/* ?unknown? L and F suffixes should be considered for ANSI. */
	break;
    case EXPR_STRING:
	clear_type(type);
	type->base = BT_CHAR;
	type->qual = QUAL_PTR;
	break;
    case EXPR_INHERIT:
	expr_type(CHILD0(n), type);
	break;
    default:
	clear_type(type);
	type->base = BT_UNKNOWN;
	break;
    }
}
