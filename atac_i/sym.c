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
#pragma csect (CODE, "sym$")
#include <mvapts.h>
MODULEID(%M%,%J%/%D%/%T%)
#endif /* MVS */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "portable.h"
#include "error.h"
#include "tnode.h"
#include "reglist.h"
#include "sym.h"
#include "tree.h"

static char const sym_c[] = "$Id: sym.c,v 3.14 2013/12/08 18:49:52 tom Exp $";
/*
* @Log: sym.c,v @
* Revision 3.13  2008/12/17 01:00:11  tom
* convert to ANSI, indent'd.  Use childX() functions rather than CHILDx()
* macros to quiet gcc 4.3.2 warnings about reuse of macro_n in parameters.
*
* Revision 3.12  1997/05/11 23:57:58  tom
* move prototypes to tnode.h
* rename sym() to do_sym() to avoid variable-shadowing warnings
*
* Revision 3.11  1997/05/11 23:09:18  tom
* include reglist.h
*
* Revision 3.10  1997/05/10 23:14:40  tom
* absorb srcpos.h into error.h
*
* Revision 3.9  1996/11/13 00:13:21  tom
* change ident to 'const' to quiet gcc
* add forward-ref prototypes
*
* Revision 3.8  1995/12/27 23:08:44  tom
* handle SCB_INLINE, CLASSTYPE_INLINE
*
* Revision 3.7  94/06/01  09:02:45  saul
* fix for ANSI f(...) 
* 
* Revision 3.6  94/04/04  10:14:36  jrh
* Add Release Copyright
* 
* Revision 3.5  93/11/19  12:14:31  saul
* MVS support for _Packed
* 
* Revision 3.4  93/08/09  12:32:37  saul
* bug fix, iconst.c dropped, sign dropped from value structure
* 
* Revision 3.3  1993/08/04  15:48:08  ewk
* Added MVS and solaris support.  Squelched some ANSI warnings.
*
* Revision 3.2  93/07/12  11:39:47  saul
* MVS MODULEID
* MVS reglist==>reglst for uniqueness
* 
* Revision 3.1  93/06/22  12:57:18  saul
* Remove "Warning: %s redefined" from parser
* 
* Revision 3.0  92/11/06  07:45:10  saul
* propagate to version 3.0
* 
* Revision 2.11  92/11/04  15:57:09  saul
* fixed srcpos in "too many qualifiers" msg.  fixed dump_sym bug
* 
* Revision 2.10  92/11/02  11:36:35  saul
* remove unused variables
* 
* Revision 2.9  92/10/30  09:48:53  saul
* include portable.h
* 
* Revision 2.8  92/10/27  13:09:49  saul
* fix qual check semantic error problem.
* remove enums for portability
* 
* Revision 2.7  92/07/10  12:58:37  saul
* Added stuff to evaluate and bind constants; pointers back to def nodes
* 
* Revision 2.6  92/06/11  13:44:40  saul
* changes for unique prefix
* 
* Revision 2.5  92/05/08  08:08:44  saul
* handle ANSI outer scope struct tag undeclaration
* 
* Revision 2.4  92/03/17  14:22:58  saul
* copyright
* 
* Revision 2.3  91/10/23  12:30:27  saul
* Allow const in structure as in struct{const *field};
* Warnings for redundant typedef struct union enum
* Allow empty declaration list as in "int;"
* 
* Revision 2.2  91/06/13  13:00:43  saul
* lots of changes for ansi declarations
* 
* Revision 2.1  91/06/13  12:39:23  saul
* Propagate to version 2.0
* 
* Revision 1.1  91/06/12  20:25:52  saul
* Aug 1990 baseline
* 
*-----------------------------------------------end of log
*/

#define CHECK_MALLOC(p) ((p)?1:internal_error(NULL, "Out of memory"))

/*
* SymType indicates the context of the type specification.  It is used
* by checkSClass() (called by getClassTypes()) to modify storage class.
* SYM_TYPE_NOSPEC is used when there are no data items listed for
* the type specification (e.g. "struct foo;" which, in ANSI, means don't
* associate struct foo with its declaration in an outer scope).
*/
typedef int symType_t;
#define SYM_TYPE_GLOBAL		((symType_t) 0)
#define SYM_TYPE_LOCAL		((symType_t) 1)
#define SYM_TYPE_SU_MEMBER	((symType_t) 2)
#define SYM_TYPE_CAST		((symType_t) 3)
#define SYM_TYPE_PARAM		((symType_t) 4)
#define SYM_TYPE_FUNC		((symType_t) 5)
#define SYM_TYPE_NOSPEC		((symType_t) 6)

/* forward declarations */
/* *INDENT-OFF* */
static DIMLIST *copy_dimlist ( DIMLIST *list );
static SYM *dcl_enum ( TNODE *node, SYMTABLIST *symtablist );
static SYM *dcl_struct ( TNODE *node, SYMTABLIST *symtablist );
static SYM *mk_label ( TNODE	*nameNode );
static SYM *mk_moesym ( TNODE *nameNode, SYM *tag, int value );
static SYM *mk_tagsym ( NAMETYPE nametype );
static SYM *mk_valsym ( VALTYPE *type, NAMETYPE nametype );
static SYM *ref_enum ( TNODE *node, SYMTABLIST *symtablist );
static SYM *ref_struct ( TNODE *node, symType_t symType, SYMTABLIST *symtablist );
static SYM *ssym_find ( char *name, NAMETYPE nametype1, NAMETYPE nametype2, SYMTABLIST *symtablist );
static SYMLIST *copy_symlist ( SYMLIST *list );
static int checkBaseType ( int base, SRCPOS *srcpos );
static int checkSClass ( int sclass, symType_t symType, SRCPOS *srcpos );
static int count_stars ( TNODE *node );
static int isVoid ( TNODE *node );
static void ansiParam ( TNODE *node, SYMTABLIST *symtablist );
static void dcl ( TNODE *node, SYMTABLIST *symtablist );
static void dcl_all ( TNODE *node, SYMTABLIST *symtablist, SYMLIST **label_tab );
static void dcl_data_item ( TNODE *node, SYM *sym, SYMTABLIST *symtablist);
static void dcl_data_specs ( TNODE *node, VALTYPE *type, SYMTABLIST *symtablist );
static void dcl_formals ( TNODE *node, SYMLIST **symtab );
static void dcl_function ( TNODE *node, SYMTABLIST *symtablist );
static void dcl_label ( TNODE *node, SYMLIST **label_tab, int definition );
static void dcl_param ( TNODE *node, SYMTABLIST *symtablist );
static void default_params ( SYMLIST	**symtab );
static void getClassTypes ( TNODE *node, symType_t symType, VALTYPE *type, SYMTABLIST *symtablist );
static void get_null_dcl ( TNODE *node, SYM *sym );
static void sym_insert ( SYM *sym, SYMLIST **symtab );
/* *INDENT-ON* */

static SYMLIST *
copy_symlist(SYMLIST * list)
{
    SYMLIST *old;
    SYMLIST *p;
    SYMLIST newhead;

    p = &newhead;

    for (old = list; old != NULL; old = old->next) {
	p->next = (SYMLIST *) malloc(sizeof *p->next);
	CHECK_MALLOC(p->next);
	p = p->next;
	p->sym = old->sym;
    }

    p->next = NULL;
    return newhead.next;
}

static DIMLIST *
copy_dimlist(DIMLIST * list)
{
    DIMLIST newhead;
    DIMLIST *old;
    DIMLIST *p;

    p = &newhead;

    for (old = list; old != NULL; old = old->next) {
	p->next = (DIMLIST *) malloc(sizeof *p->next);
	CHECK_MALLOC(p->next);
	p = p->next;
	p->size = old->size;
    }

    p->next = NULL;
    return newhead.next;
}

