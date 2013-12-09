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

static const char fg_module_c[] = "$Id: fg_module.c,v 3.13 2013/12/09 00:24:56 tom Exp $";
/*
* @Log: fg_module.c,v @
* Revision 3.11  2005/08/13 16:19:26  tom
* gcc warnings
*
* Revision 3.10  1997/12/09 00:50:13  tom
* moved externs to atac_i.h
*
* Revision 3.9  1997/05/11 23:38:16  tom
* include allpaths.h
*
* Revision 3.8  1997/05/11 20:03:27  tom
* split-out flowgraph.h
* correct type of global_defs to 'LIST*'
*
* Revision 3.7  1997/05/10 23:18:39  tom
* absorb srcpos.h into error.h
*
* Revision 3.6  1996/11/12 23:57:55  tom
* change ident to 'const' to quiet gcc
* add forward-ref prototypes
*
* Revision 3.5  1995/12/13 01:01:32  tom
* handle CLASSTYPE_INLINE
*
* Revision 3.4  94/04/04  10:12:50  jrh
* Add Release Copyright
* 
* Revision 3.3  93/12/15  12:54:50  saul
* McCabe cyclomatic number calculation
* 
* Revision 3.2  93/08/04  15:45:24  ewk
* Added MVS and solaris support.  Squelched some ANSI warnings.
* 
* 
* Revision 3.1  93/07/12  10:22:55  saul
* MVS MODULEID
* 
* Revision 3.0  92/11/06  07:45:14  saul
* propagate to version 3.0
* 
* Revision 2.9  92/11/04  15:57:46  saul
* don't free global_def symbols when freeing list.
* 
* Revision 2.8  92/11/02  11:36:54  saul
* remove unused variables
* 
* Revision 2.7  92/10/30  09:48:14  saul
* include portable.h
* 
* Revision 2.6  92/09/08  07:57:38  saul
* Add version entry in static trace.
* 
* Revision 2.5  92/07/10  11:54:56  saul
* New args for dug_du & dug_branch for new display; feasableFlag
* 
* Revision 2.4  92/04/07  09:00:20  saul
* remove ATACYSIS #ifdefs
* 
* Revision 2.3  92/03/17  14:22:26  saul
* copyright
* 
* Revision 2.2  91/06/19  13:33:46  saul
* Changes for ansi
* 
* Revision 2.1  91/06/13  12:39:04  saul
* Propagate to version 2.0
* 
 * Revision 1.1  91/06/12  20:25:40  saul
 * Aug 1990 baseline
 * 
*-----------------------------------------------end of log
*/
#include <stdio.h>
#include "portable.h"

#include "atac_i.h"
#include "error.h"
#include "tnode.h"
#include "tree.h"
#include "sym.h"
#include "dug.h"
#include "allpaths.h"
#include "flowgraph.h"
#include "version.h"

/* forward declarations */
static void fg_funcspec(TNODE * n, char **fname);
static void fg_global_defs(TNODE * n, LIST * global_defs);
static void fg_local_defs(TNODE * n, DUG * dug, BLOCK * sblk, BLOCK *
			  *endblk, int init);
static void fg_module(TNODE * n, FILE *outsrc, FILE *outtables, LIST *
		      global_defs, char *prefix);
static void init_outtables(FILE *f);

#define MAX_PREFIX	4	/* generated name prefix */

/* used in fg_*.c */
SYM decis_sym =
{
    "=decis=", NULL, NULL, NULL_SYM, 0,
    {
	{0}
    }
};

void
flowgraph(TNODE * tree,
	  FILE *outsrc,
	  FILE *outtables,
	  char *prefix)
{
    char *filename;
    int i;
    LIST *global_defs;		/* list of global variable defs. seen */

    init_outtables(outtables);
    for (i = 0; (filename = srcfname(i)); ++i)
	fprintf(outtables, "S %s %d\n", filename, srcfstamp(i));

    /*
     * Parse all functions.
     */
    global_defs = list_create();
    fg_module(tree, outsrc, outtables, global_defs, prefix);
    list_free(global_defs, (DataFree) 0);
}

