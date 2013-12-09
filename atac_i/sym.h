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
#ifndef sym_H
#define sym_H
static const char sym_h[] = "$Id: sym.h,v 3.11 2013/12/09 00:20:32 tom Exp $";
/*
* @Log: sym.h,v @
* Revision 3.9  1997/12/09 00:12:45  tom
* moved extern-declaration of decis_sym here.
*
* Revision 3.8  1997/05/12 00:20:24  tom
* correct sign in QUAL_OVERFLOW
*
* Revision 3.7  1997/05/11 23:07:09  tom
* add prototypes for print_sym.c
*
* Revision 3.6  1997/05/10 23:59:15  tom
* add prototypes for const.c, which use this module
*
* Revision 3.5  1996/11/13 00:25:02  tom
* change ident to 'const' to quiet gcc
*
* Revision 3.4  1995/12/13 01:03:44  tom
* add SCB_INLINE
*
* Revision 3.3  94/04/04  10:14:43  jrh
* Add Release Copyright
* 
* Revision 3.2  94/01/31  13:15:22  saul
* Make "too many qualifiers" warning portable.
* 
* Revision 3.1  93/08/09  12:33:14  saul
* sign and exponent dropped from value structure
* 
* Revision 3.0  92/11/06  07:45:40  saul
* propagate to version 3.0
* 
* Revision 2.5  92/10/28  08:55:15  saul
* enum's removed for portability
* 
* Revision 2.4  92/07/10  13:55:22  saul
* New CONST_VALUE type; QUAL_OVERFLOW macro; global_tab in SYMTABLIST
* 
* Revision 2.3  92/03/17  14:23:04  saul
* copyright
* 
* Revision 2.2  91/06/13  12:50:23  saul
* Changes to storage class and type values
* 
* Revision 2.1  91/06/13  12:39:25  saul
* Propagate to version 2.0
* 
* Revision 1.1  91/06/12  20:25:53  saul
* Aug 1990 baseline
* 
*-----------------------------------------------end of log
*/
/*
* Storage classes: 
*	The following storage classes are mutually exclusive:
*		PARAM, STATIC, GLOBAL, FILEGLOBAL, AUTO, TYPENAME, and EXTERN_REF.
*	REG may go with PARAM or AUTO.
*	INIT may go with any but EXTERN_REF, TYPENAME, and PARAM.
*	CONST and VOLATLE are mutually exclusive.
*	INIT, CONST, VOLATILE not implemented.
*	Examples:
*		global variable		GLOBAL
*		static global variable	FILEGLOBAL
*		extern variable		EXTERN_REF
*		global function	w body	GLOBAL	
*		static function	w body	FILEGLOBAL	
*		function withno body	EXTERN_REF	
*		undeclared function	EXTERN_REF
*		register parameter	REG PARAM
*		non-register parameter	PARAM
*		static local variable	STATIC
*		register local variable	REG
*		other local variable	<zero>
*/
#define SCB_PARAM	1	/* function parameter */
#define SCB_STATIC	2	/* static local */
#define SCB_GLOBAL	4	/* declared outside any function, not static */
#define SCB_FILEGLOBAL	8	/* static global */
#define SCB_EXTERN_REF	16	/* func dcl without body or explicit extern */
#define SCB_REG		32	/* explicit register */
#define SCB_INIT	64	/* explicitly initialized */
#define SCB_CONST	128	/* explicit constant (ANSI) */
#define	SCB_VOLATILE	256	/* explicit volitile (ANSI) */
#define SCB_AUTO	512
#define SCB_INLINE	1024
#define SCB_TYPENAME	2048	/* from typedef */

#define BTB_INT		1
#define BTB_FLOAT	2
#define BTB_UNSIGNED	4
#define BTB_SIGNED	8
#define BTB_CHAR_SIZE	16
#define BTB_SHORT_SIZE	32
#define BTB_LONG_SIZE	64

#define BT_STRUCT	128
#define BT_UNION	256
#define BT_ENUM		512
#define BT_VOID		1024
#define BT_UNKNOWN	2048

#define BT_INT		(BTB_INT | BTB_SIGNED)
#define BT_UINT		(BTB_INT | BTB_UNSIGNED)
#define BT_CHAR		(BTB_INT | BTB_SIGNED | BTB_CHAR_SIZE)
#define BT_UCHAR	(BTB_INT | BTB_CHAR_SIZE | BTB_UNSIGNED)
#define BT_SHORT	(BTB_INT | BTB_SIGNED | BTB_SHORT_SIZE)
#define BT_USHORT	(BTB_INT | BTB_SHORT_SIZE | BTB_UNSIGNED)
#define BT_LONG		(BTB_INT | BTB_SIGNED | BTB_LONG_SIZE)
#define BT_ULONG	(BTB_INT | BTB_LONG_SIZE | BTB_UNSIGNED)
#define BT_FLOAT	(BTB_FLOAT | BTB_SHORT_SIZE)
#define BT_DOUBLE	(BTB_FLOAT)
#define BT_LONGDOUBLE	(BTB_FLOAT | BTB_LONG_SIZE)