/*
* mk_valsym:  Create a new symbol entry of nametype VALSYM or TYPE_NAME,
*	and given type.  Name, def, ref, bitfield, and qual info must be
*	filled in by caller.
*?unknown? Maybe VALSYM and TYPE_NAME should both be VALSYM.  TYPE_NAMES have
*sclass with SCB_TYPEDEF.
* 
*/
static SYM *
mk_valsym(VALTYPE * type,
	  NAMETYPE nametype)
{
    SYM *sym;

    sym = (SYM *) malloc(sizeof *sym);
    CHECK_MALLOC(sym);

    sym->name = NULL;
    sym->def = NULL;
    sym->ref = NULL;
    sym->nametype = nametype;
    sym->type.valtype.sclass = type->sclass;
    sym->type.valtype.base = type->base;
    sym->type.valtype.bits = -1;
    sym->type.valtype.bit_alignment = 0;
    sym->type.valtype.qual = type->qual;
    if (type->param_list)
	sym->type.valtype.param_list = copy_symlist(type->param_list);
    else
	sym->type.valtype.param_list = NULL;
    sym->type.valtype.tag = type->tag;
    if (type->dimensions)
	sym->type.valtype.dimensions = copy_dimlist(type->dimensions);
    else
	sym->type.valtype.dimensions = NULL;
    sym->constValue = NULL;

    return sym;
}

/*
* mk_tagsym:  Create a new symbol entry of nametype STRUCT_TAG, UNION_TAG,
*	or ENUM_TAG.  Name, ref, def, fieldlist, and memlist are set to NULL.
*/
static SYM *
mk_tagsym(NAMETYPE nametype)
{
    SYM *sym;

    sym = (SYM *) malloc(sizeof *sym);
    CHECK_MALLOC(sym);

    sym->name = NULL;
    sym->def = NULL;
    sym->ref = NULL;
    sym->nametype = nametype;
    switch (nametype) {
    case STRUCT_TAG:
    case UNION_TAG:
	sym->type.struct_tag.fieldlist = NULL;
	break;
    case ENUM_TAG:
	sym->type.enum_tag.memlist = NULL;
	break;
    }
    sym->constValue = NULL;

    return sym;
}

/*
* mk_moesym:  Create a new symbol entry of nametype MEM_ENUM with given tag.
* Name and def are from nameNode.  Ref is set to NULL.
*/
static SYM *
mk_moesym(TNODE * nameNode,
	  SYM * tag,
	  int value)
{
    SYM *sym;

    sym = (SYM *) malloc(sizeof *sym);
    CHECK_MALLOC(sym);

    sym->name = nameNode->text;
    sym->def = nameNode;
    sym->nametype = MEM_ENUM;
    sym->type.mem_enum.tag = tag;
    sym->def = NULL;
    sym->ref = NULL;
    sym->constValue = (CONST_VALUE *) malloc(sizeof *sym->constValue);
    CHECK_MALLOC(sym->constValue);
    sym->constValue->type = CONST_VT;
    sym->constValue->fraction = value;

    return sym;
}

/*
* mk_label:  Create a new symbol entry of nametype LABELSYM.  Name and def
* are from nameNode.  Ref is set to NULL.
*/
static SYM *
mk_label(TNODE * nameNode)
{
    SYM *sym;

    sym = (SYM *) malloc(sizeof *sym);
    CHECK_MALLOC(sym);

    sym->name = nameNode->text;
    sym->def = nameNode;
    sym->nametype = LABELSYM;
    sym->def = NULL;
    sym->ref = NULL;
    sym->type.label = NULL;
    sym->constValue = NULL;

    return sym;
}

/*
* sym_insert:  Put symbol at front of symtab.  Print warning for duplicate name.
*	(Struct, union, and enum tags each have a separate name space.)
*/
static void
sym_insert(SYM * sym,
	   SYMLIST ** symtab)
{
    SYMLIST *p;
    SYMLIST *new;

    new = (SYMLIST *) malloc(sizeof *new);
    CHECK_MALLOC(new);
    new->sym = sym;

    if (sym->name == NULL) {	/* untagged struct/union/enum */
	new->next = *symtab;
	*symtab = new;
	return;
    }

    for (p = *symtab; p != NULL; p = p->next) {
	if (sym->name != p->sym->name)
	    continue;
	if (sym->nametype == p->sym->nametype) {
	    if (sym->nametype != VALSYM)
		break;
	    /*
	     * Permit forward or backward references to functions
	     * or globals.
	     */
/* ?unknown? should check that the types don't conflict and then merge into one sym */
	    if (sym->type.valtype.sclass & SCB_EXTERN_REF)
		continue;
	    if (p->sym->type.valtype.sclass & SCB_EXTERN_REF)
		continue;
/* ?unknown? do TYPE_NAME and VALSYM share a name space? */
	    break;
	}
	if ((sym->nametype == VALSYM) && (p->sym->nametype == MEM_ENUM))
	    break;
	if ((sym->nametype == MEM_ENUM) && (p->sym->nametype == VALSYM))
	    break;
    }

/*	if (p) semantic_error(NULL, "Warning: %s redefined", sym->name); */

    new->next = *symtab;
    *symtab = new;
}

/*
* sym_find:  Find symbol with given name and nametype in given symtab.
*/
SYM *
sym_find(char *name,
	 NAMETYPE nametype,
	 SYMLIST * symtab)
{
    SYMLIST *p;

    for (p = symtab; p != NULL; p = p->next)
	if (p->sym->name == name && p->sym->nametype == nametype)
	    return p->sym;
    return NULL;
}

/*
* ssym_find:  Find first symbol with given name and either given nametype, 
* in any symtab in symtablist.
*/
static SYM *
ssym_find(char *name,
	  NAMETYPE nametype1,
	  NAMETYPE nametype2,
	  SYMTABLIST * symtablist)
{
    SYMTABLIST *p;
    SYM *sym;

    for (p = symtablist; p != NULL; p = p->next) {
	sym = sym_find(name, nametype1, p->symtab);
	if (sym)
	    return sym;
	if (nametype2 != NULL_SYM) {
	    sym = sym_find(name, nametype2, p->symtab);
	    if (sym)
		return sym;
	}
    }

    return NULL;
}

static int
count_stars(TNODE * node)	/* genus: GEN_STARS */
{
    TNODE *p;
    int n;

    if (node->genus != GEN_STARS) {
	internal_error(node->srcpos, "Unexpected genus: %d",
		       node->genus);
    }

    n = 0;
    for (p = child0(node); p != NULL; p = TNEXT(p))
	++n;

    return n;
}