static void
fg_module(TNODE * n,
	  FILE *outsrc,
	  FILE *outtables,
	  LIST * global_defs,
	  char *prefix)
{
    TNODE *p;
    SYMLIST *sl;

    if (n == NULL)
	return;

    switch (n->genus) {
    case GEN_MODULE:
    case GEN_MODULE_ITEM:
	for (p = CHILD0(n); p != NULL; p = TNEXT(p))
	    fg_module(p, outsrc, outtables, global_defs, prefix);
	return;
    case GEN_INIT_DCL:
	fg_global_defs(n, global_defs);
	return;
    case GEN_FUNCTION:
	{
	    BLOCK *start;
	    BLOCK *e;
	    LIST *i;
	    SYM *s;
	    DUG *dug;
	    char *fname;
	    unsigned long s_type;
	    static int funcno = 0;

	    dug = (DUG *) dug_create();

	    /*
	     * Create dummy block for initial defs, etc.
	     * The "start" of this block is used to mark the function
	     * entry; it consists of a COMPSTMT. The "end" is used 
	     * to identify the function text; it consists of the
	     * whole function.
	     */
	    start = dug_newblk(dug);
	    dug_startblk(dug, start, LASTCHILD(n));
	    dug_endblk(dug, start, n);

	    /*
	     * Dummy DEF for decisions.
	     */
	    dug_du(dug, &decis_sym, start, VAR_DEF, NULL);

	    /*
	     * Initial definition of each global at start block.
	     * Only globals declared above here in source are on
	     * the list.  [ We could just read the symbol table
	     * to get the list of globals, rather than find them
	     * in the parse.  This would add globals declared below
	     * also, but it doesn't matter because there won't be
	     * any USES for them. ]
	     */
	    for (i = 0; LIST_NEXT(global_defs, &i, &s);) {
		s_type = s->type.valtype.qual;
		if (!QUAL_ISARRAY(s_type))
		    dug_du(dug, s, start, VAR_DEF, s->def);
		/*
		 * If Variable is an array or pointer, define the
		 * DREF of the variable.  (Why?)
		 */
		if (QUAL_ISPTR(s_type) || QUAL_ISARRAY(s_type)) {
		    dug_du(dug, s, start,
			   VAR_DREF | VAR_DEF, s->def);
		}
	    }

	    /*
	     * Find function name, parameter list.
	     */
	    for (sl = n->sym.symtab; sl != NULL; sl = sl->next) {
		dug_du(dug, sl->sym, start, VAR_DEF,
		       sl->sym->def);
		/*
		 * If variable is an array or pointer, define the
		 * DREF of the variable.  (Why?)
		 */
		if (sl->sym == NULL)
		    continue;	/* Shouldn't happen. */
		if (QUAL_ISPTR(sl->sym->type.valtype.qual) ||
		    QUAL_ISARRAY(sl->sym->type.valtype.qual)) {
		    dug_du(dug, sl->sym, start,
			   VAR_DREF | VAR_DEF,
			   sl->sym->def);
		}
	    }
	    fg_funcspec(n, &fname);
	    dug_fname(dug, fname);

	    /*
	     * Function body.
	     */
	    e = dug_newblk(dug);
	    dug_branch(dug, start, e, COND_UNCONDITIONAL, 0, NULL);
	    fg_body(LASTCHILD(n), dug, e, NULL_BLK, NULL_BLK,
		    NULL_BLK, &e, NULL);

	    dug_clean(dug);
	    if (dump_tables)
		dug_dump(dug);
	    if (cyclomaticFlag)
		printf("%s: cyclomatic %d\n",
		       fname, dug_cyclomatic(dug));
	    dug_mark(dug);	/* for instrumenting */
	    dug_blocks(dug, outtables);
	    if (list_all_paths)
		allpaths(dug, stderr, 1);
	    else if (all_paths)
		allpaths(dug, stderr, 0);
	    if (count_alldu)
		alldu(dug);
	    else {
		paths(dug, outtables, feasableFlag);
		dug_tables(dug, funcno++, prefix, outsrc);
	    }
	    dug_free(dug);

	    return;
	}
    default:
	internal_error(n->srcpos,
		       "*******fg_module: unexpected genus: %d", n->genus);
	return;
    }
}

