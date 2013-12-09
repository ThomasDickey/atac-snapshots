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

static const char print_sym_c[] = "$Id: print_sym.c,v 3.9 2013/12/09 00:20:22 tom Exp $";
/*
* @Log: print_sym.c,v @
* Revision 3.7  1997/05/11 23:03:24  tom
* correct gcc warnings, including an erroneous call on internal_error()
*
* Revision 3.6  1997/05/10 23:19:47  tom
* absorb srcpos.h into error.h
*
* Revision 3.5  1996/11/12 22:33:41  tom
* change ident to 'const' to quiet gcc
* add forward-ref prototypes
*
* Revision 3.4  1995/12/13 00:58:34  tom
* handle SCB_INLINE
*
* Revision 3.3  94/04/04  10:13:46  jrh
* Add Release Copyright
* 
* Revision 3.2  93/08/04  15:47:18  ewk
* Added MVS and solaris support.  Squelched some ANSI warnings.
* 
* Revision 3.1  93/07/12  11:01:25  saul
* MVS MODULEID
* 
* Revision 3.0  92/11/06  07:45:08  saul
* propagate to version 3.0
* 
* Revision 2.6  92/10/30  09:48:36  saul
* include portable.h
* 
* Revision 2.5  92/10/22  13:57:29  saul
* change qual and rqual to unsigned to prevent unterminated loop.
* 
* Revision 2.4  92/03/17  14:22:42  saul
* copyright
* 
* Revision 2.3  91/10/02  22:45:05  saul
* signed/unsigned confusion (courtesy of Bob Kayel).
* 
* Revision 2.2  91/06/13  12:52:38  saul
* Changes for printing storage class and types
* 
* Revision 2.1  91/06/13  12:39:15  saul
* Propagate to version 2.0
* 
* Revision 1.1  91/06/12  20:25:47  saul
* Aug 1990 baseline
* 
*-----------------------------------------------end of log
*/
#include <stdio.h>
#include "portable.h"
#include "error.h"
#include "tnode.h"
#include "sym.h"

/* forward declarations */
static int bit_align(FILE *f, SYM * sym, int indent, int alignment);
static void iprint_sym(FILE *f, SYM * sym, int indent, char *prefix);
static void print_rqual(FILE *f, char *name, unsigned long rqual, int
			needparen, DIMLIST * dim);
static void print_sclass(FILE *f, int sclass);
static void print_valtype(FILE *f, SYM * sym, char *prefix);

#define TAB(f,I)	do{int i = (I); while(i--) putc('\t', f);}while(0)

static void
print_rqual(FILE *f,
	    char *name,
	    unsigned long rqual,	/* reversed qual */
	    int needparen,
	    DIMLIST * dim)
{
    if (rqual == 0) {
	if (name)
	    fputs(name, f);
	else
	    fputs("?unknown?", f);
	return;
    }

    switch (rqual & QUAL_MASK) {
    case QUAL_PTR:
	if (needparen)
	    fputs("(", f);
	fputs("*", f);
	print_rqual(f, name, rqual >> QUAL_SHIFT, 0, dim);
	if (needparen)
	    fputs(")", f);
	break;
    case QUAL_FUNC:
	print_rqual(f, name, rqual >> QUAL_SHIFT, 1, dim);
	fputs("()", f);
	break;
    case QUAL_ARRAY:
	if (dim == NULL) {
	    print_rqual(f, name, rqual >> QUAL_SHIFT, 1, NULL);
	    fputs("[?unknown?]", f);
	} else {
	    print_rqual(f, name, rqual >> QUAL_SHIFT, 1, dim->next);
	    if (dim->size != -1)
		fprintf(f, "[%d]", dim->size);
	    else
		fputs("[]", f);
	}
	break;
    default:
	internal_error(NULL, "unknown type qualifier: %d, %s",
		       rqual & QUAL_MASK, name ? name : "");
	break;
    }
}

static void
print_sclass(FILE *f,
	     int sclass)
{
    if (sclass & SCB_TYPENAME)
	fputs("typedef ", f);
    if (sclass & SCB_PARAM)
	fputs("/* param */ ", f);
    if (sclass & SCB_STATIC)
	fputs("static /* local */", f);
    if (sclass & SCB_INLINE)
	fputs("inline /* extended */", f);
    if (sclass & SCB_GLOBAL)
	fputs("/* global */ ", f);
    if (sclass & SCB_FILEGLOBAL)
	fputs("static /* global */ ", f);
    if (sclass & SCB_EXTERN_REF)
	fputs("extern ", f);
    if (sclass & SCB_AUTO)
	fputs("/* auto */ ", f);
    if (sclass & SCB_REG)
	fputs("register ", f);
    if (sclass & SCB_VOLATILE)
	fputs("volitile ", f);
    if (sclass & SCB_CONST)
	fputs("const ", f);
    if (sclass & SCB_INIT)
	fputs("/* initialized */ ", f);
}

