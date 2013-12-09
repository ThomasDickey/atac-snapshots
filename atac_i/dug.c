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

static const char dug_c[] = "$Id: dug.c,v 3.16 2013/12/08 18:38:32 tom Exp $";
/*
* @Log: dug.c,v @
* Revision 3.15  2005/08/14 13:43:23  tom
* gcc warnings
*
* Revision 3.14  1998/09/19 14:53:43  tom
* fix some gcc warnings (make the ZIDENT const, prototype aTaC, and bracket
* empty-Zpath data)
*
* Revision 3.13  1997/12/11 01:27:38  tom
* corrected pointer types to compile cleanly.
*
* Revision 3.12  1997/05/11 21:23:30  tom
* corrected prototypes for du_use, du_use_type
*
* Revision 3.11  1997/05/11 19:01:20  tom
* second cut of prototyping, moved prototypes to dug.h, cleanup against fg_expr.c
*
* Revision 3.10  1997/05/10 23:21:18  tom
* absorb srcpos.h into error.h
*
* Revision 3.9  1997/04/25 13:42:31  tom
* fix for SunOS K&R, which declares "int free()"
* casts to appease compiler
*
* Revision 3.8  1996/11/13 00:07:22  tom
* change ident to 'const' to quiet gcc
* add forward-ref prototypes
*
* Revision 3.7  1995/12/27 23:27:11  tom
* don't use NULL for int value!
*
* Revision 3.6  94/04/04  10:12:25  jrh
* Add Release Copyright
* 
* Revision 3.5  93/12/15  12:53:53  saul
* McCabe cyclomatic number calculation
* 
* Revision 3.4  93/08/04  15:44:39  ewk
* Added MVS and solaris support.  Squelched some ANSI warnings.
* 
* Revision 3.3  93/07/12  10:15:02  saul
* MVS ModuleID
* node_srcpos_i ==> node_isrcpos for 7 char uniqueness
* 
* Revision 3.2  93/04/29  07:40:21  saul
* Fix "condition conflict" error.
* 
* Revision 3.1  93/03/30  08:34:47  saul
* Change "version" to "release".
* 
* Revision 3.0  92/11/06  07:45:19  saul
* propagate to version 3.0
* 
* Revision 2.12  92/11/04  15:58:22  saul
* plug du memory leak
* 
* Revision 2.11  92/11/02  11:37:02  saul
* remove unused variables
* 
* Revision 2.10  92/10/30  09:47:42  saul
* include portable.h
* 
* Revision 2.9  92/09/08  08:00:08  saul
* Runtime data structure changes for freq info, ident stamps.
* 
* Revision 2.8  92/07/15  10:30:01  saul
* parse pos bug
* 
* Revision 2.7  92/07/10  14:04:01  saul
* syntax error hidden by #ifdef DEBUG
* 
* Revision 2.6  92/07/10  11:59:00  saul
* changes for new display and infeasable detection
* 
* Revision 2.5  92/04/07  09:00:15  saul
* remove ATACYSIS #ifdefs
* 
* Revision 2.4  92/03/17  14:22:21  saul
* copyright
* 
* Revision 2.3  91/06/20  13:38:26  saul
* bug in dug_clean; possible coredump when a function has no variables.
* 
* Revision 2.2  91/06/19  13:37:40  saul
* avoid malloc(0) (aix doesn't like it.)
* 
* Revision 2.1  91/06/13  12:39:00  saul
* Propagate to version 2.0
* 
* Revision 1.1  91/06/12  20:25:37  saul
* Aug 1990 baseline
* 
*-----------------------------------------------end of log
*/

#include "portable.h"

#include "sym.h"
#include "error.h"
#include "tnode.h"
#include "dug.h"
#include "version.h"

typedef struct {
    SYM *symbol;		/* key. Must be first */
    int id;
} V_ID;

#define TABLE_DATATYPE V_ID
#include "table.h"

/* forward declarations */
static void block_dump(BLOCK * b);
static void branch_print(BRANCH * r);
static void du_dump(DU * du);
static void free_block(BLOCK * b);
static void free_branch(BRANCH * r);
static void print_structs(char *prefix, FILE *f);
static void var_clean(DUG * dug);

#define CHECK_MALLOC(p) ((p)?1:internal_error(NULL, "Out of memory\n"))