static void
fg_funcspec(TNODE * n,
	    char **fname)
{
    TNODE *p;

    if (n == NULL)
	return;

    p = n;

    while (p) {
	switch (p->genus) {
	case GEN_STARS:
	case GEN_CLASSTYPES:
	    /* Ignore these */
	    p = TNEXT(p);
	    break;
	case GEN_FUNCTION:
	case GEN_FUNC_SPEC:
	    /*
	     * Try each child.
	     */
	    p = CHILD0(p);
	    break;
	case GEN_FNAME:
	    *fname = p->sym.sym->name;
	    return;
	default:
	    internal_error(p->srcpos,
			   "*******fg_funcspec: unexpected genus: %d",
			   p->genus);
	    return;
	}
    }
    internal_error(n->srcpos, "*****fg_funcspec: can't find function name");
}

void
fg_body(TNODE * n,
	DUG * dug,
	BLOCK * sblk,		/* block at which statement or expression starts */
	BLOCK * bblk,		/* block reached by break stmt */
	BLOCK * cblk,		/* block reached by continue stmt */
	BLOCK * swblk,		/* block containing switch to case label */
	BLOCK ** endblk,	/* return block at end of stmt or expression */
	BLOCK ** dblk)		/* return block with "default:" label (used as flag) */
{
    BLOCK *end;

    end = sblk;
    if (n == NULL) {
	*endblk = end;
	return;
    }

    switch (n->genus) {
    case GEN_STMT_LIST:
	{
	    TNODE *p;

	    for (p = CHILD0(n); p != NULL; p = TNEXT(p)) {
		fg_stmt(p, dug, end, bblk, cblk, swblk, &end,
			dblk);
	    }
	    /* if (end == sblk) dug_endblk(dug, end, n); */
	    /* -- don't need: always covered by COMPSTMT */
	    break;
	}
    case GEN_INDATA_DCLS:
	fg_local_defs(n, dug, sblk, &end, 0);
	if (end == sblk)
	    dug_endblk(dug, end, n);
	break;
    case GEN_COMPSTMT:
	{
	    TNODE *p;

	    dug_startblk(dug, sblk, n);
	    for (p = CHILD0(n); p != NULL; p = TNEXT(p)) {
		fg_body(p, dug, end, bblk, cblk, swblk, &end,
			dblk);
	    }
	    if (end == sblk)
		dug_endblk(dug, end, n);
	    break;
	}
    case GEN_EXP_LIST:
	{
	    TNODE *p;

	    for (p = CHILD0(n); p != NULL; p = TNEXT(p)) {
		fg_expr(p, dug, end, &end, NULL, NULL,
			VAR_CUSE, VAR_VOID);
	    }
	    break;
	}
    default:
	internal_error(n->srcpos,
		       "*******fg_body: unexpected genus: %d\n", n->genus);
	break;
    }

    *endblk = end;
    return;
}

static void
fg_global_defs(TNODE * n,
	       LIST * global_defs)
{
    TNODE *p;

    if (n == NULL)
	return;

    switch (n->genus) {
    case GEN_INIT_DCL:
    case GEN_DATA_SPECS:
    case GEN_DATA_SPEC:
    case GEN_DATA_ITEMS:
    case GEN_DATA_ITEM:
	for (p = CHILD0(n); p != NULL; p = TNEXT(p))
	    fg_global_defs(p, global_defs);
	return;
    case GEN_NAME:
	if (n->sym.sym)		/* shouldn't be NULL */
	    list_put(global_defs, n->sym.sym);
	return;
    default:
	return;
    }
}

