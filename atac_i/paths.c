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

static const char paths_c[] = 
	"$Header: /users/source/archives/atac.vcs/atac_i/RCS/paths.c,v 3.9 1997/05/12 00:38:53 tom Exp $";
/*
* $Log: paths.c,v $
* Revision 3.9  1997/05/12 00:38:53  tom
* fix most gcc warnings
*
* Revision 3.8  1997/05/10 23:19:35  tom
* absorb srcpos.h into error.h
*
* Revision 3.7  1996/11/12 23:31:58  tom
* change ident to 'const' to quiet gcc
* add forward-ref prototypes
*
* Revision 3.6  1995/12/27 23:32:30  tom
* don't use NULL for int value!
*
* Revision 3.5  94/04/05  13:19:00  saul
* Get rid of $length$ field in .atac.  atacysis doesn't understand it.
* 
* Revision 3.4  94/04/04  10:13:39  jrh
* Add Release Copyright
* 
* Revision 3.3  93/08/04  15:46:57  ewk
* Added MVS and solaris support.  Squelched some ANSI warnings.
* 
* Revision 3.2  93/07/12  10:58:47  saul
* MVS MODULEID
* MVS node_srcpos_i,tFindDefValue,bitvector==>node_isrcpos,tFindVDef,bitvec
* 
* Revision 3.1  93/03/31  11:41:33  saul
* Void function should be int.
* 
* Revision 3.0  92/11/06  07:45:21  saul
* propagate to version 3.0
* 
* Revision 2.11  92/11/04  15:58:46  saul
* remove unused variable fanout
* 
* Revision 2.10  92/11/02  11:37:34  saul
* remove unused variables
* 
* Revision 2.9  92/10/30  09:48:33  saul
* include portable.h
* 
* Revision 2.8  92/10/09  08:24:02  saul
* #include <ctypes.h> for portability
* 
* Revision 2.7  92/09/22  15:22:48  saul
* Add length field to literal text that may contain blank.
* 
* Revision 2.6  92/09/08  08:02:16  saul
* Use case label text in place of value when available.
* 
* Revision 2.5  92/07/15  10:22:00  saul
* parse_pos bug
* 
* Revision 2.4  92/07/10  12:25:39  saul
* changes for new display and infeasable detection
* 
* Revision 2.3  92/04/07  09:00:23  saul
* remove ATACYSIS #ifdefs
* 
* Revision 2.2  92/03/17  14:22:41  saul
* copyright
* 
* Revision 2.1  91/06/13  12:39:14  saul
* Propagate to version 2.0
* 
* Revision 1.1  91/06/12  20:25:47  saul
* Aug 1990 baseline
* 
*-----------------------------------------------end of log
*/
#if HAVE_STDLIB_H
#include <stdlib.h>
#endif
#include <stdio.h>
#include <ctype.h>
#include <string.h>

#include "portable.h"
#include "error.h"
#include "tnode.h"
#include "sym.h"
#include "dug.h"
#include "bitvec.h"

/* forward declarations */
static int feasableBranch P_(( BRANCH *br ));
static void pathCount P_(( DUG *dug, BLOCK *node, int *counts ));
static void paths_from P_(( DUG *dug, BLOCK *node, int feasableOnly ));
static void print_dupath P_(( int use_type, int sym, BLOCK *def_node, BRANCH *branch, BLOCK *prev_node, TNODE * dPos, TNODE * uPos ));
static void uCount P_(( DUG *dug, BRANCH *branch, BVPTR *list, DU *def, int p_use, int *counts ));
static void u_traverse P_(( DUG *dug, BRANCH *branch, BVPTR *list, BLOCK *d_node, DU *def, int p_use, BLOCK *prev_node, TNODE * prevPos, int feasableOnly ));

static char	*fname;
static FILE	*outfile;