#define DUG_MAGIC	((int)dug_create)	/* unlikely number */
#define BRANCH_MAGIC	(DUG_MAGIC + 1)

DUG *
dug_create(void)
{
    DUG *dug;

    dug = (DUG *) malloc(sizeof *dug);
    CHECK_MALLOC(dug);
    dug->magic = DUG_MAGIC;
    dug->fname = 0;
    dug->count = 0;
    dug->block_list = (LIST *) list_create();
    dug->nvar = 0;
    dug->vartab = 0;
    return dug;
}

static void
free_branch(BRANCH * r)
{
#ifdef DEBUG
    if (r->magic != BRANCH_MAGIC)
	internal_error(NULL, "free_branch: memory corrupted\n");
#endif
    r->magic = 0;
    free(r);
}

static void
free_block(BLOCK * b)
{
    if (b->branches)
	LIST_FREE(b->branches, free_branch);
    LIST_FREE(b->du_list, free);
    b->magic = 0;
    free(b);
}

int
dug_free(DUG * dug)
{
    LIST_FREE(dug->block_list, free_block);
    if (dug->vartab)
	free(dug->vartab);
    dug->magic = 0;
    free(dug);

    return 1;
}

BLOCK *
dug_newblk(DUG * dug)
{
    BLOCK *b;

#ifdef DEBUG
    if (dug->magic != DUG_MAGIC)
	internal_error(NULL, "dug_newblk: memory corrupted\n");
#endif

    b = (BLOCK *) malloc(sizeof *b);
    CHECK_MALLOC(b);
    list_put(dug->block_list, b);
    b->magic = DUG_MAGIC;
    b->block_id = dug->count++;
    b->du_list = (LIST *) list_create();
    if (b->du_list == 0) {
	free(b);
	return 0;
    }
    b->branches = 0;
    b->to_count = 0;
    b->parse_start = 0;
    b->parse_end = 0;

    return b;
}

int
dug_startblk(DUG * dug,
	     BLOCK * blk,
	     struct tnode *parse_start)
{
#ifdef DEBUG
    if (dug->magic != DUG_MAGIC)
	internal_error(NULL, "dug_startblk: memory corrupted\n");
#endif
    if (blk == 0)
	return 1;
#ifdef DEBUG
    if (blk->magic != DUG_MAGIC)
	internal_error(NULL, "dug_startblk 2: memory corrupted\n");
#endif

    if (blk->parse_start == 0 || parse_start == 0)
	blk->parse_start = (char *) parse_start;

    return 1;
}

int
dug_fname(DUG * dug,
	  char *fname)
{
#ifdef DEBUG
    if (dug->magic != DUG_MAGIC)
	internal_error(NULL, "dug_startblk 3: memory corrupted\n");
#endif

    dug->fname = fname;

    return 1;
}

int
dug_endblk(DUG * dug,
	   BLOCK * blk,
	   struct tnode *parse_end)
{
#ifdef DEBUG
    if (dug->magic != DUG_MAGIC)
	internal_error(NULL, "dug_endblk: memory corrupted\n");
#endif
    if (blk == NULL)
	return 1;
#ifdef DEBUG
    if (blk->magic != DUG_MAGIC)
	internal_error(NULL, "dug_endblk 2: memory corrupted\n");
#endif

    blk->parse_end = (char *) parse_end;

    return 1;
}

int				/* 0 - failed */
dug_branch(DUG * dug,
	   BLOCK * from,
	   BLOCK * bTo,
	   COND_TYPE condType,
	   long value,
	   void *node)
{
    LIST *t;
    BRANCH *r;

#ifdef DEBUG
    if (dug->magic != DUG_MAGIC)
	internal_error(NULL, "dug_branch: memory corrupted\n");
#endif
    if (from == NULL || bTo == NULL)
	return 0;
#ifdef DEBUG
    if (bTo->magic != DUG_MAGIC)
	internal_error(NULL, "dug_branch bTo: memory corrupted\n");
    if (from->magic != DUG_MAGIC)
	internal_error(NULL, "dug_branch from: memory corrupted\n");
#endif
    if (from == bTo)
	return 1;

#ifdef DGVERIFY
    if (!is_a_block(dug, bTo))
	internal_error(NULL, "dug_branch: to-edge not on edge list");
    if (!is_a_block(dug, from))
	internal_error(NULL, "dg_addedge: from-edge not on edge list");
#endif
    /*
     * Edge already there?
     */
    if (from->branches == NULL)
	from->branches = (LIST *) list_create();
    else
	for (t = NULL; LIST_NEXT(from->branches, &t, &r);)
	    if (r->to == bTo)
		return 1;

    /*
     * New edge.
     */
    r = (BRANCH *) malloc(sizeof *r);
    CHECK_MALLOC(r);
    r->magic = BRANCH_MAGIC;
    r->to = bTo;
    r->condType = condType;
    r->value = value;
    r->node = node;
    if (list_put(from->branches, r)) {
	++bTo->to_count;
	return 1;
    }

    return 0;
}

