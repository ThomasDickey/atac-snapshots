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
 #pragma csect (CODE, "const$")
#include <mvapts.h>
MODULEID(%M%,%J%/%D%/%T%)
#include <string.h>
#include <stdlib.h>
#endif /* MVS */

static const char const_c[] = 
	"$Header: /users/source/archives/atac.vcs/atac_i/RCS/const.c,v 3.8 1997/05/10 23:14:09 tom Exp $";
/*
* $Log: const.c,v $
* Revision 3.8  1997/05/10 23:14:09  tom
* absorb srcpos.h into error.h
*
* Revision 3.7  1996/11/13 00:57:09  tom
* change ident to 'const' to quiet gcc
* add forward-ref prototypes
*
* Revision 3.6  94/04/04  10:12:06  jrh
* Add Release Copyright
*
* Revision 3.5  94/03/21  08:29:31  saul
* MVS support __offsetof as builtin (not handled by cpp)
*
* Revision 3.4  93/08/09  12:24:44  saul
* minor bug fixes, portability changes, test code
*
* Revision 3.3  1993/08/04  15:44:15  ewk
* Added MVS and solaris support.  Squelched some ANSI warnings.
*
* Revision 3.2  93/07/12  14:07:48  saul
* MVS \v portablility
*
* Revision 3.1  93/07/12  10:04:37  saul
* MVS MODULEID
*
* Revision 3.0  92/11/06  07:46:11  saul
* propagate to version 3.0
*
* Revision 2.3  92/11/04  15:59:58  saul
* avoid use of uninitialized memory (was benign)
*
* Revision 2.2  92/10/30  09:47:37  saul
* include portable.h
*
* Revision 2.1  92/07/10  13:36:14  saul
* new
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

#define HEX_DECODE(c) (							\
	((c) >= '0' && (c) <= '9') ? ((c) - '0') :			\
	((c) >= 'A' && (c) <= 'F') ? ((c) - 'A' + 10) : 		\
	((c) >= 'a' && (c) <= 'f') ? ((c) - 'a' + 10) : -1)
#define DEC_DECODE(c) (							\
	((c) >= '0' && (c) <= '9') ? ((c) - '0') : -1)

static int evalSizeof P_(( SYM *sym ));
static void evalIcon P_(( SRCPOS *srcpos, char *fulltext, CONST_VALUE *value ));

static int
evalSizeof(sym)
SYM	*sym;
{
	SYM	s;
	SYMLIST	*p;
	int	size;
	int	max;

	switch(sym->nametype)
	{
	case STRUCT_TAG:
		size = 0;
		for (p = sym->type.struct_tag.fieldlist; p; p = p->next)
			size += evalSizeof(p->sym);	/* ?unknown? doesn't do padding */
		return size;
	case UNION_TAG:
		max = 0;
		for (p = sym->type.struct_tag.fieldlist; p; p = p->next) {
			size = evalSizeof(p->sym);
			if (size > max) max = size;
		}
		return max;
	case TYPE_NAME:
	case VALSYM:
		if (QUAL_ISPTR(sym->type.valtype.qual))
			return sizeof(long);	/* ?unknown? not retargetable */
		if (QUAL_ISFUNC(sym->type.valtype.qual))
			return 0;
		if (QUAL_ISARRAY(sym->type.valtype.qual)) {
			if (sym->type.valtype.dimensions == NULL) {
			    /* 
			     * Can't happen.  Dimensions is always allocated
			     * when ISARRAY.
			     */
			    return 0;
			}
			s = *sym;
			s.type.valtype.qual >>= QUAL_SHIFT;
			s.type.valtype.dimensions =
				s.type.valtype.dimensions->next;
			/* ?unknown? padding between arrays ? */
			size = sym->type.valtype.dimensions->size;
			if (size == -1 || size == 0)
			    return 0;
			return size * evalSizeof(&s);
		}
		switch (sym->type.valtype.base)
		{
		case BT_CHAR:
		case BT_UCHAR:
			return 1;
		case BT_SHORT:
		case BT_USHORT:
			return sizeof(short);	/* ?unknown? not retargetable */
		case BT_INT:
		case BT_UINT:
			/* ?unknown? doesn't pack bit fields at all */
			return sizeof(int);	/* ?unknown? not retargetable */
		case BT_LONG:
		case BT_ULONG:
			return sizeof(long);	/* ?unknown? not retargetable */
		case BT_FLOAT:
			return sizeof(float);	/* ?unknown? not retargetable */
		case BT_DOUBLE:
			return sizeof(double);	/* ?unknown? not retargetable */
		case BT_LONGDOUBLE:
			return sizeof(double);	/* ?unknown? not retargetable */
		case BT_STRUCT:
		case BT_UNION:
		case BT_ENUM:
			return evalSizeof(sym->type.valtype.tag);
		case BT_VOID:
		case BT_UNKNOWN:
		default:
			return 0;
		}
	case MEM_ENUM:	/* Can't happen.  Expr reduced to int type. */
	case ENUM_TAG:
		return sizeof(int);	/* ?unknown? not retargetable */
	case NULL_SYM:
	case LABELSYM:
	default:
		return 0;
	}
}

