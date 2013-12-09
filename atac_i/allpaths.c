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

static const char allpaths_c[] = "$Id: allpaths.c,v 3.9 2013/12/08 18:14:57 tom Exp $";
/*
* @Log: allpaths.c,v @
* Revision 3.8  1997/12/09 00:19:02  tom
* int/size_t fixes.
*
* Revision 3.7  1997/05/11 23:45:33  tom
* split-out allpaths.h, fix compiler warnings.
*
* Revision 3.6  1997/05/10 23:20:53  tom
* absorb srcpos.h into error.h
*
* Revision 3.5  1996/11/12 23:45:16  tom
* change ident to 'const' to quiet gcc
* add forward-ref prototypes
*
* Revision 3.4  1995/12/27 23:32:26  tom
* don't use NULL for int value!
*
* Revision 3.3  94/04/04  10:11:46  jrh
* Add Release Copyright
* 
* Revision 3.2  93/08/04  15:43:50  ewk
* Added MVS and solaris support.  Squelched some ANSI warnings.
* 
* Revision 3.1  93/07/12  10:02:34  saul
* MVS
* 
* Revision 3.0  92/11/06  07:46:03  saul
* propagate to version 3.0
* 
* Revision 2.6  92/11/02  11:38:38  saul
* remove unused variables
* 
* Revision 2.5  92/10/30  09:47:30  saul
* include portable.h
* 
* Revision 2.4  92/07/10  14:03:26  saul
* missing #include "sym.h"
* 
* Revision 2.3  92/03/17  14:22:13  saul
* copyright
* 
* Revision 2.2  91/06/19  13:35:38  saul
* avoid malloc(0) (aix doesn't like it.)
* 
* Revision 2.1  91/06/13  12:38:54  saul
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
#include <stdio.h>
#include <ctype.h>

#include "portable.h"

#include "error.h"
#include "sym.h"
#include "dug.h"
#include "allpaths.h"

#define CHECK_MALLOC(p)	((p)?1:internal_error(NULL, "Out of memory"))

/* forward declarations */
static PATH *subpaths(BLOCK * node, unsigned *visited, size_t setsize);
static PATH *mkpath(int end_id, PATH * next, size_t setsize, unsigned *nodes);

#define BPW	32		/* Bits Per Word */
#define LBPW	5		/* Log Bits Per Word */
#define LBYPW	2		/* Log Bytes Per Word */
#define BITTEST(n,s) ((s)[(n)>>LBPW] & (1 << ((n) & (BPW - 1))))
#define BITSET(n,s) ((s)[(n)>>LBPW] |= (1 << ((n) & (BPW - 1))))

static PATH *
mkpath(int end_id,
       PATH * next,
       size_t setsize,
       unsigned *nodes)
{
    PATH *p;
    size_t i;

    p = (PATH *) malloc(sizeof *p + ((setsize - 1) << LBYPW));
    CHECK_MALLOC(p);
    p->end_id = end_id;
    p->next = next;
    if (nodes) {
	for (i = 0; i < setsize; ++i)
	    p->nodes[i] = nodes[i];
    } else
	for (i = 0; i < setsize; ++i)
	    p->nodes[i] = 0;

    return p;
}