typedef struct varsym {
    SYM *symbol;
    int ref_type;
} VARSYM;

int
dug_du(DUG * dug,
       SYM * symbol,
       BLOCK * block,
       int ref_type,
       struct tnode *parse_pos)
{
    LIST *i;
    DU *du;

#ifdef DEBUG
    if (dug->magic != DUG_MAGIC)
	internal_error(NULL, "dug_du: dug memory corrupted\n");
#endif
    if (block == NULL)
	return FALSE;
#ifdef DEBUG
    if (block->magic != DUG_MAGIC)
	internal_error(NULL, "dug_du: block memory corrupted\n");
#endif

    /*
     * If symbol already there with same DREF setting, OR ref_types together.
     * Omit C-USE if symbol previously defined at the same node (because
     * this is not a "global" C-USE).
     */
    for (i = 0; LIST_NEXT(block->du_list, &i, &du);)
	if ((du->symbol == symbol) &&
	    ((du->ref_type & VAR_DREF) == (ref_type & VAR_DREF))) {
	    if (du->ref_type & VAR_DEF)
		du->ref_type |= ref_type & ~VAR_CUSE;
	    else
		du->ref_type |= ref_type;
	    if (ref_type & VAR_DEF) {
		du->defPos = parse_pos;
	    }
	    if (ref_type & (VAR_CUSE | VAR_PUSE)) {
		du->usePos = parse_pos;
	    }
	    return TRUE;
	}

    du = (DU *) malloc(sizeof *du);
    CHECK_MALLOC(du);

    du->symbol = symbol;
    du->var_id = VAR_ID(0);
    du->ref_type = ref_type;
    if (ref_type & VAR_DEF) {
	du->defPos = parse_pos;
    } else
	du->defPos = 0;
    if (ref_type & (VAR_CUSE | VAR_PUSE)) {
	du->usePos = parse_pos;
    } else
	du->usePos = 0;

    return list_put(block->du_list, du);
}

#ifdef DGVERIFY
static
is_a_block(DUG * dug,
	   BLOCK * block)
{
    LIST *t;
    BLOCK *b;

    for (t = NULL; LIST_NEXT(dug->block_list, &t, &b);)
	if (b == block)
	    return 1;

    return 0;
}
#endif

static void
branch_print(BRANCH * r)
{
    if (r->magic != BRANCH_MAGIC)
	internal_error(NULL, "branch_print: memory corrupted\n");
    fprintf(stderr, "block %d ", r->to->block_id);
    switch (r->condType) {
    case COND_UNCONDITIONAL:
	fprintf(stderr, "UNCONDITIONAL");
	break;
    case COND_BOOLEAN:
	fprintf(stderr, "BOOLEAN");
	break;
    case COND_CHAR:
	fprintf(stderr, "CHAR");
	break;
    case COND_INT:
	fprintf(stderr, "INT");
	break;
    case COND_PTR:
	fprintf(stderr, "PTR");
	break;
    case COND_ENUM:
	fprintf(stderr, "ENUM");
	break;
    case COND_SWITCH:
	fprintf(stderr, "SWITCH");
	break;
    case COND_SWITCH_DEFAULT:
	fprintf(stderr, "SWITCH_DEFAULT");
	break;
    default:
	fprintf(stderr, "%d ", r->condType);
	break;
    }

    fprintf(stderr, "-%ld ", r->value);

    if (r->node) {
	node_srcpos(r->node, 1, stderr);
	node_srcpos(r->node, 0, stderr);
    }
    fprintf(stderr, "\n");
}