static void
fg_local_defs(TNODE * n,
	      DUG * dug,
	      BLOCK * sblk,
	      BLOCK ** endblk,
	      int init)
{
    BLOCK *end;

    end = sblk;

    if (n == NULL) {
	*endblk = end;
	return;
    }

    switch (n->genus) {
    case GEN_DATA_SPEC:
	if (CHILD0(n) == CHILD1(n))	/* no initializer */
	    fg_local_defs(CHILD0(n), dug, sblk, &end, init);
	else {
	    fg_local_defs(CHILD0(n), dug, sblk, &end, 1);
	    fg_local_defs(CHILD1(n), dug, sblk, &end, 0);
	}
	break;
    case GEN_EXPR:
	{
	    TNODE *p;

	    p = PARENT(n);
	    if (p->genus != GEN_DATA_ITEM)
		fg_expr(n, dug, sblk, &end, NULL, NULL,
			VAR_CUSE, VAR_VOID);
	    /* else: array bound constant - can't instrument here */
	    break;
	}
    case GEN_INDATA_DCL:
	{
	    TNODE *p;
	    int init2;

	    init2 = 0;
	    p = CHILD0(n);
	    if (p->genus != GEN_CLASSTYPES) {
		break;
	    }
	    for (p = CHILD0(p); p != NULL; p = TNEXT(p)) {
		if ((p->species == CLASSTYPE_STATIC) ||
		    (p->species == CLASSTYPE_INLINE) ||
		    (p->species == CLASSTYPE_EXTERN)) {
		    init2 = 1;
		    break;
		}
	    }
	    fg_local_defs(CHILD1(n), dug, sblk, &end, init2);
	    break;
	}
    case GEN_INDATA_DCLS:
	{
	    TNODE *p;

	    for (p = CHILD0(n); p != NULL; p = TNEXT(p))
		fg_local_defs(p, dug, end, &end, init);
	    dug_endblk(dug, end, n);
	    break;
	}
    case GEN_DATA_SPECS:
    case GEN_DATA_ITEM:
    case GEN_INITIALIZER:
    case GEN_INIT_LIST:
    case GEN_INIT_ITEM:
	{
	    TNODE *p;

	    for (p = CHILD0(n); p != NULL; p = TNEXT(p))
		fg_local_defs(p, dug, end, &end, init);
	    break;
	}
    case GEN_NAME:
	if (init || n->sym.sym->type.valtype.sclass & SCB_INIT) {
	    dug_du(dug, n->sym.sym, sblk, VAR_DEF, n->sym.sym->def);
	    /*
	     * If variable is an array or pointer, define the
	     * DREF of the variable.  (Why?)
	     */
	    if (QUAL_ISPTR(n->sym.sym->type.valtype.qual) ||
		QUAL_ISARRAY(n->sym.sym->type.valtype.qual)) {
		dug_du(dug, n->sym.sym, sblk,
		       VAR_DREF | VAR_DEF, n->sym.sym->def);
	    }
	}
	break;
    default:
	break;
    }

    *endblk = end;
    return;
}

static void
init_outtables(FILE *f)
{
    fprintf(f, "v %s\n", VERSION);
    fputs("# v version\n", f);
    fputs("# S source-file-name time-stamp\n", f);
    fputs("# F function-name s-file s-line s-col e-file e-line e-col\n", f);
    fputs("# B s-file s-line s-col e-file e-line e-col\n", f);
    fputs("# V variable-name\n", f);
    fputs("# C variable definition-block C-use-block\n", f);
    fputs("# P variable definition-block P-use-block to-block\n", f);
    fputs("# c formal-number-of-C-uses\n", f);
    fputs("# p formal-number-of-P-uses\n", f);
    return;
}