#define BT_ISARITH(b)	((b) & (BTB_INT | BTB_FLOAT | BT_ENUM))

#define QUAL_PTR	1
#define QUAL_FUNC	2
#define QUAL_ARRAY	3

#define QUAL_SHIFT	2
#define QUAL_MASK	3
#define QUAL_OVERFLOW(q) ((q) & (unsigned long)(~ LURSHIFT(~0L, QUAL_SHIFT)))

#define QUAL_ISPTR(q)	(((q) & QUAL_MASK) == QUAL_PTR)
#define QUAL_ISFUNC(q)	(((q) & QUAL_MASK) == QUAL_FUNC)
#define QUAL_ISARRAY(q)	(((q) & QUAL_MASK) == QUAL_ARRAY)

typedef int NAMETYPE;
#define NULL_SYM	((NAMETYPE) 0)	/* type not specified */
#define STRUCT_TAG	((NAMETYPE) 1)
#define UNION_TAG	((NAMETYPE) 2)
#define ENUM_TAG	((NAMETYPE) 3)
#define MEM_ENUM	((NAMETYPE) 4)	/* member of enum */
#define TYPE_NAME	((NAMETYPE) 5)	/* from typedef */
#define VALSYM		((NAMETYPE) 6)	/* variable or function name */
#define LABELSYM	((NAMETYPE) 7)	/* goto label */

typedef int reftype_t;
#define REF_REF	((reftype_t) 0)
#define DEF_REF	((reftype_t) 1)

typedef struct tnode_list {
    struct tnode_list *next;
    struct tnode *ref;
    reftype_t type;
} TNODE_LIST;

typedef struct symtablist {
    struct symtablist *next;
    struct symlist *symtab;
    struct symlist **global_tab;
} SYMTABLIST;

typedef struct symlist {
    struct symlist *next;
    struct sym *sym;
} SYMLIST;

typedef struct dimlist {
    struct dimlist *next;
    int size;
} DIMLIST;

/*
* Symbol Table:  List of symbols.
*
* name: variable, function, typedef, or tag name.  NULL if tag was omitted.
* def: Points at NAME node of declaring instance in parse tree.  For multiple
*	declarations points to most complete; last for tie.
* ref: List of pointers to NAME nodes for all instances in parse tree.
*	sym->ref->type specifies declaration or reference.
* struct/union tags: Sym->type.struct_tag points to a list of symbols for 
*	fields in the structure.  These symbols do not apear in any other
*	symbol table.
* enums: Sym->type.enum_tag.memlist points to a list of symbols for enum
*	members.  Enum members are in the same scope as the enum itself.
*	If the enum has no tag, Sym->name is NULL.
* enum members:  Sym->type.mem_enum.tag is a pointer to the tag for the enum
*	containing this member.
* bit fields: Sym->type.valtype.bits is -1 if no bit field has been specified.
*	otherwise it indicates the number of bits.
*	Sym->type.valtype.bit_alignment indicates the total number of bits
*	specified in consecutive bit fields before this field.  An unnamed
*	0 bit field specification sets the alignment back to 0.  Unamed bit
*	fields do not apear in the symbol table.
* array dimensions:  If sym->type.valtype.qual indicates an array,
*	sym->type.valtype.dimensions is a list of sizes of each dimension
*	with last dimension first.  An unspecified dimension has a size of -1.
*	(Define first and last as in: int a[first]...[last];)
*/

typedef struct valtype {
    short sclass;
    short base;
    short bits;
    short bit_alignment;
    unsigned long qual;
    SYMLIST *param_list;
    struct sym *tag;
    DIMLIST *dimensions;
} VALTYPE;

typedef int CONST_VALUE_TYPE;
#define CONST_VT		   0	/* value present */
#define CONST_VT_NON_NULL_ADDRESS  1	/* Non NULL address */
#define CONST_VT_UNDETERMINED      2	/* not a constant expression */
#define CONST_VT_UNDEFINED	   3	/* e.g. divide by 0 */
#define CONST_VT_UNKNOWN	   4	/* e.g. precision lost, overflow */

typedef struct {
    CONST_VALUE_TYPE type;
    long fraction;		/* actually whole integ. value */
    /* ??? Changes needed to handle float values. */
} CONST_VALUE;

typedef struct sym {
    char *name;
    struct tnode *def;
    TNODE_LIST *ref;
    NAMETYPE nametype;
    CONST_VALUE *constValue;
    union {
	struct {
	    SYMLIST *fieldlist;
	} struct_tag;		/* also union_tag */
	struct {
	    SYMLIST *memlist;
	} enum_tag;
	struct {
	    struct sym *tag;
	} mem_enum;
	struct list *label;
	VALTYPE valtype;	/* also type_name */
    } type;
} SYM;

/* fg_module.c */
extern SYM decis_sym;

/* const.c */
extern int evalIConstExpr(struct tnode *node);
extern void evalConstExpr(struct tnode *n, CONST_VALUE * value);
extern void testConst(void);

/* print_sym.c */
extern void print_sym(SYM * sym, char *prefix);
extern void print_type(FILE *f, VALTYPE * type, const char *name, const char *prefix);

#endif /* sym_H */