/*
* dcl_data_item: Update sym with name and qualifiers from data_item node.
*	Sym assumed to already contain base type (and possible qualifier
*	info from typedef).
*/
static void
dcl_data_item(TNODE * node,	/* genus GEN_FUNC_SPEC or GEN_DATA_ITEM */
	      SYM * sym,
	      SYMTABLIST * symtablist)
{
    unsigned long q;
    DIMLIST *dim;
    int n;

    if (node->genus != GEN_DATA_ITEM && node->genus != GEN_FUNC_SPEC) {
	internal_error(node->srcpos, "Unexpected genus: %d",
		       node->genus);
    }

    switch (node->species) {
    case FUNC_STARS_SPEC:
	n = count_stars(child0(node));
	q = sym->type.valtype.qual;
	while (n--) {
	    if (QUAL_OVERFLOW(q))
		semantic_error(node->srcpos, "too many qualifiers");
	    q = (q << QUAL_SHIFT) | QUAL_PTR;
	}
	sym->type.valtype.qual = q;
	dcl_data_item(child1(node), sym, symtablist);
	break;
    case FUNC_SPEC_NFCALL:
    case FUNC_SPEC_ANSI:
    case FUNC_SPEC_ANSI_E:
    case FUNC_SPEC_E_ANSI:
	q = sym->type.valtype.qual;
	if (QUAL_OVERFLOW(q))
	    semantic_error(node->srcpos, "too many qualifiers");
	sym->type.valtype.qual = (q << QUAL_SHIFT) | QUAL_FUNC;
	dcl_data_item(child0(node), sym, symtablist);
	break;
    case FUNC_SPEC_ARRAY_EXPR:
	dcl_all(child1(node), symtablist, NULL);
	q = sym->type.valtype.qual;
	if (q == 0 && sym->type.valtype.sclass != SCB_PARAM) {
	    /*
	     * The primary qualifier is ARRAY.  Arrays have constant
	     * value (unless it is a parameter) which is their address.
	     * The value is never NULL.
	     */
	    sym->constValue = (CONST_VALUE *) malloc(
							sizeof *sym->constValue);
	    CHECK_MALLOC(sym->constValue);
	    sym->constValue->type = CONST_VT_NON_NULL_ADDRESS;
	    sym->constValue->fraction = 0;
	} else if (QUAL_OVERFLOW(q))
	    semantic_error(node->srcpos, "too many qualifiers");
	sym->type.valtype.qual = (q << QUAL_SHIFT) | QUAL_ARRAY;
	dim = (DIMLIST *) malloc(sizeof *dim);
	CHECK_MALLOC(dim);
	dim->next = sym->type.valtype.dimensions;
	sym->type.valtype.dimensions = dim;
	dim->size = evalIConstExpr(child1(node));
	dcl_data_item(child0(node), sym, symtablist);
	break;
    case FUNC_SPEC_ARRAY:
	q = sym->type.valtype.qual;
	if (q == 0 && sym->type.valtype.sclass != SCB_PARAM) {
	    /*
	     * The primary qualifier is ARRAY.  Arrays have constant
	     * value (unless it is a parameter) which is their address.
	     * The value is never NULL.
	     */
	    sym->constValue = (CONST_VALUE *) malloc(
							sizeof *sym->constValue);
	    CHECK_MALLOC(sym->constValue);
	    sym->constValue->type = CONST_VT_NON_NULL_ADDRESS;
	    sym->constValue->fraction = 0;
	} else if (QUAL_OVERFLOW(q))
	    semantic_error(node->srcpos, "too many qualifiers");
	sym->type.valtype.qual = (q << QUAL_SHIFT) | QUAL_ARRAY;
	dim = (DIMLIST *) malloc(sizeof *dim);
	CHECK_MALLOC(dim);
	dim->next = sym->type.valtype.dimensions;
	sym->type.valtype.dimensions = dim;
	dim->size = -1;
	dcl_data_item(child0(node), sym, symtablist);
	break;
    case FUNC_SPEC_INHERIT:
	dcl_data_item(child0(node), sym, symtablist);
	break;
    case FUNC_FCALL_NAMES:
    case FUNC_FCALL:
    case FUNC_FCALL_ANSI:
    case FUNC_FCALL_ANSI_E:
    case FUNC_FCALL_E_ANSI:
	q = sym->type.valtype.qual;
	if (QUAL_OVERFLOW(q))
	    semantic_error(node->srcpos, "too many qualifiers");
	sym->type.valtype.qual = (q << QUAL_SHIFT) | QUAL_FUNC;
	sym->name = child0(node)->text;
	sym->def = child0(node);
	child0(node)->sym.sym = sym;
	break;
    case DATA_NAME:
	sym->name = child0(node)->text;
	sym->def = child0(node);
	child0(node)->sym.sym = sym;
	break;
    }
    return;
}

/*
* get_null_dcl: Update sym with qualifiers from NULL_DCL node.
*	Sym assumed to already contain base type.
*/
static void
get_null_dcl(TNODE * node,	/* genus GEN_NULL_DCL */
	     SYM * sym)
{
    unsigned long q;
    DIMLIST *dim;

    if (node->genus != GEN_NULL_DCL) {
	internal_error(node->srcpos, "Unexpected genus: %d",
		       node->genus);
    }

/*?unknown? Are the recursions out of order here? (See dcl_data_item()). */
    switch (node->species) {
    case NULL_N_FUNC:
    case NULL_ANSI:
    case NULL_ANSI_E:
    case NULL_E_ANSI:
	get_null_dcl(child0(node), sym);
	q = sym->type.valtype.qual;
	if (QUAL_OVERFLOW(q))
	    semantic_error(node->srcpos, "too many qualifiers");
	sym->type.valtype.qual = (q << QUAL_SHIFT) | QUAL_FUNC;
	break;
    case NULL_INHERIT:
	break;
    case NULL_STAR_N:
	get_null_dcl(child1(node), sym);
	/* fall through */
    case NULL_STAR:
	q = sym->type.valtype.qual;
	if (QUAL_OVERFLOW(q))
	    semantic_error(node->srcpos, "too many qualifiers");
	sym->type.valtype.qual = (q << QUAL_SHIFT) | QUAL_PTR;
	break;
    case NULL_N:
    case NULL_INHERIT_N:
	get_null_dcl(child0(node), sym);
	break;
    case NULL_N_SUB_E:
	get_null_dcl(child0(node), sym);
	q = sym->type.valtype.qual;
	if (QUAL_OVERFLOW(q))
	    semantic_error(node->srcpos, "too many qualifiers");
	sym->type.valtype.qual = (q << QUAL_SHIFT) | QUAL_ARRAY;
	dim = (DIMLIST *) malloc(sizeof *dim);
	CHECK_MALLOC(dim);
	dim->next = sym->type.valtype.dimensions;
	sym->type.valtype.dimensions = dim;
	dim->size = evalIConstExpr(child1(node));
	break;
    case NULL_SUB_E:
	q = sym->type.valtype.qual;
	if (QUAL_OVERFLOW(q))
	    semantic_error(node->srcpos, "too many qualifiers");
	sym->type.valtype.qual = (q << QUAL_SHIFT) | QUAL_ARRAY;
	dim = (DIMLIST *) malloc(sizeof *dim);
	CHECK_MALLOC(dim);
	dim->next = sym->type.valtype.dimensions;
	sym->type.valtype.dimensions = dim;
	dim->size = evalIConstExpr(child0(node));
	break;
    case NULL_N_SUB:
	get_null_dcl(child0(node), sym);
	/* fall through */
    case NULL_SUB:
	q = sym->type.valtype.qual;
	if (QUAL_OVERFLOW(q))
	    semantic_error(node->srcpos, "too many qualifiers");
	sym->type.valtype.qual = (q << QUAL_SHIFT) | QUAL_ARRAY;
	dim = (DIMLIST *) malloc(sizeof *dim);
	CHECK_MALLOC(dim);
	dim->next = sym->type.valtype.dimensions;
	sym->type.valtype.dimensions = dim;
	dim->size = -1;
	break;
    }
    return;
}