static void
du_dump(DU * du)
{
    fprintf(stderr, "du \"%s\" %d", du->symbol->name, du->ref_type);
    if (du->defPos) {
	fprintf(stderr, " D: ");
	node_srcpos(du->defPos, 1, stderr);
	node_srcpos(du->defPos, 0, stderr);
    }
    if (du->usePos) {
	fprintf(stderr, " U: ");
	node_srcpos(du->usePos, 1, stderr);
	node_srcpos(du->usePos, 0, stderr);
    }
    fprintf(stderr, "\n");
}

static void
block_dump(BLOCK * b)
{
    fprintf(stderr, "Block ");
    if (b->magic != DUG_MAGIC)
	internal_error(NULL, "block_dump: invalid format");
    else {
	fprintf(stderr, "-- id: %d, start: ", b->block_id);
	node_srcpos(b->parse_start, 1, stderr),
	    fprintf(stderr, ", end: ");
	node_srcpos(b->parse_end, 0, stderr),
	    fprintf(stderr, ", to_count: %d\n", b->to_count);
	if (b->branches)
	    LIST_DUMP(b->branches, branch_print, " branches");
	LIST_DUMP(b->du_list, du_dump, " def-use");
    }
}

void
dug_dump(DUG * dug)
{
    char label[100];

    if (dug->magic != DUG_MAGIC)
	internal_error(NULL, "dug_dump error: invalid Dug format");
    else {
	sprintf(label, " \"%s\" Dug Dump", dug->fname);
	LIST_DUMP(dug->block_list, block_dump, label);
    }
}

void
dug_du_combine(DUG * dug,
	       BLOCK * first,
	       BLOCK * second)
{
    LIST *t;
    DU *du;

    for (t = NULL; LIST_NEXT(second->du_list, &t, &du);) {
	if (du->ref_type & VAR_DEF) {
	    dug_du(dug, du->symbol, first,
		   du->ref_type & ~(VAR_CUSE | VAR_PUSE), du->defPos);
	}
	if (du->ref_type & (VAR_CUSE | VAR_PUSE)) {
	    dug_du(dug, du->symbol, first, du->ref_type & ~VAR_DEF,
		   du->usePos);
	}
	free(du);
	list_delete(second->du_list, &t);
    }

    if (first->parse_start == 0)
	first->parse_start = second->parse_start;
    if (second->parse_end)
	first->parse_end = second->parse_end;

    /*
     * Make sure second will get cleaned up.
     */
    if (second->branches) {
	LIST_FREE(second->branches, free_branch);
	second->branches = NULL;
    }
    second->parse_start = 0;
    second->parse_end = 0;
}

static void
var_clean(DUG * dug)
{
    LIST *i;
    LIST *j;
    BLOCK *node;
    DU *du;
    int var_count;
    TABLE *var_list;
    TABLE *dref_list;
    TABLE *list;
    V_ID *var_id;
    VARSYM *vartab;

    var_list = table_create(NULL);
    dref_list = table_create(NULL);

    /*
     * Put variables with at least one use in temporary tables.
     */
    var_id = (V_ID *) malloc(sizeof *var_id);
    CHECK_MALLOC(var_id);
    var_count = 0;
    for (i = 0; LIST_NEXT(dug->block_list, &i, &node);) {
	for (j = 0; LIST_NEXT(node->du_list, &j, &du);) {
	    if ((du->ref_type & (VAR_CUSE | VAR_PUSE)) == 0)
		continue;
	    var_id->symbol = du->symbol;
	    if (du->ref_type & VAR_DREF)
		list = dref_list;
	    else
		list = var_list;
	    if (table_insert(list, var_id, 0)) {
		var_id = (V_ID *) malloc(sizeof *var_id);
		CHECK_MALLOC(var_id);
		++var_count;
	    }
	}
    }
    free(var_id);		/* extra */

    /*
     * Copy temporary tables into final table.
     */
    if (var_count) {
	NODE *n;
	vartab = (VARSYM *) malloc(var_count * sizeof(VARSYM));
	CHECK_MALLOC(vartab);
	var_count = 0;
	for (n = 0; (var_id = table_next(var_list, &n)) != 0;) {
	    vartab[var_count].symbol = var_id->symbol;
	    vartab[var_count].ref_type = VAR_VOID;
	    var_id->id = var_count++;
	}
	for (n = 0; (var_id = table_next(dref_list, &n)) != 0;) {
	    vartab[var_count].symbol = var_id->symbol;
	    vartab[var_count].ref_type = VAR_DREF;
	    var_id->id = var_count++;
	}
	dug->nvar = var_count;
	dug->vartab = vartab;
    }

    /*
     * At each du reference to variable, put var id.
     */
    for (i = 0; LIST_NEXT(dug->block_list, &i, &node);) {
	for (j = 0; LIST_NEXT(node->du_list, &j, &du);) {
	    var_id = table_find((du->ref_type & VAR_DREF)
				? dref_list
				: var_list,
				(TABLE_DATATYPE *) (du->symbol), 0, 0);
	    if (var_id)
		du->var_id = var_id->id;
	    else {
		free(du);
		list_delete(node->du_list, &j);
	    }
	}
    }

    /*
     * Cleanup.
     */
    table_free(var_list, (TabledataFree) free);
    table_free(dref_list, (TabledataFree) free);
}