void
print_type(FILE *f,
	   VALTYPE * type,
	   const char *name,
	   const char *prefix)
{
    int base;
    unsigned int qual;
    unsigned int rqual;		/* reversed qual */

    base = type->base;

    switch (base) {
    case BT_UNKNOWN:
	fputs("/* unknown */ int ", f);
	break;
    case BT_VOID:
	fputs("void ", f);
	break;
    case BT_FLOAT:
	fputs("float ", f);
	break;
    case BT_DOUBLE:
	fputs("double ", f);
	break;
    case BT_LONGDOUBLE:
	fputs("long double ", f);
	break;
    case BT_STRUCT:
	if (type->tag && type->tag->name)
	    fprintf(f, "struct %s ",
		    type->tag->name);
	else
	    fprintf(f, "struct %s%p ", prefix ? prefix : "",
		    type->tag);
	break;
    case BT_UNION:
	if (type->tag && type->tag->name)
	    fprintf(f, "union %s ",
		    type->tag->name);
	else
	    fprintf(f, "union %s%p ", prefix ? prefix : "", type->tag);
	break;
    case BT_ENUM:
	if (type->tag && type->tag->name)
	    fprintf(f, "enum %s ",
		    type->tag->name);
	else
	    fprintf(f, "enum %s%p ", prefix ? prefix : "", type->tag);
	break;
    default:
	if (base & BTB_UNSIGNED)
	    fputs("unsigned ", f);
	if (base & BTB_SIGNED)
	    fputs("/* signed */ ", f);
	if (base & BTB_CHAR_SIZE)
	    fputs("char ", f);
	else {
	    if (base & BTB_SHORT_SIZE)
		fputs("short ", f);
	    if (base & BTB_LONG_SIZE)
		fputs("long ", f);
	    fputs("int ", f);
	}
	break;
    }

    if (type->bits >= 0) {
	if (name) {
	    fputs(name, f);
	    fputs(" ", f);
	}
	fprintf(f, ": %d", type->bits);
	return;
    }

    /*
     * Reverse qual into rqual for printing.
     */
    rqual = 0;
    for (qual = type->qual; qual != 0; qual >>= QUAL_SHIFT)
	rqual = (rqual << QUAL_SHIFT) | (qual & QUAL_MASK);
    print_rqual(f, name, rqual, 0, type->dimensions);
}

/*
* Print declaration for symbol of type VALSYM.  Omit trailing ';'.
*/
static void
print_valtype(FILE *f,
	      SYM * sym,
	      char *prefix)
{
    print_sclass(f, sym->type.valtype.sclass);
    print_type(f, &sym->type.valtype, sym->name, prefix);
}

/*
* Print unnamed bit fields to get field properly aligned given current
*	alignment.  Return alignment after this field.  Indent lines by
*	indent.
*/
static int
bit_align(FILE *f,
	  SYM * sym,
	  int indent,
	  int alignment)
{
    int bits;
    int bit_alignment;
    int cur_alignment;

    bits = sym->type.valtype.bits;
    if (bits == -1)
	return 0;

    cur_alignment = alignment;

    bit_alignment = sym->type.valtype.bit_alignment;
    if (bit_alignment < cur_alignment) {
	TAB(f, indent);
	fputs(" :0;\n", f);
	cur_alignment = 0;
    }
    if (bit_alignment > cur_alignment) {
	TAB(f, indent);
	fprintf(f, " :%d;\n",
		bit_alignment - cur_alignment);
	cur_alignment = bit_alignment;
    }
    return cur_alignment + bits;
}

/*
* Print declaration for symbol.  Indent by indent.  Omit trailing ';'.
*/
static void
iprint_sym(FILE *f,
	   SYM * sym,
	   int indent,
	   char *prefix)
{
    SYMLIST *p;
    int bit_alignment = 0;

    TAB(f, indent);

    switch (sym->nametype) {
    case STRUCT_TAG:
    case UNION_TAG:
	if (sym->nametype == STRUCT_TAG)
	    fputs("struct", f);
	else
	    fputs("union", f);
	if (sym->name) {
	    fputs(" ", f);
	    fputs(sym->name, f);
	}
	if (sym->type.struct_tag.fieldlist) {
	    fputs(" { \n", f);
	    for (p = sym->type.struct_tag.fieldlist;
		 p; p = p->next) {
		bit_alignment = bit_align(f, p->sym,
					  indent + 1, bit_alignment);
		TAB(f, indent + 1);
		print_valtype(f, p->sym, prefix);
		fputs(";\n", f);
	    }
	    TAB(f, indent);
	    fputs("}", f);
	}
	if (sym->name == NULL &&
	    sym->type.struct_tag.fieldlist == NULL) {
	    fputs(" ?unknown?", f);
	}
	break;
    case ENUM_TAG:
	fputs("enum", f);
	if (sym->name) {
	    fputs(" ", f);
	    fputs(sym->name, f);
	}
	if (sym->type.enum_tag.memlist) {
	    fputs(" { \n", f);
	    for (p = sym->type.struct_tag.fieldlist;
		 p; p = p->next) {
		TAB(f, indent + 1);
		if (p->sym->name)
		    fprintf(f, "%s,\n",
			    p->sym->name);
		else
		    fputs("?unknown?,\n", f);
	    }
	    TAB(f, indent);
	    fputs("}", f);
	} else
	    fputs(" ?unknown?", f);
	break;
    case MEM_ENUM:
	fputs("enum", f);
	if (sym->type.mem_enum.tag->name) {
	    fputs(" ", f);
	    fputs(sym->type.mem_enum.tag->name, f);
	}
	if (sym->name)
	    fprintf(f, " { ..., %s, ... }", sym->name);
	else
	    fputs(" { ..., ?unknown?, ... }", f);
	break;
    case VALSYM:
    case TYPE_NAME:
	print_valtype(f, sym, prefix);
	break;
    case LABELSYM:
	fprintf(f, "%s:", sym->name);
	break;
    default:
	internal_error((SRCPOS *) 0, "unknown symbol type: %d", sym->nametype);
	break;
    }
}

/*
* Print declaration for symbol.  Indent by indent.
*/
void
print_sym(SYM * sym,
	  char *prefix)
{
    FILE *f = stderr;

    iprint_sym(f, sym, 0, prefix);
    fputs(";\n", f);
}