static void
dcl_data_specs(TNODE * node,	/* genus: GEN_DATA_SPECS */
	       VALTYPE * type,
	       SYMTABLIST * symtablist)
{
    TNODE *p;
    SYM *sym;

    if (node->genus != GEN_DATA_SPECS) {
	internal_error(node->srcpos, "Unexpected genus: %d",
		       node->genus);
    }

    for (p = child0(node); p != NULL; p = TNEXT(p)) {
	sym = mk_valsym(type, VALSYM);

	dcl_data_item(child0(p), sym, symtablist);

	/*
	 * All functions are EXTERN (except functions with bodies
	 * which are handled by dcl_function).  Other class info
	 * is ignored (e.g. register, static).
	 */
	if (QUAL_ISFUNC(sym->type.valtype.qual))
	    sym->type.valtype.sclass = SCB_EXTERN_REF;
/*?unknown? Warn if PARAM or REG set */

/*?unknown? Here we put all local extern defs in the global symbol table.
*?unknown? In old C, once an extern was encountered it holds even outside it's scope.
*?unknown? This is not quite right because a local in an outer scope could hide the
*?unknown? extern.  There is currently no attempt to resolve externs and globals.
*?unknown?*/
	if (sym->type.valtype.sclass & SCB_EXTERN_REF)
	    sym_insert(sym, symtablist->global_tab);
	else
	    sym_insert(sym, &symtablist->symtab);
	if (p->species == DATA_SPEC_INIT) {
	    dcl_all(child1(p), symtablist, NULL);
	    sym->type.valtype.sclass |= SCB_INIT;
	}

	if ((sym->type.valtype.sclass & SCB_CONST) &&
	    (sym->type.valtype.sclass & (SCB_STATIC | SCB_FILEGLOBAL))) {
	    sym->constValue = (CONST_VALUE *) malloc(
							sizeof *sym->constValue);
	    CHECK_MALLOC(sym->constValue);
	    if (p->species == DATA_SPEC_INIT) {
		if (child1(p)->species == INITIALIZER_EXPR)
		    evalConstExpr(child0(child1(p)),
				  sym->constValue);
		else {
		    free(sym->constValue);
		    sym->constValue = NULL;
		}
	    } else {
		/*
		 * uninitialized static defaults to 0.
		 */
		sym->constValue->type = CONST_VT;
		sym->constValue->fraction = 0;
	    }
	}
    }
}

static SYM *			/* returns pointer to tag symbol */
dcl_struct(TNODE * node,	/* genus: GEN_STRUCT_DCL */
	   SYMTABLIST * symtablist)
{
    TNODE *tagnode = NULL;
    SYM *tagsym;
    NAMETYPE nametype = NULL_SYM;
    TNODE *tmemlist;
    TNODE *tmem;
    TNODE *tclasstypes;
    TNODE *tmemdcls;
    TNODE *tdcl;
    VALTYPE type;
    SYM *fieldsym;
    int bit_alignment;
    int bits;

    if (node->genus != GEN_STRUCT_DCL) {
	internal_error(node->srcpos, "Unexpected genus: %d",
		       node->genus);
    }

    switch (node->species) {
    case DCL_STRUCT_TAG:
	tagnode = child0(node);
	nametype = STRUCT_TAG;
	break;
    case DCL_STRUCT_NOTAG:
	tagnode = NULL;
	nametype = STRUCT_TAG;
	break;
    case DCL_UNION_TAG:
	tagnode = child0(node);
	nametype = UNION_TAG;
	break;
    case DCL_UNION_NOTAG:
	tagnode = NULL;
	nametype = UNION_TAG;
	break;
#ifdef MVS
    case DCL_PSTRUCT_TAG:
	tagnode = child0(node);
	nametype = STRUCT_TAG;
	break;
    case DCL_PSTRUCT_NOTAG:
	tagnode = NULL;
	nametype = STRUCT_TAG;
	break;
    case DCL_PUNION_TAG:
	tagnode = child0(node);
	nametype = UNION_TAG;
	break;
    case DCL_PUNION_NOTAG:
	tagnode = NULL;
	nametype = UNION_TAG;
	break;
#endif /* MVS */
    default:
	internal_error(node->srcpos,
		       "Unknown species for STRUCT_DCL: %d", node->species);
    }

    tagsym = NULL;
    if (tagnode) {
	tagsym = sym_find(tagnode->text, nametype, symtablist->symtab);
	if (tagsym && tagsym->type.struct_tag.fieldlist) {
	    semantic_error(node->srcpos,
			   "Warning: struct/union %s redefined",
			   tagnode->text);
	    tagsym = NULL;
	}
    }
    if (tagsym == NULL) {
	tagsym = mk_tagsym(nametype);
	if (tagnode) {
	    tagsym->name = tagnode->text;
	    tagsym->def = tagnode;
	} else
	    tagsym->name = NULL;
	sym_insert(tagsym, &symtablist->symtab);
    }

    switch (node->species) {
    case DCL_STRUCT_TAG:
    case DCL_UNION_TAG:
#ifdef MVS
    case DCL_PSTRUCT_TAG:
    case DCL_PUNION_TAG:
#endif /* MVS */
	child0(node)->sym.sym = tagsym;
    }

    node->sym.sym = tagsym;	/* Helps make fake names for tagless structs. */

    bit_alignment = 0;

    tmemlist = LASTCHILD(node);
    for (tmem = child0(tmemlist); tmem != NULL; tmem = TNEXT(tmem)) {
	tclasstypes = child0(tmem);
	tmemdcls = child1(tmem);
	getClassTypes(tclasstypes, SYM_TYPE_SU_MEMBER, &type,
		      symtablist);
	for (tdcl = child0(tmemdcls); tdcl != NULL; tdcl = TNEXT(tdcl)) {
	    switch (tdcl->species) {
	    case MEM_DCL:
		fieldsym = mk_valsym(&type, VALSYM);
		dcl_data_item(child0(tdcl), fieldsym,
			      symtablist);
		sym_insert(fieldsym,
			   &tagsym->type.struct_tag.fieldlist);
		bit_alignment = 0;
		break;
	    case MEM_DCL_BIT:
		fieldsym = mk_valsym(&type, VALSYM);
		dcl_data_item(child0(tdcl), fieldsym,
			      symtablist);
		sym_insert(fieldsym,
			   &tagsym->type.struct_tag.fieldlist);
		bits = evalIConstExpr(LASTCHILD(tdcl));
		fieldsym->type.valtype.bits = bits;
		fieldsym->type.valtype.bit_alignment =
		    bit_alignment;
		bit_alignment += bits;
		break;
	    case MEM_BIT:
		bits = evalIConstExpr(LASTCHILD(tdcl));
		if (bits == 0)
		    bit_alignment = 0;
		else
		    bit_alignment += bits;
		break;
	    }
	}
    }

    return tagsym;
}

static SYM *			/* returns pointer to tag symbol */
ref_struct(TNODE * node,	/* genus: GEN_STRUCT_REF */
	   symType_t symType,
	   SYMTABLIST * symtablist)
{
    TNODE *tagnode;
    SYM *tagsym;
    NAMETYPE nametype = NULL_SYM;

    if (node->genus != GEN_STRUCT_REF) {
	internal_error(node->srcpos, "Unexpected genus: %d",
		       node->genus);
    }

    tagnode = child0(node);

    switch (node->species) {
    case REF_STRUCT:
	nametype = STRUCT_TAG;
	break;
    case REF_UNION:
	nametype = UNION_TAG;
	break;
#ifdef MVS
    case REF_PSTRUCT:
	nametype = STRUCT_TAG;
	break;
    case REF_PUNION:
	nametype = UNION_TAG;
	break;
#endif /* MVS */
    default:
	internal_error(node->srcpos,
		       "Unknown species for STRUCT_REF: %d", node->species);
    }

    if (symType == SYM_TYPE_NOSPEC)	/* ANSI: hide tag from outer scope. */
	tagsym = sym_find(tagnode->text, nametype, symtablist->symtab);
    else
	tagsym = ssym_find(tagnode->text, nametype, NULL_SYM, symtablist);
    if (tagsym == NULL) {
	tagsym = mk_tagsym(nametype);
	tagsym->name = tagnode->text;
	tagsym->def = tagnode;
	sym_insert(tagsym, &symtablist->symtab);
    }

    child0(node)->sym.sym = tagsym;

    return tagsym;
}