void
dug_clean(DUG * dug)
{
    LIST *t;
    BLOCK *block;
    BRANCH *r;
    BLOCK *b;
    LIST *i;
    LIST *j;
    BRANCH *rTo;
    BRANCH *new;
    int count;

#ifdef DEBUG
    if (dug->magic != DUG_MAGIC)
	internal_error(NULL, "dug_clean: memory corrupted\n");
#endif

    /*
     * Bypass empty blocks:
     * Look at each block for branches to an "empty" block.
     * If found, replace the branch with a branch to the target of
     * the empty block and reduce the reference count to the empty
     * block.  Replacement branches are added at the end of the
     * branch list, so they will be checked in case of two empty
     * blocks in a row.
     * Empty blocks removed from the branch list of block are
     * marked as visited for block.  An empty block will not be removed
     * if it has a branch to a block already visited.
     */
    for (t = NULL; LIST_NEXT(dug->block_list, &t, &block);)
	block->visited = NULL;
    for (t = NULL; LIST_NEXT(dug->block_list, &t, &block);) {
	if (block->branches == NULL)
	    continue;
	block->visited = block;
	for (j = 0; LIST_NEXT(block->branches, &j, &r);) {
#ifdef DEBUG
	    if (r->magic != BRANCH_MAGIC)
		internal_error(NULL, "dug_clean: r memory corrupted\n");
#endif
	    if (r->to->parse_start && r->to->parse_end)
		continue;
	    if (LIST_NEXT(r->to->du_list, NULL, NULL)) {
		LIST *i2;
		DU *du;

		for (i2 = 0; LIST_NEXT(r->to->du_list, &i2, &du);) {
		    if (du->symbol != &decis_sym)
			internal_error(NULL, "dug: unexpec du_list");
		    list_put(block->du_list, du);
		    list_delete(r->to->du_list, &i2);
		}
	    }

	    if (r->to->branches) {
		rTo = 0;
		/*
		 * Multiple branches from "empty" node possible as in
		 * "if (f()) ...".
		 */
		for (i = 0; LIST_NEXT(r->to->branches, &i, &rTo);)
		    if (rTo->to->visited == block)
			break;
		if (rTo && rTo->to->visited == block)
		    continue;
		for (i = 0; LIST_NEXT(r->to->branches, &i, &rTo);) {
		    new = (BRANCH *) malloc(sizeof *new);
		    CHECK_MALLOC(new);
		    if (r->condType == COND_UNCONDITIONAL)
			*new = *rTo;
		    else {
			*new = *r;
			new->to = rTo->to;
		    }
		    list_put(block->branches, new);
		    ++rTo->to->to_count;
		}
	    }
	    list_delete(block->branches, &j);
	    --r->to->to_count;
	    r->to->visited = block;
	    free(r);
	}
    }

    /*
     * Delete empty blocks and renumber.
     */
    count = 0;
    for (t = 0; LIST_NEXT(dug->block_list, &t, &b);) {
	if ((b->to_count == 0) &&
	    (b->parse_end == 0 || b->parse_start == 0)) {
	    if (b->branches)
		for (i = 0; LIST_NEXT(b->branches, &i, &r);)
		    --r->to->to_count;
	    free_block(b);
	    list_delete(dug->block_list, &t);
	} else
	    b->block_id = count++;
    }
    dug->count = count;

    var_clean(dug);
}