void
paths(dug, f, feasableOnly)
DUG	*dug;
FILE	*f;
int	feasableOnly;
{ 
	LIST	*t;
	BLOCK	*node;
	int	counts[2];
	
	dug_var_table(dug, f);

	fname = dug->fname;
	outfile = f;

	counts[0] = 0;
	counts[1] = 0;

	/*
	* Find DU paths from each node in flow graph.
	*/
	if (dug->block_list)
		for (t = 0; LIST_NEXT(dug->block_list, &t, &node);) {
		    pathCount(dug, node, counts);
		    paths_from(dug, node, feasableOnly);
		}
	fprintf(outfile, "c %d\n", counts[0]);
	fprintf(outfile, "p %d\n", counts[1]);
}

static void
paths_from(dug, node, feasableOnly)
DUG	*dug;
BLOCK	*node;
int	feasableOnly;
{
	LIST		*i;
	LIST		*j;
	BRANCH		*f;
	DU		*def;
	BVPTR		*v_list;
	CONST_VALUE	value;

	/*
	* For each symbol defined at node
	* traverse graph down from node to find C-USEs and P-USEs.
	* U_traverse adds each node to v_list when it is visited. 
	*/
	for (i = 0; (def = du_use(dug, node, &i)) != 0;) {
		if ((def->ref_type & VAR_DEF) == 0) continue;
		tFindVDef(def->defPos, &value);
		if (value.type == CONST_VT)
		    ((TNODE *)def->defPos)->sym.sym->constValue = &value;
		v_list = BVALLOC(dug->count);
		if (node->branches)
			for (j = 0; LIST_NEXT(node->branches, &j, &f);) {
			    if (feasableOnly && !feasableBranch(f)) continue;
			    u_traverse(dug, f, v_list, node, def, def->ref_type,
				       node, def->usePos, feasableOnly);
			}
		free(v_list);
		if (value.type == CONST_VT)
		    ((TNODE *)def->defPos)->sym.sym->constValue = NULL;
	}
}

static void
u_traverse(dug, branch, list, d_node, def, p_use, prev_node, prevPos,
	   feasableOnly)
DUG*	dug;			/* flow graph */
BRANCH*	branch;			/* branch to possible use node */
BVPTR*	list;			/* list of nodes visited already */
BLOCK*	d_node;			/* defining node */
DU*	def;			/* symbol definition */
int	p_use;			/* use type of previous node */
BLOCK*	prev_node;		/* possible use node */
TNODE*	prevPos;		/* parse position of prev_node; */
int	feasableOnly;
{
	LIST*	i;
	BRANCH*	f;
	DU*	use;
	int	use_type;
	TNODE*	usePos;

if (branch->to->block_id == 0) return;
/*
* ?unknown? Block 0 is the start block.  But, branch back to block 0 means return.
* This should be a valid P-USE but the runtime won't catch it so we don't
* report it.  Since block 0 always has exactly one branch, to block 1,
* and block 1 is on the visited list, and there are never any Uses at block 0,
* we could just remove the line above this comment to have the "P-USE at
* return" print.
*/
	/*
	* If PUSE in previous node, print PUSE here.
	*/
	if (p_use & VAR_PUSE)
		print_dupath(VAR_PUSE, def->var_id + 1, d_node, branch,
			     prev_node, def->defPos, prevPos);

	/*
	* Already visited this node ?
	*/
	if (BVTEST(list, branch->to->block_id)) return;
	BVSET(list, branch->to->block_id);

	use = du_use_type(dug, branch->to, def->var_id + 1, def->ref_type);
	if (use) {
	    use_type = use->ref_type;
	    usePos = use->usePos;
	}
	else {
	    use_type = 0;
	    usePos = 0;
	}

	/*
	* If C-USE at node, print it.
	*/
	if (use_type & VAR_CUSE)
		print_dupath(VAR_CUSE, def->var_id + 1, d_node, branch,
			     prev_node, def->defPos, usePos);
	
	/*
	* Defining use?
	*/
	if (use_type & VAR_DEF) return;

	/*
	* Visit each node reachable from node.
	*/
	if (branch->to->branches)
		for (i = 0; LIST_NEXT(branch->to->branches, &i, &f);) {
			if (feasableOnly && !feasableBranch(f)) continue;
			u_traverse(dug, f, list, d_node, def, use_type,
				branch->to, usePos, feasableOnly);
		}
}