static SYM *			/* returns pointer to tag symbol */
dcl_enum(TNODE * node,		/* genus: GEN_ENUM_DCL */
	 SYMTABLIST * symtablist)
{
    TNODE *tagnode = NULL;
    SYM *tagsym = NULL;
    SYM *moesym;
    TNODE *tmoelist;
    TNODE *tmoe;
    int moeValue;

    if (node->genus != GEN_ENUM_DCL) {
	internal_error(node->srcpos,
		       "Unexpected genus: %d", node->genus);
    }

    switch (node->species) {
    case sENUM_TAG:
	tagnode = child0(node);
	tagsym = sym_find(tagnode->text, ENUM_TAG, symtablist->symtab);
	if (tagsym && tagsym->type.enum_tag.memlist) {
	    semantic_error(node->srcpos,
			   "Warning: enum %s redefined", tagnode->text);
	    tagsym = NULL;
	}
	break;
    case sENUM_NOTAG:
	tagsym = NULL;
	tagnode = NULL;
	break;
    default:
	internal_error(node->srcpos, "Unknown species for ENUM_DCL: %d",
		       node->species);
    }

    if (tagsym == NULL) {
	tagsym = mk_tagsym(ENUM_TAG);
	if (tagnode) {
	    tagsym->name = tagnode->text;
	    tagsym->def = tagnode;
	} else
	    tagsym->name = NULL;
	sym_insert(tagsym, &symtablist->symtab);
    }

    if (node->species == sENUM_TAG)
	child0(node)->sym.sym = tagsym;
    node->sym.sym = tagsym;	/* Helps make fake names for tagless structs. */

    moeValue = 0;
    tmoelist = LASTCHILD(node);
    for (tmoe = child0(tmoelist); tmoe != NULL; tmoe = TNEXT(tmoe)) {
	if (tmoe->species == MOE_VAL)
	    moeValue = evalIConstExpr(child1(tmoe));
	moesym = mk_moesym(child0(tmoe), tagsym, moeValue++);
	child0(tmoe)->sym.sym = moesym;
	sym_insert(moesym, &tagsym->type.enum_tag.memlist);
	sym_insert(moesym, &symtablist->symtab);
    }

    return tagsym;
}

static SYM *			/* returns pointer to tag symbol */
ref_enum(TNODE * node,		/* genus: GEN_ENUM_REF */
	 SYMTABLIST * symtablist)
{
    TNODE *tagnode;
    SYM *tagsym;

    if (node->genus != GEN_ENUM_REF) {
	internal_error(node->srcpos, "Unexpected genus: %d",
		       node->genus);
    }

    tagnode = child0(node);

    tagsym = ssym_find(tagnode->text, ENUM_TAG, NULL_SYM, symtablist);
    if (tagsym == NULL) {
	tagsym = mk_tagsym(ENUM_TAG);
	tagsym->name = tagnode->text;
	tagsym->def = tagnode;
	sym_insert(tagsym, &symtablist->symtab);
    }

    child0(node)->sym.sym = tagsym;

    return tagsym;
}

static int
checkBaseType(int base,
	      SRCPOS * srcpos)
{
    switch (base) {
    case BTB_CHAR_SIZE:
    case BTB_CHAR_SIZE | BTB_SIGNED:
    case BTB_INT | BTB_CHAR_SIZE:
    case BTB_INT | BTB_CHAR_SIZE | BTB_SIGNED:
	return BT_CHAR;
    case BTB_CHAR_SIZE | BTB_UNSIGNED:
    case BTB_INT | BTB_CHAR_SIZE | BTB_UNSIGNED:
	return BT_UCHAR;
    case BTB_SHORT_SIZE:
    case BTB_SHORT_SIZE | BTB_SIGNED:
    case BTB_INT | BTB_SHORT_SIZE:
    case BTB_INT | BTB_SHORT_SIZE | BTB_SIGNED:
	return BT_SHORT;
    case BTB_SHORT_SIZE | BTB_UNSIGNED:
    case BTB_INT | BTB_SHORT_SIZE | BTB_UNSIGNED:
	return BT_USHORT;
    case 0:
    case BTB_INT:
    case BTB_SIGNED:
    case BTB_INT | BTB_SIGNED:
	return BT_INT;
    case BTB_UNSIGNED:
    case BTB_INT | BTB_UNSIGNED:
	return BT_UINT;
    case BTB_LONG_SIZE:
    case BTB_LONG_SIZE | BTB_SIGNED:
    case BTB_INT | BTB_LONG_SIZE:
    case BTB_INT | BTB_LONG_SIZE | BTB_SIGNED:
	return BT_LONG;
    case BTB_LONG_SIZE | BTB_UNSIGNED:
    case BTB_INT | BTB_LONG_SIZE | BTB_UNSIGNED:
	return BT_ULONG;
    case BTB_FLOAT | BTB_SHORT_SIZE:
    case BTB_FLOAT | BTB_SHORT_SIZE | BTB_SIGNED:
    case BTB_FLOAT | BTB_SHORT_SIZE | BTB_UNSIGNED:
	return BT_FLOAT;
    case BTB_FLOAT:
    case BTB_FLOAT | BTB_SIGNED:
    case BTB_FLOAT | BTB_UNSIGNED:
    case BTB_FLOAT | BTB_SHORT_SIZE | BTB_LONG_SIZE:	/* long float */
    case BTB_FLOAT | BTB_SHORT_SIZE | BTB_LONG_SIZE | BTB_SIGNED:
    case BTB_FLOAT | BTB_SHORT_SIZE | BTB_LONG_SIZE | BTB_UNSIGNED:
	return BT_DOUBLE;
    case BTB_FLOAT | BTB_LONG_SIZE:
    case BTB_FLOAT | BTB_LONG_SIZE | BTB_SIGNED:
    case BTB_FLOAT | BTB_LONG_SIZE | BTB_UNSIGNED:
	return BT_LONGDOUBLE;
    case BT_STRUCT:
    case BT_UNION:
    case BT_ENUM:
    case BT_VOID:
	return base;
    case BT_UNKNOWN:
    default:
	semantic_error(srcpos, "Warning: conflicting types");
	return BT_UNKNOWN;
    }
}