void
dug_tables(DUG * dug,
	   int funcno,
	   char *prefix,
	   FILE *f)
{
    static int first_call = 1;
    LIST *i;
    LIST *j;
    BLOCK *node;
    DU *du;
    int du_count;
    int blk_count;
    char *filename;
#ifndef NOCOMMENT
    int n;
#endif

    if (first_call) {
	int i2;
	filename = srcfname(0);
	if (filename == 0)
	    filename = "-";

	/* gcc complains about unused ZINDENT unless we make it const */
	fprintf(f, "#undef ATAC_CONST\n");
	fprintf(f, "#ifdef __GNUC__\n");
	fprintf(f, "#define ATAC_CONST const\n");
	fprintf(f, "#else\n");
	fprintf(f, "#define ATAC_CONST /*nothing*/\n");
	fprintf(f, "#endif /* __GNUC__ */\n");

	fprintf(f, "static ATAC_CONST char %sIDENT[] = \"$Log: @(#)", prefix);
	fprintf(f, "%.*s", (int) strlen(filename) - 2, filename + 1);
	fprintf(f, " instrumented by ATAC release %s$\";\n",
		RT_VERSION);

	/* quiet gcc warnings about implicit declaration of function */
	fprintf(f, "#if __STDC__\n");
	fprintf(f, "extern aTaC(int level, long block);\n");
	fprintf(f, "#endif\n");

	print_structs(prefix, f);

	/*
	 * Print filename table.
	 */
	fprintf(f, "static struct %sfiles %sF[] = { /* FILE */\n",
		prefix, prefix);
	for (i2 = 0; (filename = srcfname(i2)); ++i2)
	    fprintf(f, "\t{ %s, %d },\n", filename, srcfstamp(i2));
	fprintf(f, "\t{ 0, 0 }\n");
	fprintf(f, "};\n");

	first_call = 0;
    }

    /*
     * Print DU table.
     */
    fprintf(f, "static struct %spath %sU%s[] = { /* DU */\n",
	    prefix, prefix, dug->fname);
    du_count = 0;
    for (i = 0; LIST_NEXT(dug->block_list, &i, &node);) {
	for (j = 0; LIST_NEXT(node->du_list, &j, &du);) {
	    fprintf(f, "\t{%d, %d},",
		    du->var_id, du->ref_type & ~VAR_DREF);
#ifndef NOCOMMENT
	    fprintf(f, "\t/* %d:%d:%s */\n",
		    du_count, node->block_id,
		    du->symbol ? du->symbol->name : "");
#else
	    fputc('\n', f);
#endif
	    ++du_count;
	}
    }
    if (du_count == 0)		/* Can't init to nothing. */
	fprintf(f, "\t{0, 0} /*nothing*/\n");
    fprintf(f, "};\n");

    /*
     * Print BLK table.
     */
    fprintf(f, "static struct %spath *%sB%s[] = { /* BLK */\n",
	    prefix, prefix, dug->fname);
    blk_count = 0;
    du_count = 0;
#ifndef NOCOMMENT
    n = 0;
#endif

    fprintf(f, "\t&%sU%s[0],\n", prefix, dug->fname);
    for (i = 0; LIST_NEXT(dug->block_list, &i, &node);) {
	for (j = 0; LIST_NEXT(node->du_list, &j, &du);)
	    ++du_count;
	fprintf(f, "\t&%sU%s[%d],", prefix, dug->fname, du_count);
#ifndef NOCOMMENT
	fprintf(f, "\t/* %d:%d-%d */\n",
		node->block_id, n, du_count - 1);
	n = du_count;
#else
	fputc('\n', f);
#endif
	++blk_count;
    }
    fprintf(f, "};\n");

    /*
     * Print blkCount[] declaration.
     */
    fprintf(f, "static unsigned int %sC%s[%d];\n",
	    prefix, dug->fname, blk_count);

    /*
     * Print root table.
     */
    fprintf(f, "static struct %stables %sT%s", prefix, prefix, dug->fname);
    fprintf(f, " = { %sF, \"%s\", 0, %d, %d, %d, %sB%s, 0, %sC%s};\n",
	    prefix, RT_VERSION, funcno, blk_count, dug->nvar,
	    prefix, dug->fname, prefix, dug->fname);
}