static void
evalIcon(srcpos, fulltext, value) 
SRCPOS		*srcpos;
char		*fulltext;
CONST_VALUE	*value;
{
    int		v;
    char	*text;

    text = fulltext;	
    value->type = CONST_VT;
    if (text == NULL)
	internal_error(srcpos, "missing ICON text\n", 0, 0, 0);
    v = 0;
    switch (*text)
    {
    case '0':
	++text;
	if (*text == 'x' || *text == 'X') {
	    /*
	     * Hex
	     */
	    for (++text;;++text) {
		int hval = HEX_DECODE(*text);
		if (hval == -1) break;
		v = (v * 16) + hval;
	    }
	} else {
	    /*
	     * Octal (or 0)
	     */
	    for (;;++text) {
		int dval = DEC_DECODE(*text);
		if (dval == -1) break;
		v = (v * 8) + dval;
	    }
	}
	if (*text == 'L' || *text == 'l') {
	    ++text;
	    if (*text == 'U' || *text == 'u')
		++text;
	}
	else if (*text == 'U' || *text == 'u') {
	    ++text;
	    if (*text == 'L' || *text == 'l')
		++text;
	}
	break;
    case '1': case '2': case '3': case '4': case '5':
    case '6': case '7': case '8': case '9':
	/*
	 * Decimal
	 */
	while (1) {
	    int dval = DEC_DECODE(*text);
	    if (dval == -1) break;
	    v = (v * 10) + dval;
	    ++text;
	}
	if (*text == 'L' || *text == 'l') {
	    ++text;
	    if (*text == 'U' || *text == 'u')
		++text;
	}
	else if (*text == 'U' || *text == 'u') {
	    ++text;
	    if (*text == 'L' || *text == 'l')
		++text;
	}
	break;
    case 'L':
	++text;
	if (*text != '\'') {
	    lexical_error(srcpos, "bad wide character: \"%s\"\n",
			  fulltext, 0, 0);
	    text = "'\\0'";	/* dummy */
	}
	/* else fall through */
    case '\'':
	++text;
	v = 0;
	while (*text != '\'' && *text != '\0') {
	    int val = *text++;
	    /*
	     * The value of a multi-character constant
	     * is implementation defined according to ANSI.
	     * We attempt to treat it as a base n number
	     * where n is the greatest chacter value plus 1
	     * and each character represents its ascii
	     * (or EBCDIC) value.  (This is likely the
	     * implementation intended by the standard.)
	     * The size of the greatest character value is
	     * BYTE(~0).
	     */
	    if (val == '\\') switch (*text++)
	    {
	    case 'n':
		val = '\n';
		break;
	    case 't':
		val = '\t';
		break;
	    case 'v':
#ifdef __STDC__
		val = '\v';	/* ?unknown? not retargetable */
#else				/* not __STDC__ */
		val = '\013';	/* ?unknown? not portable */
#endif				/* not __STDC__ */
		break;
	    case 'b':
		val = '\b';
		break;
	    case 'r':
		val = '\r';
		break;
	    case 'f':
		val = '\f';
		break;
	    case 'a':
#ifdef __STDC__
		val = '\a';
#else				/* not __STDC__ */
		val = '\007';	/* ?unknown? not retargetable */
#endif				/* not __STDC__ */
		break;
	    case '\\':
		val = '\n';
		break;
	    case '?':
		val = '?';
		break;
	    case '\'':
		val = '\'';
		break;
	    case '"':
		val = '"';
		break;
	    case 'x':
		val = 0;
		while (1) {
		    int hval = HEX_DECODE(*text);
		    if (hval == -1) break;
		    val = (val * 16) + hval;
		    ++text;
		}
		break;
	    case '0': case '1': case '2': case '3': case '4':
	    case '5': case '6': case '7': case '8': case '9':
	    {
		/*
		 * In '\ooo', ooo must be exactly 1, 2, or
		 * 3 octal digits.  We allow 8, and 9 for
		 * compatability with K&R C.  If more digits
		 * follow, they are taken as separate additional
		 * characters as permitted by ANSI.
		 * e.g. '\0123' is a two character constant
		 * consisting of newline and '3'.
		 * ?unknown? Note '\09' is tab, not nul and '9'.
		 */
		int dval;
		--text;
		val = DEC_DECODE(*text); /* digit 1 */
		++text;
		dval = DEC_DECODE(*text); /* digit 2 */
		if (dval == -1) break;
		val = (val * 8) + dval;
		++text;
		dval = DEC_DECODE(*text); /* digit 3 */
		if (dval == -1) break;
		val = (val * 8) + dval;
		++text;
		break;
	    }
	    case '\0':
		lexical_error(srcpos, "bad character constant: \"%s\"\n",
			      fulltext, 0, 0);
		text = "'";	/* dummy */
		break;
	    default:
		/*
		 * Unknown escape.  Ignore.
		 */
		val = *text++;
		break;
	    }
	    v = (v * (BYTE(~0) + 1)) + val;
	}
	if (*text++ != '\'' || *text != '\0')
	    lexical_error(srcpos, "bad character constant: \"%s\"\n",
			  fulltext, 0, 0);
	break;
    default:
	lexical_error(srcpos, "bad character constant: \"%s\"\n",
		      fulltext, 0, 0);
    }
    value->fraction = v;
}