/*
* checkSClass: Check that storage class makes sence.
*/
static int
checkSClass(int sclass,
	    symType_t symType,
	    SRCPOS * srcpos)
{
    int sc;
    int constVolatile;

    constVolatile = sclass & (SCB_CONST | SCB_VOLATILE);
    sc = sclass & ~(SCB_CONST | SCB_VOLATILE);

    switch (symType) {
    case SYM_TYPE_GLOBAL:
    case SYM_TYPE_FUNC:
	sc |= SCB_GLOBAL;
	break;
    case SYM_TYPE_PARAM:
	sc |= SCB_PARAM;
	break;
    case SYM_TYPE_SU_MEMBER:
    case SYM_TYPE_CAST:
	if (sc) {
	    semantic_error(srcpos,
			   "Warning: storage class not allowed");
	    return 0;
	}
	break;
    case SYM_TYPE_LOCAL:
	break;
    case SYM_TYPE_NOSPEC:
	return 0;		/* Be generous; storage class is irrelevant. */
    default:
	internal_error(srcpos, "Unexpected symType: %d", symType);
    }

    switch (sc) {
    case SCB_PARAM:
    case SCB_PARAM | SCB_AUTO:
	return SCB_PARAM | constVolatile;	/* param */
    case SCB_PARAM | SCB_REG:
    case SCB_PARAM | SCB_AUTO | SCB_REG:
	return SCB_PARAM | SCB_REG | constVolatile;	/* reg param */
    case SCB_EXTERN_REF | SCB_GLOBAL | SCB_INLINE:
    case SCB_STATIC | SCB_GLOBAL | SCB_INLINE:
    case SCB_INLINE:
	return SCB_INLINE | constVolatile;	/* static local */
    case SCB_AUTO:
    case 0:
	return constVolatile;	/* local */
    case SCB_REG:
    case SCB_AUTO | SCB_REG:
	return SCB_REG | constVolatile;		/* reg local */
    case SCB_STATIC:
	return SCB_STATIC | constVolatile;	/* static local */
    case SCB_EXTERN_REF:
    case SCB_EXTERN_REF | SCB_GLOBAL:
	return SCB_EXTERN_REF | constVolatile;	/* extern */
    case SCB_GLOBAL | SCB_STATIC:
	return SCB_FILEGLOBAL | constVolatile;	/* file global */
    case SCB_GLOBAL:
	return SCB_GLOBAL | constVolatile;	/* global */
    case SCB_TYPENAME:
    case SCB_TYPENAME | SCB_GLOBAL:
    case SCB_TYPENAME | SCB_STATIC:
    case SCB_TYPENAME | SCB_GLOBAL | SCB_STATIC:
	return SCB_TYPENAME | constVolatile;	/* typedef */
    default:
	fprintf(stderr, "Did not find storage-class %d (%#x)\n", sc, sc);
	semantic_error(srcpos, "Warning: conflicting storage classes");
	return 0;
    }
}

static void
getClassTypes(TNODE * node,
	      symType_t symType,
	      VALTYPE * type,
	      SYMTABLIST * symtablist)
{
    TNODE *p;
    TNODE *t;
    SYM *tname_type;

    if (node->genus != GEN_CLASSTYPES) {
	internal_error(node->srcpos, "Unexpected genus: %d",
		       node->genus);
    }

    type->sclass = 0;
    type->base = 0;
    type->qual = 0;
    type->tag = NULL;
    type->param_list = NULL;
    type->dimensions = NULL;

    for (p = child0(node); p != NULL; p = TNEXT(p)) {
	switch (p->species) {
	case CLASSTYPE_INT:
	    type->base |= BTB_INT;
	    break;
	case CLASSTYPE_CHAR:
	    type->base |= BTB_CHAR_SIZE;
	    break;
	case CLASSTYPE_FLOAT:
	    type->base |= (BTB_FLOAT | BTB_SHORT_SIZE);
	    break;
	case CLASSTYPE_DOUBLE:
	    type->base |= BTB_FLOAT;
	    break;
	case CLASSTYPE_LONG:
	    type->base |= BTB_LONG_SIZE;
	    break;
	case CLASSTYPE_SHORT:
	    type->base |= BTB_SHORT_SIZE;
	    break;
	case CLASSTYPE_UNSIGNED:
	    type->base |= BTB_UNSIGNED;
	    break;
	case CLASSTYPE_SIGNED:
	    type->base |= BTB_SIGNED;
	    break;
	case CLASSTYPE_VOID:
	    type->base |= BT_VOID;
	    break;
	case CLASSTYPE_CONST:
	    type->sclass |= SCB_CONST;
	    break;
	case CLASSTYPE_VOLATILE:
	    type->sclass |= SCB_VOLATILE;
	    break;
	case CLASSTYPE_AUTO:
	    type->sclass |= SCB_AUTO;
	    break;
	case CLASSTYPE_REGISTER:
	    type->sclass |= SCB_REG;
	    break;
	case CLASSTYPE_STATIC:
	    type->sclass |= SCB_STATIC;
	    break;
	case CLASSTYPE_INLINE:
	    type->sclass |= SCB_INLINE;
	    break;
	case CLASSTYPE_EXTERN:
	    type->sclass |= SCB_EXTERN_REF;
	    break;
	case CLASSTYPE_TYPEDEF:
	    if (type->sclass & SCB_TYPENAME)
		semantic_error(p->srcpos,
			       "Warning: Redundant typedef", 0);
	    type->sclass |= SCB_TYPENAME;
	    break;
	case CLASSTYPE_TNAME:
	    t = child0(p);
	    tname_type = ssym_find(t->text, TYPE_NAME, NULL_SYM,
				   symtablist);
	    if (tname_type == NULL) {
		semantic_error(node->srcpos,
			       "Warning: Undefined typename %s",
			       child0(node)->text);
		type->base = BT_UNKNOWN;
		return;
	    }
	    t->sym.sym = tname_type;	/* Resolve while we're here. */
	    type->base |= tname_type->type.valtype.base;
	    type->tag = tname_type->type.valtype.tag;
	    type->qual = tname_type->type.valtype.qual;
	    /*
	     * Note: Pointers to lists are used --
	     * lists are not copied here.
	     */
	    type->param_list = tname_type->type.valtype.param_list;
	    type->dimensions = tname_type->type.valtype.dimensions;
	    break;
	case CLASSTYPE_STRUCT_D:
	    if (type->base & (BT_STRUCT | BT_UNION | BT_ENUM))
		semantic_error(p->srcpos,
			       "Multiple struct/union/enum");
	    type->tag = dcl_struct(child0(p), symtablist);
	    if (type->tag->nametype == STRUCT_TAG)
		type->base |= BT_STRUCT;
	    else
		type->base |= BT_UNION;
	    break;
	case CLASSTYPE_STRUCT_R:
	    if (type->base & (BT_STRUCT | BT_UNION | BT_ENUM))
		semantic_error(p->srcpos,
			       "Multiple struct/union/enum");
	    type->tag = ref_struct(child0(p), symType, symtablist);
	    if (type->tag->nametype == STRUCT_TAG)
		type->base |= BT_STRUCT;
	    else
		type->base |= BT_UNION;
	    break;
	case CLASSTYPE_ENUM_D:
	    if (type->base & (BT_STRUCT | BT_UNION | BT_ENUM))
		semantic_error(p->srcpos,
			       "Multiple struct/union/enum");
	    type->tag = dcl_enum(child0(p), symtablist);
	    type->base |= BT_ENUM;
	    break;
	case CLASSTYPE_ENUM_R:
	    if (type->base & (BT_STRUCT | BT_UNION | BT_ENUM))
		semantic_error(p->srcpos,
			       "Multiple struct/union/enum");
	    type->tag = ref_enum(child0(p), symtablist);
	    type->base |= BT_ENUM;
	    break;
	default:
	    internal_error(p->srcpos, "Unexpected species: %d",
			   p->species);
	}
    }

    type->base = checkBaseType(type->base, node->srcpos);
    type->sclass = checkSClass(type->sclass, symType, node->srcpos);
}