static void
print_dupath(use_type, sym, def_node, branch, prev_node, dPos, uPos)
int	use_type;
int	sym;
BLOCK*	def_node;
BRANCH*	branch;
BLOCK*	prev_node;
TNODE*	dPos;
TNODE*	uPos;
{
	int	use_node_id;
	TNODE *	defPos;
	TNODE *	usePos;

	defPos = tFindDef(dPos);

	if (use_type & VAR_CUSE) {
		fprintf(outfile, "C %d %d %d", sym - 1,
			def_node->block_id, branch->to->block_id);
		usePos = uPos;
	} else {
		if (branch->to)
			use_node_id = branch->to->block_id;
		else use_node_id = 0;
		fprintf(outfile, "P %d %d %d %d", sym - 1,
			def_node->block_id, prev_node->block_id, use_node_id);
		usePos = tFindPred(uPos);
	}
	node_isrcpos(defPos, 1, outfile);
	node_isrcpos(defPos, 0, outfile);
	node_isrcpos(usePos, 1, outfile);
	node_isrcpos(usePos, 0, outfile);
	if (use_type & VAR_PUSE) {
		switch(branch->condType)
		{
		case COND_BOOLEAN:
		    if (branch->value) fprintf(outfile, " TRUE");
		    else fprintf(outfile, " FALSE");
		    break;
		case COND_CHAR:
		    if (isprint(branch->value) && branch->value != ' ' &&
			branch->value != '\'')
		    {
		        fprintf(outfile, " '%c'", (char)(branch->value));
		    }
		    else switch(branch->value)
		    {
		    case ' ':
		        fprintf(outfile, " BLANK");
			break;
		    case '\b':
		        fprintf(outfile, " '\\b'");
			break;
		    case '\f':
		        fprintf(outfile, " '\\f'");
			break;
		    case '\n':
		        fprintf(outfile, " '\\n'");
			break;
		    case '\r':
		        fprintf(outfile, " '\\r'");
			break;
		    case '\t':
		        fprintf(outfile, " '\\t'");
			break;
		    case '\v':
		        fprintf(outfile, " '\\v'");
			break;
		    case '\\':
		        fprintf(outfile, " '\\\\'");
			break;
		    case '\"':
		        fprintf(outfile, " '\\\"'");
			break;
		    case '\'':
		        fprintf(outfile, " '\\''");
			break;
		    case '\0':
		        fprintf(outfile, " '\\0'");
			break;
		    case -1:
		        fprintf(outfile, " EOF");
			break;
		    default:
		        fprintf(outfile, " '\\%3.3o'", (unsigned)(branch->value));
			break;
		    }
		    break;
		case COND_SWITCH:
		    {
			TNODE	*p;

			p = (TNODE *)(branch->node);
			p = CHILD0(p);
			if (p->text) {
			    fprintf(outfile, " %s", p->text);
			} else {
			    fprintf(outfile, " %ld", branch->value);
			}
		    }
		    break;
		case COND_ENUM:
		case COND_INT:
		    if (branch->value) fprintf(outfile, " NonZERO");
		    else fprintf(outfile, " ZERO");
		    break;
		case COND_PTR:
		    if (branch->value) fprintf(outfile, " NonNULL");
		    else fprintf(outfile, " NULL");
		    break;
		case COND_SWITCH_DEFAULT:
		    fprintf(outfile, " DEFAULT");
		    break;
		case COND_UNCONDITIONAL:
		    fprintf(outfile, " UNCONDITIONAL");
		    break;
	        default:
		    internal_error(branch->node, "invalid condType %d",
				   branch->condType);
		    break;
		}
	}
	putc('\n',  outfile);
}