void
evalConstExpr(n, value)
TNODE		*n;			/* genus: GEN_EXPR */
CONST_VALUE	*value;
{
	CONST_VALUE	op1;
	CONST_VALUE	op2;
	SYM		sym;
	int		operator;

	if (n->genus == GEN_HOOK) {
		evalConstExpr(CHILD0(n), value);
		return;
	}

	if (n->genus != GEN_EXPR)
		internal_error(n->srcpos, "evalConstExpr: unexpected genus: %d",
			n->genus, 0, 0);

	value->type = CONST_VT_UNKNOWN;
	value->fraction = 0;

	switch(n->species)
	{
	case EXPR_QCOLON:
		/*
		* "E ? A : B"
		*/
		evalConstExpr(CHILD0(n), &op1);
		switch (op1.type)
		{
		case CONST_VT:
			if (op1.fraction)
				evalConstExpr(CHILD1(n), value);
			else evalConstExpr(CHILD2(n), value);
			break;
		case CONST_VT_NON_NULL_ADDRESS:
			evalConstExpr(CHILD1(n), value);
			break;
		case CONST_VT_UNDETERMINED:
		case CONST_VT_UNDEFINED:
		case CONST_VT_UNKNOWN:
			value->type = op1.type;
			break;
		default:
			internal_error(n->srcpos, "unknown value type %d\n",
				op1.type, 0, 0);
		}
		break;
	case EXPR_COMMA:
		evalConstExpr(CHILD1(n), value);
		break;
	case EXPR_BINOP:
		operator = CHILD1(n)->species;
		if (operator == BINOP_ASGN)
			op1.type = CONST_VT;
		else evalConstExpr(CHILD0(n), &op1);
		evalConstExpr(CHILD2(n), &op2);
		if (op1.type == CONST_VT_UNDEFINED ||
			op2.type == CONST_VT_UNDEFINED)
		{
			value->type = CONST_VT_UNDEFINED;
			break;
		}
		if (op1.type == CONST_VT_UNDETERMINED ||
			op2.type == CONST_VT_UNDETERMINED)
		{
			value->type = CONST_VT_UNDETERMINED;
			break;
		}
		if (op1.type == CONST_VT_UNKNOWN ||
			op2.type == CONST_VT_UNKNOWN)
		{
			value->type = CONST_VT_UNKNOWN;
			break;
		}
		if (op1.type == CONST_VT_NON_NULL_ADDRESS ||
			op2.type == CONST_VT_NON_NULL_ADDRESS)
		{
			value->type = CONST_VT_NON_NULL_ADDRESS;
			break;
		}
		value->type = CONST_VT;
		switch (operator)
		{
	        case BINOP_ASGN:
		        value->fraction = op2.fraction;
			break;
		case BINOP_PLUS:
		case BINOP_APLUS:
			clear_type(&sym.type.valtype);
			expr_type(CHILD0(n), &sym.type.valtype);
			if (QUAL_ISPTR(sym.type.valtype.qual)) {
				sym.type.valtype.qual >>= QUAL_SHIFT;
				sym.nametype = TYPE_NAME;
				value->fraction = op1.fraction +
					evalSizeof(&sym) * op2.fraction;
				break;
			}
			clear_type(&sym.type.valtype);
			expr_type(CHILD2(n), &sym.type.valtype);
			if (QUAL_ISPTR(sym.type.valtype.qual)) {
				sym.type.valtype.qual >>= QUAL_SHIFT;
				sym.nametype = TYPE_NAME;
				value->fraction = op2.fraction +
					evalSizeof(&sym) * op1.fraction;
				break;
			}
			value->fraction = op1.fraction + op2.fraction;
			break;
		case BINOP_MINUS:
		case BINOP_AMINUS:
			clear_type(&sym.type.valtype);
			expr_type(CHILD0(n), &sym.type.valtype);
			if (QUAL_ISPTR(sym.type.valtype.qual)) {
				sym.type.valtype.qual >>= QUAL_SHIFT;
				sym.nametype = TYPE_NAME;
				value->fraction = op1.fraction -
					evalSizeof(&sym) * op2.fraction;
				break;
			}
			clear_type(&sym.type.valtype);
			expr_type(CHILD2(n), &sym.type.valtype);
			if (QUAL_ISPTR(sym.type.valtype.qual)) {
				sym.type.valtype.qual >>= QUAL_SHIFT;
				sym.nametype = TYPE_NAME;
				value->fraction =
				    evalSizeof(&sym) * op1.fraction -
					op2.fraction;
				break;
			}
			value->fraction = op1.fraction - op2.fraction;
			break;
		case BINOP_MUL:
		case BINOP_AMUL:
			if (op1.type == CONST_VT && op1.fraction == 0)
				*value = op1;
			else if (op2.type == CONST_VT && op2.fraction == 0)
				*value = op2;
			else if (value->type == CONST_VT)
				value->fraction = op1.fraction * op2.fraction;
			break;
		case BINOP_DIV:
		case BINOP_ADIV:
			if (op1.type == CONST_VT && op1.fraction == 0)
				*value = op1;
			else if (op2.type == CONST_VT && op2.fraction == 0)
				value->type = CONST_VT_UNDEFINED;
			else if (value->type == CONST_VT)
				value->fraction = op1.fraction / op2.fraction;
			break;
		case BINOP_MOD:
		case BINOP_AMOD:
			if (op1.type == CONST_VT && op1.fraction == 0)
				*value = op1;
			else if (op2.type == CONST_VT && op2.fraction == 0)
				value->type = CONST_VT_UNDEFINED;
			else if (value->type == CONST_VT)
				value->fraction = op1.fraction % op2.fraction;
			break;
		case BINOP_AND:
		case BINOP_AAND:
			if (op1.type == CONST_VT && op1.fraction == 0)
				*value = op1;
			else if (op2.type == CONST_VT && op2.fraction == 0)
				*value = op2;
			else value->fraction = op1.fraction & op2.fraction;
			break;
		case BINOP_OR:
		case BINOP_AOR:
			if (op1.type == CONST_VT && ~op1.fraction == 0)
				*value = op1;
			else if (op2.type == CONST_VT && ~op2.fraction == 0)
				*value = op2;
			else value->fraction = op1.fraction | op2.fraction;
			break;
		case BINOP_ER:
		case BINOP_AER:
			value->fraction = op1.fraction ^ op2.fraction;
			break;
		case BINOP_LS:
		case BINOP_ALS:
			if (op1.type == CONST_VT && op1.fraction == 0)
				*value = op1;
			else if (op2.type == CONST_VT && op2.fraction < 0)
				value->type = CONST_VT_UNDEFINED;
			else if (value->type == CONST_VT)
				value->fraction = op1.fraction << op2.fraction;
			break;
		case BINOP_RS:
		case BINOP_ARS:
			if (op1.type == CONST_VT && op1.fraction == 0)
				*value = op1;
			else if (op2.type == CONST_VT && op2.fraction < 0)
				value->type = CONST_VT_UNDEFINED;
			else if (value->type == CONST_VT)
				value->fraction = op1.fraction >> op2.fraction;
			break;
		case BINOP_ANDAND:
			if (op1.type == CONST_VT && op1.fraction == 0)
				value->type = CONST_VT;
			else if (op2.type == CONST_VT && op2.fraction == 0)
				value->type = CONST_VT;
			value->fraction = op1.fraction && op2.fraction;
			break;
		case BINOP_OROR:
			if (op1.type == CONST_VT && op1.fraction)
				value->type = CONST_VT;
			else if (op2.type == CONST_VT && op2.fraction)
				value->type = CONST_VT;
			value->fraction = op1.fraction || op2.fraction;
			break;
		case BINOP_EQ:
			value->fraction = op1.fraction == op2.fraction;
			break;
		case BINOP_GTEQ:
			value->fraction = op1.fraction >= op2.fraction;
			break;
		case BINOP_LTEQ:
			value->fraction = op1.fraction <= op2.fraction;
			break;
		case BINOP_NEQ:
			value->fraction = op1.fraction != op2.fraction;
			break;
		case BINOP_LT:
			value->fraction = op1.fraction < op2.fraction;
			break;
		case BINOP_GT:
			value->fraction = op1.fraction > op2.fraction;
			break;
		default:
			internal_error(CHILD1(n)->srcpos, "bad operator %d\n",
				operator, 0, 0);
		}
		break;
	case EXPR_UNOP:
		evalConstExpr(CHILD1(n), value);
		switch(CHILD0(n)->species)
		{
		case UNOP_AND:
			/* ?unknown? NON_NULL_ADDRESS ? */
			value->type = CONST_VT_UNKNOWN;
			break;
		case UNOP_MINUS:
			value->fraction = - value->fraction;
			break;
		case UNOP_INC:
			clear_type(&sym.type.valtype);
			expr_type(CHILD1(n), &sym.type.valtype);
			if (QUAL_ISPTR(sym.type.valtype.qual)) {
				sym.type.valtype.qual >>= QUAL_SHIFT;
				sym.nametype = TYPE_NAME;
				value->fraction += evalSizeof(&sym);
			} else value->fraction++;
			break;
		case UNOP_DEC:
			clear_type(&sym.type.valtype);
			expr_type(CHILD1(n), &sym.type.valtype);
			if (QUAL_ISPTR(sym.type.valtype.qual)) {
				sym.type.valtype.qual >>= QUAL_SHIFT;
				sym.nametype = TYPE_NAME;
				value->fraction -= evalSizeof(&sym);
			} else value->fraction--;
		case UNOP_NOT:
			value->fraction = !value->fraction;
			break;
		case UNOP_COMPL:
			value->fraction = ~value->fraction;
			break;
		default:
			internal_error(CHILD0(n)->srcpos, "bad uoperator %d\n",
				CHILD0(n)->species, 0, 0);
		}
		break;
	case EXPR_INCOP:
		evalConstExpr(CHILD0(n), value);
		switch(CHILD1(n)->species)
		{
		case INCOP_INC:
			clear_type(&sym.type.valtype);
			expr_type(CHILD0(n), &sym.type.valtype);
			if (QUAL_ISPTR(sym.type.valtype.qual)) {
				sym.type.valtype.qual >>= QUAL_SHIFT;
				sym.nametype = TYPE_NAME;
				value->fraction += evalSizeof(&sym);
			} else value->fraction++;
			break;
		case INCOP_DEC:
			clear_type(&sym.type.valtype);
			expr_type(CHILD0(n), &sym.type.valtype);
			if (QUAL_ISPTR(sym.type.valtype.qual)) {
				sym.type.valtype.qual >>= QUAL_SHIFT;
				sym.nametype = TYPE_NAME;
				value->fraction -= evalSizeof(&sym);
			} else value->fraction--;
			break;
		default:
			internal_error(CHILD1(n)->srcpos,"bad incoperator %d\n",
				CHILD1(n)->species, 0, 0);
		}
		break;
	case EXPR_LSTAR:
		/* ?unknown? Any chance of doing better ? */
		value->type = CONST_VT_UNKNOWN;
		break;
	case EXPR_LARRAY:
		/* ?unknown? Any chance of doing better ? */
		value->type = CONST_VT_UNKNOWN;
		break;
	case EXPR_LARROW:
	case EXPR_LDOT:
		/* ?unknown? Any chance of doing better ? */
		value->type = CONST_VT_UNKNOWN;
		break;
	case EXPR_LFCALL1:
	case EXPR_LFCALL0:
		value->type = CONST_VT_UNDETERMINED;
		break;
	case EXPR_CAST:
		evalConstExpr(CHILD1(n), value);
		break;
	case EXPR_SIZEOF:
		value->type = CONST_VT;
		clear_type(&sym.type.valtype);
		expr_type(CHILD0(n), &sym.type.valtype);
		sym.nametype = TYPE_NAME;
		value->fraction = evalSizeof(&sym);
		break;
	case EXPR_SIZEOF_TYPE:
		value->type = CONST_VT;
		if (CHILD0(n)->sym.sym == NULL)
			value->type = CONST_VT_UNKNOWN;
		else value->fraction = evalSizeof(CHILD0(n)->sym.sym);
		break;
#ifdef MVS
	case EXPR_OFFSET:
		/* ? compute offset value */
		value->type = CONST_VT_UNKNOWN;
		break;
#endif /* MVS */
	case EXPR_LNAME:
		{
		    SYM	*sym2;

		    sym2 = CHILD0(n)->sym.sym;
		    if (sym2 && sym2->constValue)
			*value = *sym2->constValue;
		    else value->type = CONST_VT_UNDETERMINED;
		}
		break;
	case EXPR_ICON:
		evalIcon(CHILD0(n)->srcpos, CHILD0(n)->text, value);
		break;
	case EXPR_FCON:
		/* ?unknown? compute float const value */
		value->type = CONST_VT_UNKNOWN;
		break;
	case EXPR_STRING:
		value->type = CONST_VT_NON_NULL_ADDRESS;
		break;
	case EXPR_INHERIT:
		evalConstExpr(CHILD0(n), value);
		break;
	default:
		internal_error(n->srcpos, "unknown EXPR species: \"%d\"\n",
			n->species);
	}
}

int
evalIConstExpr(node)
TNODE	*node;				/* genus: GEN_EXPR */
{
    CONST_VALUE	value;

    evalConstExpr(node, &value);

    if (value.type == CONST_VT)
    	return value.fraction;
    else return 0;
}

void
testConst()
{
    CONST_VALUE	value;
    static SRCPOS	srcpos[2] = { {-1, 0, 0}, {-1, 0, 0} };

    evalIcon(srcpos, "L", &value);
    evalIcon(srcpos, "'\\", &value);
    evalIcon(srcpos, "'", &value);
    evalIcon(srcpos, "'x'junk", &value);
    evalIcon(srcpos, "junk", &value);
}