static void
dcl(TNODE * node,		/* GEN_INIT_DCL or GEN_INDATA_DCL */
    SYMTABLIST * symtablist)
{
    TNODE *p;
    VALTYPE type;
    SYM *sym;
    symType_t symType;

    if (node->genus != GEN_INIT_DCL && node->genus != GEN_INDATA_DCL) {
	internal_error(node->srcpos, "Unexpected genus: %d",
		       node->genus);
    }

    if (node->species == INIT_DCL_NOSPEC)
	symType = SYM_TYPE_NOSPEC;
    else if (node->genus == GEN_INIT_DCL)
	symType = SYM_TYPE_GLOBAL;
    else
	symType = SYM_TYPE_LOCAL;
    getClassTypes(child0(node), symType, &type, symtablist);

    switch (node->species) {
    case INIT_DCL_SPEC:
	if (type.sclass & SCB_TYPENAME) {
	    p = child1(node);
	    for (p = child0(p); p != NULL; p = TNEXT(p)) {
		sym = mk_valsym(&type, TYPE_NAME);
		dcl_data_item(child0(p), sym, symtablist);
		sym_insert(sym, &symtablist->symtab);
	    }
	} else
	    dcl_data_specs(child1(node), &type, symtablist);
	break;
    case INIT_DCL_NOSPEC:
	/*
	 * A type declaration without any data items may be
	 * used to define a struct/union/enum tag.  Some programmers
	 * like to preceede these declarations with "typedef" for
	 * clarity (even though no type name is defined).  ANSI
	 * allows undeclaration of a struct/union tag this way.
	 * We allow all declarations without data items even
	 * though they don't make much sense.  In any case,
	 * getClassTypes has taken care of the tags so there is
	 * nothing to do here.
	 */
	break;
    case INIT_DCL_EMPTY:
	break;
    default:
	internal_error(node->srcpos,
		       "Unknown species for INIT/INDATA_DCL: %d",
		       node->species);
    }
}

static void
dcl_function(TNODE * node,	/* GEN_FUNCTION */
	     SYMTABLIST * symtablist)
{
    VALTYPE type;
    SYM *sym;

    if (node->genus != GEN_FUNCTION) {
	internal_error(node->srcpos, "Unexpected genus: %d",
		       node->genus);
    }

    switch (node->species) {
    case FUNC_TFPC:
    case FUNC_TFC:
	getClassTypes(child0(node), SYM_TYPE_FUNC, &type, symtablist);
	sym = mk_valsym(&type, VALSYM);
	dcl_data_item(child1(node), sym, symtablist);
	sym_insert(sym, &symtablist->symtab);
	break;
    case FUNC_FPC:
    case FUNC_FC:
	type.sclass = SCB_GLOBAL;
	type.base = BT_INT;
	type.qual = 0;
	type.tag = NULL;
	type.param_list = NULL;
	type.dimensions = NULL;
	sym = mk_valsym(&type, VALSYM);
	dcl_data_item(child0(node), sym, symtablist);
	sym_insert(sym, &symtablist->symtab);
	break;
    }
}

static void
dcl_formals(TNODE * node,	/* GEN_NAMES */
	    SYMLIST ** symtab)
{
    TNODE *p;
    SYM *sym;

    if (node->genus != GEN_NAMES) {
	internal_error(node->srcpos, "Unexpected genus: %d",
		       node->genus);
    }

    for (p = child0(node); p != NULL; p = TNEXT(p)) {
	sym = (SYM *) malloc(sizeof *sym);
	CHECK_MALLOC(sym);

	sym->name = p->text;
	sym->def = p;
	sym->ref = NULL;
	sym->nametype = NULL_SYM;
	sym_insert(sym, symtab);
	p->sym.sym = sym;
	sym->constValue = NULL;
    }
}

static void
dcl_param(TNODE * node,		/* GEN_PARAM_DCL */
	  SYMTABLIST * symtablist)
{
    TNODE *p;
    TNODE *namep;
    VALTYPE type;
    SYM *sym;

    if (node->genus != GEN_PARAM_DCL) {
	internal_error(node->srcpos, "Unexpected genus: %d",
		       node->genus);
    }

    getClassTypes(child0(node), SYM_TYPE_PARAM, &type, symtablist);
    p = child1(node);
    for (p = child0(p); p != NULL; p = TNEXT(p)) {
	namep = child0(p);
	while (namep) {
	    if (namep->genus == GEN_NAME)
		break;
	    if (namep->genus == GEN_DATA_ITEM)
		namep = child0(namep);
	    else
		namep = TNEXT(namep);
	}
	if (namep == NULL)
	    return;		/* shouldn't happen */
	sym = sym_find(namep->text, NULL_SYM, symtablist->symtab);
	if (sym) {
	    sym->nametype = VALSYM;
	    sym->type.valtype.sclass = type.sclass;
	    sym->type.valtype.base = type.base;
	    sym->type.valtype.bits = -1;
	    sym->type.valtype.bit_alignment = 0;
	    sym->type.valtype.qual = type.qual;
	    if (type.param_list)
		sym->type.valtype.param_list =
		    copy_symlist(type.param_list);
	    else
		sym->type.valtype.param_list = NULL;
	    sym->type.valtype.tag = type.tag;
	    if (type.dimensions)
		sym->type.valtype.dimensions =
		    copy_dimlist(type.dimensions);
	    else
		sym->type.valtype.dimensions = NULL;
	} else {
	    semantic_error(node->srcpos,
			   "Warning: formal parameter %s missing",
			   namep->text ? namep->text : "");
	    sym = mk_valsym(&type, VALSYM);
	    sym_insert(sym, &symtablist->symtab);
	}

	dcl_data_item(p, sym, symtablist);
    }
}

/*
* isVoid:  Is this a declaration for a function with no params?  Search
* 	down the ANSI_PARAMS tree for a TYPE_VOID node.  There should
*	be only one child at each node.
*/
static int
isVoid(TNODE * node)		/* GEN_ANSI_PARAMS */
{
    TNODE *p;
    TNODE *last;

    last = node;
    for (p = child0(node); p != NULL; p = child0(p)) {
	if (p != LASTCHILD(last))
	    return 0;
	last = p;
    }

    if (last && last->genus == GEN_CLASSTYPE &&
	last->species == CLASSTYPE_VOID)
	return 1;

    return 0;
}

static void
ansiParam(TNODE * node,		/* GEN_ANSI_PARAM */
	  SYMTABLIST * symtablist)
{
    VALTYPE type;
    SYM *sym;

    if (node->genus != GEN_ANSI_PARAM) {
	internal_error(node->srcpos, "Unexpected genus: %d",
		       node->genus);
    }
    if (node->species != ANSI_DATA_ITEM) {
	if (!isVoid(PARENT(node)))
	    semantic_error(node->srcpos, "Parameter name missing");
	return;
    }

    getClassTypes(child0(node), SYM_TYPE_PARAM, &type, symtablist);
    sym = mk_valsym(&type, VALSYM);
    dcl_data_item(child1(node), sym, symtablist);
    sym_insert(sym, &symtablist->symtab);
}

static void
default_params(SYMLIST ** symtab)
{
    SYMLIST *p;
    SYM *sym;

    for (p = *symtab; p != NULL; p = p->next) {
	sym = p->sym;
	if (sym->nametype == NULL_SYM) {
	    sym->nametype = VALSYM;
	    sym->type.valtype.sclass = SCB_PARAM;
	    sym->type.valtype.base = BT_INT;
	    sym->type.valtype.bits = -1;
	    sym->type.valtype.bit_alignment = 0;
	    sym->type.valtype.qual = 0;
	    sym->type.valtype.param_list = NULL;
	    sym->type.valtype.tag = NULL;
	    sym->type.valtype.dimensions = NULL;
	}
    }
}

static void
dcl_label(TNODE * node,
	  SYMLIST ** label_tab,
	  int definition)
{
    SYM *sym;

    if (node->genus != GEN_NAME) {
	internal_error(node->srcpos, "Unexpected genus: %d",
		       node->genus);
    }

    if (label_tab == NULL) {
	internal_error(node->srcpos, "No scope for label: %s",
		       node->text);
	return;
    }

    sym = sym_find(node->text, LABELSYM, *label_tab);
    if (sym == NULL) {
	sym = mk_label(node);
	sym_insert(sym, label_tab);
    }

    if (definition) {
	if (sym->def)
	    semantic_error(node->srcpos, "duplicate label: %s",
			   node->text);
	sym->def = node;
    }

    node->sym.sym = sym;
}

