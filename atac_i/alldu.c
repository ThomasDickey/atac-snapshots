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

static const char alldu_c[] = "$Id: alldu.c,v 3.10 2013/12/08 23:21:40 tom Exp $";
/*
* @Log: alldu.c,v @
* Revision 3.8  1997/12/09 00:41:45  tom
* repair int/SYM* cast with macro ID_SYM.
* correct int/ulong type of prev_adu_count.
*
* Revision 3.7  1997/07/17 18:32:53  tom
* missed a NULL used as int
*
* Revision 3.6  1997/05/11 23:28:56  tom
* remove redundant prototypes, fix compiler warnings for list.c interface
*
* Revision 3.5  1996/11/12 23:45:51  tom
* change ident to 'const' to quiet gcc
* add forward-ref prototypes
*
* Revision 3.4  1995/12/27 23:32:23  tom
* don't use NULL for int value!
*
* Revision 3.3  94/04/04  10:11:40  jrh
* Add Release Copyright
* 
* Revision 3.2  93/08/04  15:43:34  ewk
* Added MVS and solaris support.  Squelched some ANSI warnings.
* 
* Revision 3.1  93/07/12  09:43:25  saul
* MVS MODULEID
* Change bitvector.h bitvec.h for MVS
* 
* Revision 3.0  92/11/06  07:46:01  saul
* propagate to version 3.0
* 
* Revision 2.4  92/10/30  09:47:28  saul
* include portable.h
* 
* Revision 2.3  92/07/10  12:28:00  saul
* change in args to du_use_type
* 
* Revision 2.2  92/03/17  14:22:11  saul
* copyright
* 
* Revision 2.1  91/06/13  12:38:53  saul
* Propagate to version 2.0
* 
 * Revision 1.1  91/06/12  20:25:34  saul
 * Aug 1990 baseline
 * 
*-----------------------------------------------end of log
*/
#if HAVE_STDLIB_H
#include <stdlib.h>
#endif
#include <string.h>
#include <stdio.h>
#include "portable.h"
#include "sym.h"
#include "dug.h"
#include "bitvec.h"

/* forward declarations */
static void u_traverse(BLOCK * node);
static void paths_from(DUG * dug, BLOCK * node);

#ifndef GIVE_UP
#define GIVE_UP	(10*1000*1000)
#endif

typedef struct definfo {
    DUG *dug;			/* flow graph */
    BVPTR *list;		/* list of nodes visited on path */
    BVPTR *fruitless;		/* nodes that do not reach a use */
    BLOCK *d_node;		/* defining node */
    SYM *sym;			/* symbol being traced */
    int star;			/* for DREF symbol */
} DEFINFO;

static unsigned long adu_count;

static DEFINFO definfo;

void
alldu(DUG * dug)
{
    LIST *t;
    BLOCK *node;

    adu_count = 0;

    /*
     * Find DU paths from each node in flow graph.
     */
    if (dug->block_list)
	for (t = 0; LIST_NEXT(dug->block_list, &t, &node);)
	    paths_from(dug, node);

    fprintf(stderr, "%s blocks: %d\t\tAll_du_paths: %lu\n",
	    dug->fname, dug->count, adu_count);
}

static void
paths_from(DUG * dug,
	   BLOCK * node)
{
    LIST *i;
    LIST *j;
    BLOCK *f;
    DU *du;

    /*
     * For each symbol defined at node
     * traverse graph down from node to find C-USEs and P-USEs.
     * U_traverse adds each node to v_list when it is visited. 
     */
    definfo.dug = dug;
    definfo.d_node = node;
    for (i = 0; (du = du_use(dug, node, &i)) != 0;) {
	if ((du->ref_type & VAR_DEF) == 0)
	    continue;
	if (node->branches == NULL)
	    continue;
	definfo.sym = ID_SYM(du->var_id);
	definfo.star = du->ref_type & VAR_DREF;
	definfo.list = BVALLOC(dug->count);
	definfo.fruitless = BVALLOC(dug->count);
	for (j = 0; LIST_NEXT(node->branches, &j, &f);)
	    u_traverse(f);
	free(definfo.list);
	free(definfo.fruitless);
    }
}

static void
u_traverse(BLOCK * node)	/* possible use node */
{
    LIST *i;
    BLOCK *f;
    DU *du;
    unsigned long prev_adu_count;

    if (node->block_id == 0)
	return;
    /*
     * ?unknown?  Block 0 is the start block.  But, branch back to block 0
     * means return.  This should be a valid P-USE but the runtime won't
     * catch it so we don't report it.  Since block 0 always has exactly one
     * branch, to block 1, and block 1 is on the visited list, and there are
     * never any Uses at block 0, we could just remove the line above this
     * comment to have the "P-USE at return" print.
     */
    if (BVTEST(definfo.fruitless, node->block_id))
	return;			/* already visited */

    if (BVTEST(definfo.list, node->block_id))
	return;			/* already visited */

    if (adu_count >= GIVE_UP)
	return;

    prev_adu_count = adu_count;

    du = du_use_type(definfo.dug, node, definfo.sym, definfo.star);

    /*
     * If C-USE at node, count it.
     */
    if (du->ref_type & VAR_CUSE)
	++adu_count;

    /*
     * If P-USE at node, count them with all following nodes.
     */
    if ((du->ref_type & VAR_PUSE) && node->branches)
	for (i = 0; LIST_NEXT(node->branches, &i, &f);)
	    ++adu_count;

    /*
     * If node does not have a defining use, visit each reachable node.
     */
    if (!(du->ref_type & VAR_DEF)) {
	BVSET(definfo.list, node->block_id);
	if (node->branches)
	    for (i = 0; LIST_NEXT(node->branches, &i, &f);)
		u_traverse(f);
	BVCLR(definfo.list, node->block_id);
    }

    if (adu_count == prev_adu_count)
	BVSET(definfo.fruitless, node->block_id);
}