static void
print_structs(char *prefix,
	      FILE *f)
{
    fprintf(f, "struct %spath {\n", prefix);
    fprintf(f, "	unsigned short		var;\n");
    fprintf(f, "	unsigned short		type;\n");
    fprintf(f, "	char			*path;\n");
    fprintf(f, "};\n");

    fprintf(f, "struct %sfiles {\n", prefix);
    fprintf(f, "	char	*name;\n");
    fprintf(f, "	int	stamp;\n");
    fprintf(f, "};\n");
    fprintf(f, "struct %stables {\n", prefix);
    fprintf(f, "	struct %sfiles	*files;\n", prefix);
    fprintf(f, "	char		version[2 * sizeof(char *)];\n");
    fprintf(f, "	unsigned short	fileId;\n");
    fprintf(f, "	unsigned short	funcno;\n");
    fprintf(f, "	unsigned short	nblk;\n");
    fprintf(f, "	unsigned short	nvar;\n");
    fprintf(f, "	struct %spath	**blk;\n", prefix);
    fprintf(f, "	struct %stables	*next;\n", prefix);
    fprintf(f, "	unsigned int	*blkCounts;\n");
    fprintf(f, "};\n");
}

int
dug_blocks(DUG * dug,
	   FILE *f)
{
    BLOCK *b;
    LIST *t;

#ifdef DEBUG
    if (dug->magic != DUG_MAGIC)
	internal_error(NULL, "dug_blocks: memory corrupted\n");
#endif

    if (dug->fname == 0)
	return FALSE;

    /*
     * Function info.
     */
    if (LIST_NEXT(dug->block_list, NULL, &b)) {
	fprintf(f, "F %s", dug->fname);
	node_isrcpos(b->parse_end, 1, f),
	    fprintf(f, " ");
	node_isrcpos(b->parse_end, 0, f),
	    fprintf(f, "\n");
    }

    /*
     * Block info.
     */
    for (t = 0; LIST_NEXT(dug->block_list, &t, &b);) {
	fprintf(f, "B");
	node_isrcpos(b->parse_start, 1, f),
	    fprintf(f, " ");
	node_isrcpos(b->parse_end, 0, f),
	    fprintf(f, "\n");
    }

    return TRUE;
}

/*
* dug_cyclomatic: Calculate McCabe's cyclomatic number.
*/
int
dug_cyclomatic(DUG * dug)
{
    BLOCK *b;
    LIST *t;
    LIST *j;
    int cyclomatic;

#ifdef DEBUG
    if (dug->magic != DUG_MAGIC)
	internal_error(NULL, "dug_blocks: memory corrupted\n");
#endif

    if (dug->fname == 0)
	return FALSE;

    cyclomatic = 1;
    for (t = 0; LIST_NEXT(dug->block_list, &t, &b);) {
	if (b->branches) {
	    j = 0;
	    LIST_NEXT(b->branches, &j, NULL);
	    while (LIST_NEXT(b->branches, &j, NULL))
		++cyclomatic;
	}
    }

    return cyclomatic;
}

DU *
du_use(DUG * dug,
       BLOCK * node,
       LIST ** n)
{
    DU *du;

    if (node == 0)
	return 0;
    if (node->du_list == 0)
	return 0;

    if (!LIST_NEXT(node->du_list, n, &du))
	return 0;
    if (du == 0)
	return 0;

    return du;
}

DU *
du_use_type(DUG * dug,
	    BLOCK * node,
	    SYM * symbol,
	    int mode)
{
    LIST *i;
    DU *du;

    if (node == 0)
	return 0;
    if (node->du_list == 0)
	return 0;

    for (i = 0; LIST_NEXT(node->du_list, &i, &du);)
	if ((du->var_id == VAR_ID(symbol)) &&
	    ((du->ref_type & VAR_DREF) == (mode & VAR_DREF))) {
	    return du;
	}

    return 0;
}

void
dug_var_table(DUG * dug,
	      FILE *f)
{
    int i;

    for (i = 0; i < dug->nvar; ++i)
	fprintf(f, "V %s\n", dug->vartab[i].symbol->name);
}