static void
dcl_all(TNODE * node,
	SYMTABLIST * symtablist,
	SYMLIST ** label_tab)
{
    SYMTABLIST stl;
    TNODE *p;
    SYM *sym;
    VALTYPE type;

    if (node == NULL)
	return;

    switch (node->genus) {
    case GEN_MODULE:
	if (symtablist != NULL)
	    internal_error(node->srcpos, "Unexpected global tab");
	stl.next = symtablist;	/* NULL */
	stl.symtab = NULL;
	stl.global_tab = &stl.symtab;
	for (p = child0(node); p != NULL; p = TNEXT(p))
	    dcl_all(p, &stl, NULL);
	node->sym.symtab = stl.symtab;
	break;
    case GEN_FUNCTION:
	if (symtablist == NULL)
	    internal_error(node->srcpos, "No global tab");
	dcl_function(node, symtablist);
	stl.next = symtablist;
	stl.symtab = NULL;
	stl.global_tab = symtablist->global_tab;
	for (p = child0(node); p != NULL; p = TNEXT(p))
	    dcl_all(p, &stl, &stl.symtab);
	node->sym.symtab = stl.symtab;
	break;
    case GEN_COMPSTMT:
	if (symtablist == NULL)
	    internal_error(node->srcpos, "No global tab");
	if (PARENT(node)->genus == GEN_FUNCTION)
	    default_params(&symtablist->symtab);
	if (node->species != COMPSTMT_STMTS) {
	    stl.next = symtablist;
	    stl.symtab = NULL;
	    stl.global_tab = symtablist->global_tab;
	} else
	    stl = *symtablist;
	for (p = child0(node); p != NULL; p = TNEXT(p))
	    dcl_all(p, &stl, label_tab);
	node->sym.symtab = stl.symtab;
	break;
    case GEN_PARAM_DCL:
	dcl_param(node, symtablist);
	break;
    case GEN_INIT_DCL:
    case GEN_INDATA_DCL:
	dcl(node, symtablist);
	break;
    case GEN_NAMES:
	dcl_formals(node, &symtablist->symtab);
	break;
    case GEN_NAME:
	if (PARENT(node)->genus == GEN_STMT) {
	    if (label_tab)
		dcl_label(node, label_tab, 0);
	} else if (PARENT(node)->genus == GEN_EXPR &&
		   PARENT(node)->species == EXPR_LNAME) {
	    node->sym.sym = ssym_find(node->text, VALSYM, MEM_ENUM,
				      symtablist);
	} else if (PARENT(node)->genus == GEN_EXPR &&
		   (PARENT(node)->species == EXPR_LDOT ||
		    PARENT(node)->species == EXPR_LARROW)) {
	    expr_type(child0(PARENT(node)), &type);
	    if (PARENT(node)->species == EXPR_LARROW) {
		if (!QUAL_ISPTR(type.qual) && !QUAL_ISARRAY(type.qual))
		    semantic_error(PARENT(node)->srcpos, "type error");
	    } else if (type.qual != 0)
		semantic_error(PARENT(node)->srcpos, "type error");
	    if (type.tag) {
		node->sym.sym = (SYM *) sym_find(node->text, VALSYM,
						 type.tag->type.struct_tag.fieldlist);
	    } else
		semantic_error(node->srcpos, "invalid struct/union field");
	} else
	    break;
	if (node->sym.sym == NULL) {
	    semantic_error(node->srcpos, "undeclared: %s",
			   node->text);
	    type.sclass = 0;
	    type.base = BT_UNKNOWN;
	    type.qual = 0;
	    type.tag = NULL;
	    type.param_list = NULL;
	    type.dimensions = NULL;
	    node->sym.sym = mk_valsym(&type, VALSYM);
	    node->sym.sym->name = node->text;
	    node->sym.sym->def = node;
	    sym_insert(node->sym.sym, &symtablist->symtab);
	}
	break;
    case GEN_FNAME:
	if (PARENT(node)->genus == GEN_FUNC_SPEC)
	    break;		/* Already handled under FUNCTION. */
	node->sym.sym = ssym_find(node->text, VALSYM, MEM_ENUM,
				  symtablist);
	if (node->sym.sym == NULL) {
	    type.sclass = SCB_EXTERN_REF;
	    type.base = BT_INT;
	    type.qual = QUAL_FUNC;
	    type.tag = NULL;
	    type.param_list = NULL;
	    type.dimensions = NULL;
	    node->sym.sym = mk_valsym(&type, VALSYM);
	    node->sym.sym->def = node;
	    node->sym.sym->name = node->text;
	    node->sym.sym->def = node;
	    if (symtablist == NULL)
		internal_error(node->srcpos, "No global tab");
	    sym_insert(node->sym.sym, symtablist->global_tab);
	}
	break;
    case GEN_CAST_TYPE:
	clear_type(&type);
	getClassTypes(child0(node), SYM_TYPE_CAST, &type, symtablist);
	sym = mk_valsym(&type, VALSYM);
	if (node->species == CAST_TYPE)
	    get_null_dcl(child1(node), sym);
	node->sym.sym = sym;
	break;
    case GEN_ANSI_PARAMS:
	if (PARENT(node)->genus == GEN_FUNC_SPEC)
	    for (p = child0(node); p != NULL; p = TNEXT(p))
		ansiParam(p, symtablist);
	/* Don't bother to check function prototype unless it */
	/* is part of the function definition. */
	break;
    case GEN_STMT:
	if (node->species == STMT_LABEL) {
	    dcl_label(child0(node), label_tab, 1);
	    dcl_all(child1(node), symtablist, label_tab);
	    break;
	}
	/* fall through */
    case GEN_FUNC_SPEC:
    default:
	for (p = child0(node); p != NULL; p = TNEXT(p))
	    dcl_all(p, symtablist, label_tab);
	break;
    }
}

void
dump_sym(TNODE * node,
	 char *prefix)
{
    SYMLIST *symtab;
    TNODE *p;
    char *label;
    static void *reglst = NULL;

    if (reglst == NULL) {
	reglst = (void *) reglst_create();
	dump_sym(node, prefix);
	reglst_free(reglst);
	return;
    }

    if (node == NULL)
	return;

    switch (node->genus) {
    case GEN_MODULE:
	label = "=== MODULE ===\n";
	break;
    case GEN_FUNCTION:
	label = "=== FUNCTION ===\n";
	break;
    case GEN_COMPSTMT:
	label = "=== COMPSTMT ===\n";
	break;
    case GEN_NAME:
    case GEN_FNAME:
    case GEN_TNAME:
	fprintf(stderr, "%d at ",
		reglst_insert(reglst, node->sym.sym));
	print_srcpos(node->srcpos, stderr);
	if (node->text)
	    fprintf(stderr, " \"%s\"\n", node->text);
	else
	    fprintf(stderr, "?\n");
	label = NULL;
	break;
    default:
	label = NULL;
	break;
    }

    if (label) {
	fputs(label, stderr);
	for (symtab = node->sym.symtab; symtab; symtab = symtab->next) {
	    fprintf(stderr, "%d ",
		    reglst_insert(reglst, symtab->sym));
	    print_sym(symtab->sym, prefix);
	}
    }

    for (p = child0(node); p != NULL; p = TNEXT(p))
	dump_sym(p, prefix);
}

void
do_sym(TNODE * node)
{
    if (node->genus != GEN_MODULE) {
	internal_error(node->srcpos, "Unexpected genus: %d",
		       node->genus);
    }

    dcl_all(node, NULL, NULL);
}