/*
* feasableBranch:  Return 0 if the branch cannot be taken; 1 if the branch
*	must be taken; 2 if we don't know.
*/
static int
feasableBranch(br)
BRANCH *br;
{
    CONST_VALUE	value;
    void	*p;

    switch (br->condType)
    {
    case COND_UNCONDITIONAL:
	return 1;
    case COND_BOOLEAN:
    case COND_CHAR:
    case COND_INT:
    case COND_PTR:
    case COND_ENUM:
	evalConstExpr(br->node, &value);
	if (value.type != CONST_VT) return 2;	/* Don't know. */
	if (br->value == 0) {			/* FALSE branch */
	    if (value.fraction == 0) return 1;	/* satisfied. */
	} else {				/* TRUE branch */
	    if (value.fraction != 0) return 1;	/* satisfied. */
	}
	return 0;				/* not satified */
    case COND_SWITCH:
	p = (void *)tFindSwitch(br->node);
	if (p == NULL) return 2;		/* ? Can't find switch */
	evalConstExpr(p, &value);
	if (value.type != CONST_VT) return 2;	/* Don't know. */
	if (br->value == value.fraction) return 1;	/* Match. */
	return 0;
    case COND_SWITCH_DEFAULT:
	return 2;
    }
    return 2;
}

static void
pathCount(dug, node, counts)
DUG	*dug;
BLOCK	*node;
int	*counts;
{
	LIST	*i;
	LIST	*j;
	BRANCH	*f;
	DU	*def;
	BVPTR	*v_list;

	/*
	* For each symbol defined at node
	* traverse graph down from node to find C-USEs and P-USEs.
	* uCount adds each node to v_list when it is visited. 
	*/
	for (i = NULL; (def = du_use(dug, node, &i)) != 0;) {
		if ((def->ref_type & VAR_DEF) == 0) continue;
		v_list = BVALLOC(dug->count);
		if (node->branches)
		    for (j = 0; LIST_NEXT(node->branches, &j, &f);)
		        uCount(dug, f, v_list, def, def->ref_type, counts);
		free(v_list);
	}
}

static void
uCount(dug, branch, list, def, p_use, counts)
DUG	*dug;			/* flow graph */
BRANCH	*branch;		/* branch to possible use node */
BVPTR	*list;			/* list of nodes visited already */
DU	*def;			/* symbol definition */
int	p_use;			/* use type of previous node */
int	*counts;		/* counts of C-uses & P-uses */
{
	LIST*	i;
	BRANCH*	f;
	DU*	use;
	int	use_type;

	if (branch->to->block_id == 0) return;
	/*
	 * ?unknown?  Block 0 is the start block.  But, branch back to block 0
	 * means return.  This should be a valid P-USE but the runtime won't
	 * catch it so we don't report it.  Since block 0 always has exactly
	 * one branch, to block 1, and block 1 is on the visited list, and
	 * there are never any Uses at block 0, we could just remove the line
	 * above this comment to have the "P-USE at return" print.
	 */

	/*
	 * If PUSE in previous node, print PUSE here.
	 */
	if (p_use & VAR_PUSE) ++counts[1];

	/*
	* Already visited this node ?
	*/
	if (BVTEST(list, branch->to->block_id)) return;
	BVSET(list, branch->to->block_id);

	use = du_use_type(dug, branch->to, def->var_id + 1, def->ref_type);
	if (use) {
	    use_type = use->ref_type;
	}
	else {
	    use_type = 0;
	}

	/*
	* If C-USE at node, print it.
	*/
	if (use_type & VAR_CUSE) ++counts[0];
	
	/*
	* Defining use?
	*/
	if (use_type & VAR_DEF) return;

	/*
	* Visit each node reachable from node.
	*/
	if (branch->to->branches)
	    for (i = 0; LIST_NEXT(branch->to->branches, &i, &f);)
	        uCount(dug, f, list, def, use_type, counts);
}