static PATH *
subpaths(BLOCK * node,
	 unsigned *visited,
	 size_t setsize)
{
    int node_id;		/* ID of this node. */
    unsigned *v;		/* Nodes already visited including this one. */
    BLOCK *subn;		/* A node reachable from this one. */
    PATH *subp;			/* List of subpaths from here. */
    PATH *loop;			/* List of subpaths looping back to here. */
    int nloop;
    unsigned nloop_subsets;
    unsigned **loop_tab = NULL;
    PATH *simple;		/* List of other subpaths. */
    LIST *t;			/* Loop control. */
    PATH *p;
    PATH *next;
    PATH *sp;			/* Loop control for simple. */
    PATH *lp;			/* Loop control for loop. */
    unsigned i;
    unsigned j;
    int k;
    size_t m;

    node_id = node->block_id;

    /*
     * Already visited? Return list containing only one empty path
     * ending here.
     */
    if (BITTEST(node_id, visited))
	return mkpath(node_id, NULL, setsize, NULL);

    if (node->branches == NULL ||
	list_next(node->branches, NULL, NULL) == 0) {
	/*
	 * No branches: must be a return node.
	 */
	subp = mkpath(0, NULL, setsize, NULL);
	BITSET(node_id, subp->nodes);
	return subp;
    }

    /*
     * Copy visited set and add current node.
     */
    v = (unsigned *) malloc(setsize << LBYPW);
    CHECK_MALLOC(v);
    for (i = 0; i < setsize; ++i)
	v[i] = visited[i];
    BITSET(node_id, v);

    /*
     * Get subpaths for all reachable nodes and partition into
     * "loop" or "simple".
     */
    loop = NULL;
    nloop = 0;
    simple = NULL;
    for (t = 0; LIST_NEXT(node->branches, &t, &subn);) {
	subp = subpaths(subn, v, setsize);
	for (p = subp; p != NULL; p = next) {
	    next = p->next;
	    if (p->end_id == node_id) {
		p->next = loop;
		loop = p;
		++nloop;
	    } else {
		p->next = simple;
		simple = p;
	    }
	}
    }

    free(v);

    /*
     * Copy pointer to node list for each loop path into array.
     */
    if (nloop) {
	loop_tab = (unsigned **) malloc(nloop * sizeof *loop_tab);
	CHECK_MALLOC(loop_tab);
	for (i = 0, lp = loop; lp != NULL; lp = lp->next)
	    loop_tab[i++] = lp->nodes;
    }

    /*
     * Make a list of subpaths from here containing the current node and 
     * each simple path and each simple path combined with each loop path.
     */
    subp = NULL;
    for (sp = simple; sp != NULL; sp = next) {
	next = sp->next;
	BITSET(node_id, sp->nodes);
	if (nloop > BPW) {
	    for (lp = loop; lp != NULL; lp = lp->next) {
		subp = mkpath(sp->end_id, subp, setsize,
			      sp->nodes);
		for (m = 0; m < setsize; ++m)
		    subp->nodes[m] |= lp->nodes[m];
	    }
	    sp->next = subp;
	    subp = sp;
	} else {
	    nloop_subsets = 1 << nloop;
	    for (i = 0; i < nloop_subsets; ++i) {
		subp = mkpath(sp->end_id, subp, setsize,
			      sp->nodes);
		j = i;
		k = 0;
		while (j) {
		    if (j & 1) {
			for (m = 0; m < setsize; ++m)
			    subp->nodes[m] |=
				loop_tab[k][m];
		    }
		    j >>= 1;
		    ++k;
		}

	    }
	    free(sp);
	}
    }

    /*
       * Free loop list.
     */
    for (lp = loop; lp != NULL; lp = next) {
	next = lp->next;
	free(lp);
    }

    return subp;
}

PATH *
allpaths(DUG * dug,
	 FILE *f,
	 int list_them)
{
    LIST *t;
    BLOCK *node;
    size_t setsize;
    unsigned *visited;
    int i;
    int count;
    PATH *p;
    PATH *next;

    /*
     * Skip over block 0.  It is the start block and is
     * also the flag for done.
     */
    t = 0;
    if (LIST_NEXT(dug->block_list, &t, NULL) == 0)
	return NULL;
    if (LIST_NEXT(dug->block_list, &t, &node) == 0)
	return NULL;

    setsize = (dug->count + BPW - 1) >> LBPW;

    if (setsize == 0)
	return NULL;
    visited = (unsigned *) malloc(setsize << LBYPW);
    CHECK_MALLOC(visited);
    for (i = 0; i < setsize; ++i)
	visited[i] = 0;
    BITSET(0, visited);

    count = 0;
    for (p = subpaths(node, visited, setsize); p != NULL; p = next) {
	next = p->next;
	++count;
	free(p);
	if (list_them) {
	    fprintf(f, "%s+", dug->fname);
	    for (i = 0; i < dug->count; ++i)
		if (BITTEST(i, p->nodes))
		    fprintf(f, " %d", i);
	    fprintf(f, "\n");
	}
    }
    fprintf(f, "%s allpaths: %d\n", dug->fname, count);
    return NULL;
}
