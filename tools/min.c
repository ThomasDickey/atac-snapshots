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
#ifdef MVS
#include <mvapts.h>
MODULEID(%M%,%J%/%D%/%T%)
#endif /* MVS */

static char min_c[] =
"$Header: /users/source/archives/atac.vcs/tools/RCS/min.c,v 3.10 1994/04/04 15:06:20 saul Exp $";
static char bellcoreCopyRight[] =
"Copyright (c) 1993 Bell Communications Research, Inc. (Bellcore)";

/*
*-----------------------------------------------$Log: min.c,v $
*-----------------------------------------------Revision 3.10  1994/04/04 15:06:20  saul
*-----------------------------------------------FROM_KEYS
*-----------------------------------------------
*Revision 3.10  94/04/04  15:06:20  saul
*Fix binary copyright.
*
*Revision 3.9  94/04/04  10:52:50  jrh
*Add Release Copyright
*
*Revision 3.8  93/08/11  10:01:54  saul
*missing #endif
*
*Revision 3.7  93/08/10  15:41:53  ewk
*Fixed definition of time_t for vms, MVS, and unix.
*
*Revision 3.6  93/08/09  12:37:19  saul
*recover revision 3.4 changes
*
*Revision 3.5  1993/08/04  15:50:36  ewk
*Added MVS and solaris support.  Squelched some ANSI warnings.
*
*Revision 3.4  93/06/30  15:07:31  saul
*Fix atacmin -K (minimize -s) rounding algorithm to match atac -s
*
*Revision 3.3  93/04/21  08:23:50  saul
*Core dump when each initial test has at least one unique attribute.
*
*Revision 3.2  93/03/31  11:44:49  saul
*#include <sys/types.h> for portablility.
*
*Revision 3.1  92/12/30  09:13:06  saul
*Rewrite.  Errors fixed.  More efficient.  Ordered output.  -s option.
*
*-----------------------------------------------end of log
*/
#include <stdio.h>
#include "portable.h"
#ifdef vms
#include <types.h>
#else /* not vms */
#ifdef MVS
#include <time.h>		/* for time_t */
#else /* not MVS */
#include <sys/types.h>		/* for time_t */
#endif /* not MVS */
#endif /* not vms */

/*
* Minimize:  This program takes as input bit vector representations of a
*	list of sets and outputs a list of names of sets on a minimum sized
*	sub-list which covers all the elements covered by the input list.
*	The algorithm used determines which sets must be kept and which ones
*	are definitely not needed.  It then recursively tries keeping or
*	discarding various sets. An integer argument may be provided to limit
*	the number of recursions.  If the recursion limit is exhausted,
*	the output list may not be minimal.
*/

/* forward declarations */
static struct setList *sl_cost0();
static struct setList *sl_Rminimize();
static struct setList *sl_minimize();
static void sl_dump();
static void sl_cumPrint();
static struct setVector *sl_union();
static void sl_print();
static struct setList *sl_reduce();
static void sl_dontNeed();
static int sl_mustKeep();
static int sl_coveredByOthers();
static void sl_gBN();
static int sl_1stGBN();
static void sl_greedy();
static int sl_firstGreedy();
static float sl_normalizeCost();
static struct setList *sl_get();
static void sl_compress();
static void sl_append();
static void sl_combine();
static struct setList *sl_copy();
static void sl_freeAll();
static void sl_free();
static void sl_realloc();
static struct setList *sl_create();
static int sv_empty();
static int sv_card();
static void sv_compress();
static void sv_dump();
static int sv_subset();
static void sv_subtract();
static struct setVector *sv_copy();
static void sv_free();
static struct setVector *sv_get();
static char *ckRealloc();
static char *ckMalloc();
static void usage();

#define ALLOC_SIZE 512			/* number of longs per allocation */

typedef struct setVector {
	char		*sv_name;
	long		sv_cost;
	int		sv_contentsSize;
	unsigned long	sv_contents[1];
} setVector;

typedef struct setList {
	setVector	**sl_sets;
	int		sl_alloc;
	int		sl_count;
	long		sl_cost;
} setList;

/*
* Globals.
*/
static char		*g_nodeId;
static int		g_maxLevel;
static unsigned long	g_visited;
static unsigned long	g_recursionLimit;
static unsigned long	g_printFreq;
static int		g_allMin = 0;
static boolean		g_greedyFlag = FALSE;
static boolean		g_gBNFlag = FALSE;
static boolean		g_quiet = FALSE;

static void
usage(cmd)
char	*cmd;
{
    fprintf(stderr, "Usage: %s [-aCgGhnqrs] [-l limit] [-c cost] [-p pfreq] [-R restart] [sets]\n", cmd);
    fprintf(stderr, "\t -a print locations of all minimums.\n");
    fprintf(stderr, "\t -c Solution must cost less than this.\n");
    fprintf(stderr, "\t -C Compress for efficiency. (output order random)\n");
    fprintf(stderr, "\t -g for greedy first selection.\n");
    fprintf(stderr, "\t -G for greedy on bottle necks first selection.\n");
    fprintf(stderr, "\t -h Suppress header (with -s).\n");
    fprintf(stderr, "\t -n for names only.\n");
    fprintf(stderr, "\t -l sets recursion limit.\n");
    fprintf(stderr, "\t -p log2 of number of nodes visited between prints.\n");
    fprintf(stderr, "\t -q quiet.  Don't print stats.\n");
    fprintf(stderr, "\t -r Try keep branch before discard branch.\n");
    fprintf(stderr, "\t -R restart from restart vector.\n");
    fprintf(stderr, "\t -s Print cumulative coverage and cost.\n");
}

static char *
ckMalloc(n)
int	n;
{
    register char	*p;

    p = (char *)malloc(n);
    if (p == NULL) {
	fprintf(stderr, "can't malloc\n");
	exit(1);
    }

    return p;
}

static char *
ckRealloc(q, n)
char	*q;
int	n;
{
    register char	*p;

    if (q == NULL) {
	p = (char *)malloc(n);
	if (p == NULL) {
	    fprintf(stderr, "can't malloc\n");
	    exit(1);
	}
    } else {
	p = (char *)realloc(q, n);
	if (p == NULL) {
	    fprintf(stderr, "can't realloc\n");
	    exit(1);
	}
    }

    return p;
}

/*
* sv_get: Read in a set.
*/
static setVector *
sv_get(f, pSetSize)
FILE	*f;
int	*pSetSize;
{
	static unsigned long	*contents = NULL;
	static int		contentsSize = 0;
	int		c;
	int		i;
	int		index;
	unsigned long	guide;
	unsigned long	vector;
	setVector	*set;
	char		name[200];
	int		nameLen;
	long		cost;
	int		setSize;

	/*
	 * Read set name.
	 */
	nameLen = 0;
	while ((c=getc(f)) != ':') {
		if (c == EOF) {
			if (nameLen == 0) {
			    free(contents);
			    contentsSize = 0;
			    return NULL;
			}
			fprintf(stderr, "missing colon after name\n");
			exit(1);
		}
		if (c == '\n') {
			fprintf(stderr, "missing colon after name\n");
			exit(1);
		}
		name[nameLen++] = c;
		if (nameLen == sizeof name) --nameLen;	/* Too long, truncate.*/
	}
	name[nameLen++] = '\0';

	/*
	 * Read set cost.
	 */
	cost = 0;
	while ((c=getc(f)) != ':') {
		if (c == EOF || c == '\n') {
			fprintf(stderr, "missing colon after cost\n");
			exit(1);
		}
		cost = cost * 10 + c - '0';
	}
	
	/*
	 * Read set vector.  Vector collects bits until full.  Then vector is
	 * copied into contents[] and set back to 0.  Guide indicates the next
	 * bit position in vector to get set. Vector is full when guide
	 * becomes 0 (the 1 gets shifted off the end).  (This should work
	 * for any size long.)
	 */
	for (i = 0; i < contentsSize; ++i)
	    contents[i] = 0;
	setSize = 0;
	index = 0;
	guide = 1;
	vector = 0;
	while ((c=getc(f)) != '\n') {
	    ++setSize;
	    switch (c)
	    {
	    case ' ':
		break;
	    case 'x':
		vector |= guide;
		break;
	    case EOF:
		fprintf(stderr, "unexpected end of file\n");
		exit(1);
	    default:
		fprintf(stderr, "bad character: %c\n", c);
		exit(1);
	    }
	    guide <<= 1;
	    if (guide == 0) {
		/*
		 * Alloc more space if necessary.
		 */
		while (contentsSize <= index) {
		    contentsSize += ALLOC_SIZE;
		    contents = (unsigned long *)
			ckRealloc(contents, contentsSize * sizeof *contents);
		    for (i = contentsSize - ALLOC_SIZE; i < contentsSize; ++i)
			contents[i] = 0;
		}
		/*
		 * Store vector in contents[].
		 */
		contents[index++] = vector;
		guide = 1;
		vector = 0;
	    }
	}
	/*
	 * Alloc more space if necessary.
	 */
	while (contentsSize <= index) {
	    contentsSize += ALLOC_SIZE;
	    contents = (unsigned long *)
		ckRealloc(contents, contentsSize * sizeof *contents);
	    for (i = contentsSize - ALLOC_SIZE; i < contentsSize; ++i)
		contents[i] = 0;
	}
	/*
	 * Store vector in contents[].
	 */
	contents[index++] = vector;
	while (index > 0 && contents[index - 1] == 0)
	    --index;
	
	/*
	 * Allocate set and copy contents into it.
	 */
	set = NULL;
	set = (setVector *)ckMalloc((int)&set->sv_contents[index]);
	set->sv_name = (char *)ckMalloc(strlen(name) + 1);
	strcpy(set->sv_name, name);
	set->sv_cost = cost;
	set->sv_contentsSize = index;
	for (i = 0; i < index; ++i)
		set->sv_contents[i] = contents[i];

	if (pSetSize != NULL) *pSetSize = setSize;

	return set;
}

static void
sv_free(set)
setVector *set;
{
	free(set->sv_name);
	free(set);
}

static setVector *
sv_copy(set)
setVector *set;
{
	int		i;
	setVector	*new;

	new = NULL;
	new = (setVector *)ckMalloc(&new->sv_contents[set->sv_contentsSize]);

	new->sv_name = (char *)ckMalloc(strlen(set->sv_name) + 1);
	strcpy(new->sv_name, set->sv_name);
	new->sv_cost = set->sv_cost;
	new->sv_contentsSize = set->sv_contentsSize;
	for (i = 0; i < new->sv_contentsSize; ++i)
		new->sv_contents[i] = set->sv_contents[i];

	return new;
}

/*
* sv_subtract: Remove members of setB from setA.
*/
static void
sv_subtract(setA, setB)
setVector *setA;
setVector *setB;
{
	int	i;
	int	size;

	if (setA->sv_contentsSize < setB->sv_contentsSize)
		size = setA->sv_contentsSize;
	else size = setB->sv_contentsSize;

	for (i = 0; i < size; ++i)
		setA->sv_contents[i] &= ~setB->sv_contents[i];
}

/*
* sv_subset: Return TRUE if the intersection of setB and "select"
* is equal to or a subset of the intersection of setA and "select".
*/
static boolean
sv_subset(setA, setB, select)
setVector *setA;
setVector *setB;
setVector *select;
{
	unsigned long	a;
	int		i;
	int		size;

	size = setB->sv_contentsSize;

	for (i = 0; i < size; ++i) {
		if (i < select->sv_contentsSize) {
			a = ~select->sv_contents[i];
			if (i < setA->sv_contentsSize)
			    a |= setA->sv_contents[i];
		}
		else a = 0;
		if (setB->sv_contents[i] & ~a) return FALSE;
	}
	return TRUE;
}


static void
sv_dump(set)
setVector *set;
{
    int i;
    unsigned long	guide;
    unsigned long	vector;

    fprintf(stdout, "%s:%d:", set->sv_name, set->sv_cost); 
    for (i = 0; i < set->sv_contentsSize; ++i) {
	vector = set->sv_contents[i];
	for (guide = 1; guide != 0; guide <<= 1) {
	    if (vector & 1) putc('x', stdout);
	    else putc(' ', stdout);
	    vector >>= 1;	/* (Sign extension doesn't matter here.) */
	}
    }
    fprintf(stdout, "\n");
}

/*
* sv_compress: Squeeze out bit positions in set that are 0 in "select".
*/
static void
sv_compress(set, select)
setVector *set;
setVector *select;
{
    int			i;
    int			nIndex;
    unsigned long	guide;
    unsigned long	nGuide;
    unsigned long	vector;

    nIndex = 0;
    nGuide = 1;

    for (i = 0; i < select->sv_contentsSize; ++i) {
	if (i >= set->sv_contentsSize) break;
	vector = set->sv_contents[i];
	set->sv_contents[i] = 0;
	for (guide = 1; guide != 0; guide <<= 1) {
	    if (select->sv_contents[i] & guide) {
		if (vector & guide) {
		    set->sv_contents[nIndex] |= nGuide;
		}
		nGuide <<= 1;
		if (nGuide == 0) {
		    ++nIndex;
		    nGuide = 1;
		}
	    }
	}
    }

    if (nGuide != 1) ++nIndex;

    set->sv_contentsSize = nIndex;

    /* Don't realloc because this makes it hard to free sets at end
     * (they may not be at the same address anymore).
     * set->sv_contents = (unsigned long *)
     *     ckRealloc(set->sv_contents, nIndex * sizeof *set->sv_contents);
     */

    return;
}

/*
* sv_card: Return the number of bits set in both set and select.
*/
static int
sv_card(set, select)
setVector *set;
setVector *select;
{
    int		i;
    long	card;
    int		guide;
    byte	*setContents;
    byte	*selectContents;
    byte	*end;
    int		c;
    static byte	cardCounter[LOBYTE(~0) + 1] = {0, 0};

    /*
    * Initialize cardCounter[i] to number of bits in i.
    */
    if (cardCounter[1] == 0) {
	for (i = 0; i < sizeof cardCounter; ++i) {
	    card = 0;
	    for (guide = 1; guide <= i; guide <<= 1)
		if (i & guide) ++card;
	    cardCounter[i] = card;
	}
    }

    setContents = (byte *)set->sv_contents;
    selectContents = (byte *)select->sv_contents;

    if (select->sv_contentsSize < set->sv_contentsSize)
	end = (byte *)&select->sv_contents[select->sv_contentsSize];
    else end = (byte *)&select->sv_contents[set->sv_contentsSize];

    card = 0;

    while (selectContents < end) {
	c = BYTE(*setContents++) & BYTE(*selectContents++);
	card += BYTE(cardCounter[c]);
    }

    return card;
}

static boolean
sv_empty(set, select)
setVector *set;
setVector *select;
{
	int		i;

	for (i = 0; i < set->sv_contentsSize; ++i) {
		if (i < select->sv_contentsSize) {
			if (set->sv_contents[i] & select->sv_contents[i])
				return FALSE;
		}
		else if (set->sv_contents[i])
			return FALSE;
	}

	return TRUE;
}

static setList *
sl_create(initAlloc)
int	initAlloc;
{
    setList	*list;

    list = (setList *)ckMalloc(sizeof *list);

    if (initAlloc != 0) {
	list->sl_sets = (setVector **)
	    ckMalloc(initAlloc * sizeof *list->sl_sets);
    } else list->sl_sets = NULL;

    list->sl_alloc = initAlloc;
    list->sl_count = 0;
    list->sl_cost = 0;

    return list;
}

static void
sl_realloc(list, newAlloc)
setList *list;
int	newAlloc;
{
    register setVector	**sets;
    register setVector	**setsEnd;

    /*
    * If allocation is being reduced and sets must be removed, subtract
    * their costs.
    */
    setsEnd = list->sl_sets + list->sl_count;
    sets = list->sl_sets + newAlloc;
    while (sets < setsEnd) {
	--list->sl_count;
	list->sl_cost -= (*sets)->sv_cost;
	++sets;
    }

    list->sl_sets = (setVector **)
	ckRealloc(list->sl_sets, newAlloc * sizeof *list->sl_sets);
    if (list->sl_sets == NULL) {
	fprintf(stderr, "can't realloc\n");
	exit(1);
    }
    list->sl_alloc = newAlloc;
}

/*
* sl_free: free a list.  Sets are not freed.
*/
static void
sl_free(list)
setList *list;
{
    if (list->sl_alloc != 0)
	free(list->sl_sets);
    free(list);
}

/*
* sl_freeAll: free a list.  Sets are also freed.
*/
static void
sl_freeAll(list)
setList *list;
{
    register setVector	**sets;
    register setVector	**setsEnd;

    setsEnd = list->sl_sets + list->sl_count;
    sets = list->sl_sets;
    while (sets < setsEnd) {
	sv_free(*sets++);
    }

    sl_free(list);
}


/*
* sl_copy: Allocate a copy of the list.  Sets are not copied.
*/
static setList *
sl_copy(list)
setList *list;
{
    register setVector	**sets;
    register setVector	**setsEnd;
    register setVector	**newSets;
    setList		*newList;

    newList = sl_create(list->sl_count);

    setsEnd = list->sl_sets + list->sl_count;
    sets = list->sl_sets;
    newSets = newList->sl_sets;
    while (sets < setsEnd) {
	*newSets++ = *sets++;
    }

    newList->sl_count = list->sl_count;
    newList->sl_cost = list->sl_cost;

    return newList;
}

/*
* sl_combine: Add entries from listB to listA. Free listB.
*/
static void
sl_combine(listA, listB)
setList *listA;
setList *listB;
{
    register setVector	**sets;
    register setVector	**setsEnd;
    register setVector	**newSets;

    if (listA->sl_count + listB->sl_count > listA->sl_alloc) {
	sl_realloc(listA, listA->sl_count + listB->sl_count);
    }

    setsEnd = listB->sl_sets + listB->sl_count;
    sets = listB->sl_sets;
    newSets = listA->sl_sets + listA->sl_count;
    while (sets < setsEnd) {
	*newSets++ = *sets++;
    }

    listA->sl_count += listB->sl_count;
    listA->sl_cost += listB->sl_cost;

    sl_free(listB);
}

/*
* sl_append: Add "set" to end of "list".
*/
static void
sl_append(list, set)
setList *list;
setVector *set;
{
    if (list->sl_count + 1 > list->sl_alloc) {
	sl_realloc(list, list->sl_count + 1);
    }

    list->sl_sets[list->sl_count] = set;
    ++list->sl_count;
    list->sl_cost += set->sv_cost;
}

/*
* sl_compress: Remove from each set all elements that are not in "select".
*/
static void
sl_compress(list, select)
setList *list;
setVector *select;
{
    register setVector	**sets;
    register setVector	**setsEnd;

    setsEnd = list->sl_sets + list->sl_count;
    sets = list->sl_sets;
    while (sets < setsEnd) {
	sv_compress(*sets, select);
	++sets;
    }
}

static setList *
sl_get(f, pSetSize)
FILE	*f;
int	*pSetSize;
{
	setList		*list;
	setVector	*set;
	static int	allocSize = 8;
	int		setSize;
	int		maxSetSize = 0;

	list = sl_create(allocSize);

	while (set = sv_get(f, &setSize)) {
	    if (setSize > maxSetSize)
		maxSetSize = setSize;
	    if (list->sl_count >= allocSize) {
		allocSize += allocSize;		/* double it */
		sl_realloc(list, allocSize);
	    }
	    list->sl_sets[list->sl_count] = set;
	    ++list->sl_count;
	    list->sl_cost += set->sv_cost;
	}

	allocSize = list->sl_count;
	sl_realloc(list, allocSize);	/* get exact fit */
	if (pSetSize != NULL) *pSetSize = maxSetSize;
	return list;
}

static float
sl_normalizeCost(list)
setList		*list;
{
    setVector	**sets;
    setVector	**setsEnd;
    long	maxCost;
    int		costLimit;
    float	normFactor;

    maxCost = 0;

    setsEnd = list->sl_sets + list->sl_count;
    sets = list->sl_sets;
    while (sets < setsEnd) {
	if ((*sets)->sv_cost > maxCost)
	    maxCost = (*sets)->sv_cost;
	++sets;
    }

    costLimit = LOWORD(~0);	/* maximum half word */

    if (maxCost <= costLimit) return 1.0;

    fprintf(stderr, "Warning: normalizing costs\n");
    normFactor = costLimit / (float)maxCost;
    
    setsEnd = list->sl_sets + list->sl_count;
    sets = list->sl_sets;
    while (sets < setsEnd) {
	(*sets)->sv_cost *= normFactor;
	 ++sets;
    }

    return normFactor;
}

/*
* sl_firstGreedy:  Return the index in list of the set that minimizes
*	cost/cardinality.  If list is empty or contains only empty sets
*	with respect to "select", return -1.
*/
static int
sl_firstGreedy(list, select)
setList		*list;
setVector	*select;
{
    setVector		*set;
    int			best;
    int			card;
    unsigned long	value;
    unsigned long	minValue;
    int			i;

    minValue = ~0;
    best = -1;

    for (i = 0; i < list->sl_count; ++i) {
	set = list->sl_sets[i];
	card = sv_card(set, select);
	if (card != 0) {
	    value = (set->sv_cost * (LOWORD(~0) + 1))/ card;
	    if (value < minValue) {
		minValue = value;
		best = i;
	    }
	}
    }

    return best;
}

/*
* sl_greedy:  Append to "keep" a sublist of "list" containing a
*	covering list of approximate minimum cost in greedy order
*	considering only elements that are present in "selectArg".
*/
static void
sl_greedy(listArg, selectArg, keep)
setList		*listArg;
setVector	*selectArg;
setList		*keep;
{
    setVector	*select;
    setVector	*bestSet;
    setList	*list;
    int		best;

    /*
     * Copy arguments so the originals are not modified.
     */
    select = sv_copy(selectArg);
    list = sl_copy(listArg);

    while ((best = sl_firstGreedy(list, select)) != -1) {
	bestSet = list->sl_sets[best];
	sl_append(keep, bestSet);
	sv_subtract(select, bestSet);
	list->sl_cost -= bestSet->sv_cost;
	--list->sl_count;
	list->sl_sets[best] = list->sl_sets[list->sl_count];
    }

    sv_free(select);
    sl_free(list);

    return;
}

/*
* sl_1stGBN:  Return the index in list of the set that maximizes
*	bottleneck presence / cost,  where bottleneck prsesence is the sum
*	of 1/cols[k] for each k such that the k'th elemnt is in the set
*	and cols[k] is the number of sets that contain the k'th elemnt.
*	If list is empty or contains only empty sets with respect to "select",
*	return -1.
*/
static int
sl_1stGBN(list, select)
setList		*list;
setVector	*select;
{
    setVector		*set;
    int			best;
    int			card;
    unsigned long	value;
    unsigned long	maxValue;
    int			i;
    int			j;
    register int	k;
    unsigned long	vector;
    unsigned long	sVector;
    register unsigned long	guide;
    static int		*cols = NULL;
    static int		colsSize = 0;

    if (list == NULL) {
	colsSize = 0;
	free(cols);
	cols = NULL;
	return  -1;
    }
	
    card = sv_card(select, select);

    if (card > colsSize) {
	colsSize = card;
	cols = (int *)ckRealloc(cols, colsSize * sizeof *cols);
    }

    /*
     * Set cols[k] to number of sets containing the k'th element in "select".
     */
    for (k = 0; k < card; ++k)	/* ?unknown? should be card, not colSize */
	cols[k] = 0;				/* Init to 0. */
    for (i = 0; i < list->sl_count; ++i) {
	set = list->sl_sets[i];
	k = 0;
	for (j = 0; j < select->sv_contentsSize; ++j) {
	    if (j >= set->sv_contentsSize) break;
	    vector = set->sv_contents[j];
	    sVector = select->sv_contents[j];
	    for (guide = 1; guide != 0; guide <<= 1) {
		if (sVector & guide) {
		    if (vector & guide)
			++cols[k];			/* increment */
		    ++k;
		}
	    }
	}
    }

    maxValue = 0;
    best = -1;

    for (i = 0; i < list->sl_count; ++i) {
	set = list->sl_sets[i];
	k = 0;
	value = 0;
	for (j = 0; j < select->sv_contentsSize; ++j) {
	    if (j >= set->sv_contentsSize) break;
	    vector = set->sv_contents[j];
	    sVector = select->sv_contents[j];
	    for (guide = 1; guide != 0; guide <<= 1) {
		if (sVector & guide) {
		    if (vector & guide)
			value += (1 << 16) / cols[k];
		    ++k;
		}
	    }
	}
	value /= set->sv_cost;
	if (value > maxValue) {
	    maxValue = value;
	    best = i;
	}
    }

    return best;
}

/*
* sl_gBN:  Append to "keep" a sublist of "list" containing a
*	covering list of approximate minimum cost in greedy bottle neck order
*	considering only elements that are present in "selectArg".
*/
static void
sl_gBN(listArg, selectArg, keep)
setList		*listArg;
setVector	*selectArg;
setList		*keep;
{
    setVector	*select;
    setVector	*bestSet;
    setList	*list;
    int		best;

    /*
     * Copy arguments so the originals are not modified.
     */
    select = sv_copy(selectArg);
    list = sl_copy(listArg);

    while ((best = sl_1stGBN(list, select)) != -1) {
	bestSet = list->sl_sets[best];
	sl_append(keep, bestSet);
	sv_subtract(select, bestSet);
	list->sl_cost -= bestSet->sv_cost;
	--list->sl_count;
	list->sl_sets[best] = list->sl_sets[list->sl_count];
    }

    sv_free(select);
    sl_free(list);

    return;
}

/*
* sl_coveredByOthers: Return TRUE if "set" is covered by all
*	entries on list combined, excluding itself, considering only elements
*	which are in "select".
*/
static boolean
sl_coveredByOthers(set, list, select)
setVector *set;
setList *list;
setVector *select;
{
    register setVector	**sets;
    setVector		**setsEnd;
    register int	i;
    register unsigned long	bunch;

    /*
     * For each contents index, take the sv_contents[i] of "set" and remove
     * elements that are not in "select" and elements that are in any
     * list entry other than "set".  If anything is left, it is not covered. 
     * Otherwise it is covered.
     */
    for (i = 0; i < set->sv_contentsSize; ++i) {
	if (i >= select->sv_contentsSize) break;
	bunch = set->sv_contents[i];
	bunch &= select->sv_contents[i];

	setsEnd = list->sl_sets + list->sl_count;
	sets = list->sl_sets;
	for (; sets < setsEnd && bunch; ++sets) {
	    if (*sets == set) {
		continue;	/* Ignore "set" itself. */
	    }
	    if (i >= (*sets)->sv_contentsSize) {
		continue;
	    }
	    bunch &= ~(*sets)->sv_contents[i];
	}
	if (bunch) return FALSE;
    }
    return TRUE;
}

/*
* sl_mustKeep:  Find all the sets on "list" that are not covered by all
*	the others together, considering only elements that are in "select";
*	remove them from "list" and add them to "keep" list.  Modify
*	"select" by removing any element that is in a set added to the "keep"
*	list.  If cost reaches "costLimit" return FAIL.  Otherwise SUCCEED.
*/
static int				/* FAIL or SUCCEED */
sl_mustKeep(list, select, costLimit, keep)
setList		*list;
setVector	*select;
long		costLimit;
setList		*keep;
{
    register setVector	**sets;
    register setVector	**setsEnd;
    setVector		**newSets;
    static setVector	empty = { "", 0, 0, 0 };
    int			keepCount = 0;
    long		keepCost = 0;

    setsEnd = list->sl_sets + list->sl_count;
    sets = list->sl_sets;
    newSets = sets;
    while (sets < setsEnd) {
	if (!sl_coveredByOthers(*sets, list, select)) {
	    sl_append(keep, *sets);
	    if (keep->sl_cost >= costLimit + g_allMin) {
		return FAIL;
	    }
	    ++keepCount;
	    keepCost += (*sets)->sv_cost;	/* Adjust list->sl_cost later */
	    sv_subtract(select, *sets);		/* ... to avoid messing up */
	    *sets = &empty;			/* ... sl_coveredByOthers. */
	} else {
	    if (newSets != sets) {
		*newSets = *sets;
		*sets = &empty;
	    }
	    ++newSets;
	}
	++sets;
    }

    list->sl_count -= keepCount;
    list->sl_cost -= keepCost;

    return SUCCEED;
}

/*
* sl_dontNeed:  Remove from "list", any set that is covered by a set on list
*	with lower cost, considering only elements that are in "select".
*/
static void
sl_dontNeed(list, select)
setList *list;
setVector *select;
{
    setVector		**sets;
    setVector		**newSets;
    register setVector	**sets2;
    register setVector	**setsEnd;
    static setVector	empty = { "", 0, 0, 0 };

    setsEnd = list->sl_sets + list->sl_count;
    sets = list->sl_sets;
    newSets = list->sl_sets;
    while (sets < setsEnd) {
	sets2 = list->sl_sets;
	for (; sets2 < setsEnd; ++sets2) {
	    if (sets == sets2) continue;
	    if ((*sets)->sv_cost < (*sets2)->sv_cost) continue;
	    if (sv_subset(*sets2, *sets, select))
		break;
	}
	if (sets2 < setsEnd) {
	    --list->sl_count;
	    list->sl_cost -= (*sets)->sv_cost;
	    *sets = &empty;
	} else {
	    if (newSets != sets) {
		*newSets = *sets;
		*sets = &empty;
	    }
	    ++newSets;
	}
	++sets;
    }

    if (list->sl_count == 1) {
	if (sv_empty(list->sl_sets[0], select)) {
	    --list->sl_count;
	    list->sl_cost -= list->sl_sets[0]->sv_cost;
	}
    }
}

/*
* sl_reduce:  Return a new list containing all sets on "list" that must
*	be included to cover elements in "select".  Remove covered elements
*	from "select".  Retain on "list" only sets that cover some element
*	remaining in "select".	If cost of new list reaches "costLimit"
*	remove all sets from "list" and return NULL.
*	For efficiency, create keep with enough allocation to hold final
*	covering list.
*/
static setList *
sl_reduce(list, select, costLimit)
setList		*list;
setVector	*select;
long		costLimit;
{
    setList	*keep;
    int		prevCount;
    boolean	doMustKeep;
    boolean	doDontNeed;
    int		status;
    
    /*
     * Create "keep" list.
     */
    keep = sl_create(list->sl_count);

    /*
     * Look for sets that must be kept or definitely are not needed.
     * Repeat until no more of either found.
     */
    doMustKeep = TRUE;
    doDontNeed = TRUE;
    while (doMustKeep || doDontNeed) {
	/*
	 * Remove sets from "list" that have any elements that are not
	 * in any other set, and put them on "keep".  "Select" is
	 * modified to consider only elements that are not in any
	 * set on "list".
	 */
	if (doMustKeep) {
	    prevCount = list->sl_count;
	    status = sl_mustKeep(list, select, costLimit, keep);
	    if (status == SUCCEED) {
		if (list->sl_count == 0)
		    return keep;
		if (prevCount != list->sl_count)
		    doDontNeed = TRUE;
	    } else {
		list->sl_count = 0;
		sl_free(keep);
		return NULL;
	    }
	}
	doMustKeep = FALSE;

	/*
	 * Remove sets from "list" that have no elements in "select".
	 * I.e. no elements that are not in a set on "keep" list.
	 */
	if (doDontNeed) {
	    prevCount = list->sl_count;
	    sl_dontNeed(list, select);
	    if (prevCount != list->sl_count)
		doMustKeep = TRUE;
	}
	doDontNeed = FALSE;
    }

    return keep;
}

static void
sl_print(list)
setList *list;
{
    register setVector	**sets;
    register setVector	**setsEnd;

    setsEnd = list->sl_sets + list->sl_count;
    sets = list->sl_sets;
    while (sets < setsEnd) {
	printf("%s\n", (*sets)->sv_name);
	++sets;
    }
}


/*
* sl_union:  Return a set that is the union of all the sets on list.
*/
static setVector *
sl_union(list, name)
setList *list;
char	*name;
{
    int		contentsSize;
    setVector	*uSet;
    int		i;
    register setVector	**sets;
    register setVector	**setsEnd;

    /*
     * Calculate the size of the union.
     */
    contentsSize = 0;
    setsEnd = list->sl_sets + list->sl_count;
    sets = list->sl_sets;
    while (sets < setsEnd) {
	if ((*sets)->sv_contentsSize > contentsSize)
	    contentsSize = (*sets)->sv_contentsSize;
	++sets;
    }

    /*
     * Allocate an empty set of the correct size.
     */
    uSet = NULL;
    uSet = (setVector *)ckMalloc(&uSet->sv_contents[contentsSize]);
    uSet->sv_name = (char *)ckMalloc(strlen(name) + 1);
    strcpy(uSet->sv_name, name);
    uSet->sv_contentsSize = contentsSize;
    for (i = 0; i < uSet->sv_contentsSize; ++i)
	uSet->sv_contents[i] = 0;

    /*
     * OR all the sets on list into the union set.
     */
    setsEnd = list->sl_sets + list->sl_count;
    sets = list->sl_sets;
    while (sets < setsEnd) {
	for (i = 0; i < (*sets)->sv_contentsSize; ++i)
	    uSet->sv_contents[i] |= (*sets)->sv_contents[i];
	++sets;
    }

    return uSet;
}

static void
sl_cumPrint(list, setSize, normFactor, header)
setList	*list;
int	setSize;
float	normFactor;
boolean	header;
{
    long		cov;
    long		cost;
    register setVector	**sets;
    register setVector	**setsEnd;
    setVector		*listUnion;
    int			unionCard;
    int			z;

    if (header) {
	printf("coverage\tcost\ttest (cumulative)\n");
	printf("--------\t----\t----\n");
    }

    listUnion = sl_union(list, "union");
    unionCard = sv_card(listUnion, listUnion);

    cost = 0;

    setsEnd = list->sl_sets + list->sl_count;
    sets = list->sl_sets;
    while (sets < setsEnd) {
	sv_subtract(listUnion, *sets);
	cov = unionCard - sv_card(listUnion, listUnion);
	cost += (*sets)->sv_cost / normFactor;
	if (setSize == 0) {
	    z = 100;				/* 0 of 0 is 100% */
	} else {
	    z = (cov*100 + setSize/2) / setSize;/* % of setSize rounded. */
	    if (z == 100 && cov != setSize)	/* 99% < x < 100% round down. */
		z = 99;
	}
	if (z < 100) printf("%2d(%d/%d)", z, cov, setSize);
	else printf("100(%d)", setSize);
	printf("\t%d\t%s\n", cost, (*sets)->sv_name);
	++sets;
    }
}

static void
sl_dump(list)
setList *list;
{
    register setVector	**sets;
    register setVector	**setsEnd;

    setsEnd = list->sl_sets + list->sl_count;
    sets = list->sl_sets;
    while (sets < setsEnd) {
	sv_dump(*sets);
	++sets;
    }
}

#if 0
/*
* sl_select:  Return a set 
*/
static setVector *
sl_select(list, name)
setList *list;
char	*name;
{
    int		contentsSize;
    setVector	*set;
    int		i;
    register setVector	**sets;
    register setVector	**setsEnd;

    contentsSize = 0;

    setsEnd = list->sl_sets + list->sl_count;
    sets = list->sl_sets;
    while (sets < setsEnd) {
	if ((*sets)->sv_contentsSize > contentsSize)
	    contentsSize = (*sets)->sv_contentsSize;
	++sets;
    }

    set = NULL;
    set = (setVector *)ckMalloc(&set->sv_contents[contentsSize]);
    set->sv_name = (char *)ckMalloc(strlen(name) + 1);
    strcpy(set->sv_name, name);
    set->sv_contentsSize = contentsSize;
    for (i = 0; i < set->sv_contentsSize; ++i)
	set->sv_contents[i] = ~0;

    return set;
}
#endif /* 0 */

/*
* sl_minimize: Return a sub-list of "listArg" containing a minimal
*	covering list of cost less than costLimit
*	considering only elements that are present in "selectArg".
*	If there is no covering list of cost < costLimit, NULL is returned.
*	Recursion limit, if present,  is decremented each time this
*	routine is called.  If recursion limit is zero, NULL is returned.
*	(This will result in a covering list that is not minimal.)
*/
static setList *
sl_minimize(listArg, selectArg, costLimit, costSoFar)
setList		*listArg;
setVector	*selectArg;
long		costLimit;
long		costSoFar;
{
    setVector	*select;
    setList	*list;
    setList	*keep;
    setList	*prevBest;
    setList	*best;
    setVector	*dropSet;
    int		dropIndex;
    static int	level = 0;

    ++level;
    if (level > g_maxLevel) {
	g_maxLevel = level;
    }

    /*
     * Check recursion limit.
     */
    if (g_visited == g_recursionLimit) {
	if (g_recursionLimit != 0) {
	    --level;
	    return NULL;
	}
    }
    ++g_visited;
    if ((g_visited & g_printFreq) == 0) {
	fprintf(stderr, "At node <%s> visit %d\n", g_nodeId, g_visited);
    }

    /*
     * Copy arguments so the originals are not modified.
     */
    select = sv_copy(selectArg);
    list = sl_copy(listArg);

    /*
     * Create "keep" list.
     */
    keep = sl_reduce(list, select, costLimit);

    /*
     * Failed?  (Cost of keep list already exceeds costLimit.)
     */
    if (keep == NULL) {
	sv_free(select);
	sl_free(list);
	--level;
	return NULL;
    }

    /*
     * Done?
     */
    if (list->sl_count == 0) {
	if (!g_quiet) fprintf(stderr,
			      "solution of cost %d at visit %d level %d\n",
			      costSoFar + keep->sl_cost, g_visited, level);
	sv_free(select);
	sl_free(list);
	--level;
	return keep;
    }

    /*
     * At this point:
     * 	1. "list" must contain at least three sets.
     *	2. At least one of the sets is needed to cover "select".
     *	3. "list" minus any one set will still cover "select".
     * (3 is true because otherwise mustKeep would have put on the keep
     * list the set that is needed to cover.  2 is true because otherwise
     * dontNeed would have removed all the sets.  1 is true because
     * if there were only one, mustKeep would have kept it.  If there were
     * only two, dontNeed would have removed the one with higher cost.
     */

    /*
     * Remove a set from list.
     */
    if (g_greedyFlag)
	dropIndex = sl_firstGreedy(list, select);
    else if (g_gBNFlag)
	dropIndex = sl_1stGBN(list, select);
    else dropIndex = 0;
    dropSet = list->sl_sets[dropIndex];
    --list->sl_count;
    list->sl_sets[dropIndex] = list->sl_sets[list->sl_count];
    list->sl_cost -= dropSet->sv_cost;
	
    if (g_nodeId[level - 1] == '1') {
	prevBest = NULL;		/* Already taken this route. */
    } else {
	/*
	 * Find minimum after discarding one set.
	 */
	if (g_nodeId[level - 1] == '\0') {
	    g_nodeId[level - 1] = '0';
	    g_nodeId[level] = '\0';
	}
	prevBest = sl_minimize(list, select, costLimit - keep->sl_cost,
			       costSoFar + keep->sl_cost);
	g_nodeId[level - 1] = '1';
    }

    /*
     * Find minimum after keeping one set.
     */
    sv_subtract(select, dropSet);
    if (prevBest == NULL) {
	best = sl_minimize(list, select,
			   costLimit - keep->sl_cost - dropSet->sv_cost,
			   costSoFar + keep->sl_cost + dropSet->sv_cost);
	if (best == NULL) {
	    sv_free(select);
	    sl_free(list);
	    sl_free(keep);
	    g_nodeId[level - 1] = '\0';
	    --level;
	    return NULL;
	} else {
	    sl_append(keep, dropSet);
	}
    } else {
	best = sl_minimize(list, select, prevBest->sl_cost - dropSet->sv_cost,
			   costSoFar + keep->sl_cost + dropSet->sv_cost);
	if (best == NULL) {
	    best = prevBest;
	} else {
	    sl_free(prevBest);
	    sl_append(keep, dropSet);
	}
    }
    g_nodeId[level - 1] = '\0';

    /*
     * Clean up.
     */
    sv_free(select);
    sl_free(list);

    /*
     * Combine minimum sub-list with "keep" list and return.
     */
    sl_combine(keep, best);
    --level;
    return keep;
}

/*
* sl_Rminimize: Return a sub-list of "listArg" containing a minimal
*	covering list of cost less than costLimit
*	considering only elements that are present in "selectArg".
*	If there is no covering list of cost < costLimit, NULL is returned.
*	Recursion limit, if present,  is decremented each time this
*	routine is called.  If recursion limit is zero, NULL is returned.
*	(This will result in a covering list that is not minimal.)
*/
static setList *
sl_Rminimize(listArg, selectArg, costLimit, costSoFar)
setList		*listArg;
setVector	*selectArg;
long		costLimit;
long		costSoFar;
{
    setVector	*select;
    setList	*list;
    setList	*keep;
    setList	*prevBest;
    setList	*best;
    setVector	*dropSet;
    setVector	*dropSelect = NULL;
    int		dropIndex;
    static int	level = 0;

    ++level;
    if (level > g_maxLevel) {
	g_maxLevel = level;
    }

    /*
     * Check recursion limit.
     */
    if (g_visited == g_recursionLimit) {
	if (g_recursionLimit != 0) {
	    --level;
	    return NULL;
	}
    }
    ++g_visited;
   if ((g_visited & g_printFreq) == 0) {
       fprintf(stderr, "At node <%s> visit %d\n", g_nodeId, g_visited);
   }

    /*
     * Copy arguments so the originals are not modified.
     */
    select = sv_copy(selectArg);
    list = sl_copy(listArg);

    /*
     * Create "keep" list.
     */
    keep = sl_reduce(list, select, costLimit);

    /*
     * Failed?  (Cost of keep list already exceeds costLimit.)
     */
    if (keep == NULL) {
	sv_free(select);
	sl_free(list);
	--level;
	return NULL;
    }

    /*
     * Done?
     */
    if (list->sl_count == 0) {
	if (!g_quiet) fprintf(stderr,
			      "solution of cost %d at visit %d level %d\n",
			      costSoFar + keep->sl_cost, g_visited, level);
	sv_free(select);
	sl_free(list);
	--level;
	return keep;
    }

    /*
     * At this point:
     * 	1. "list" must contain at least three sets.
     *	2. At least one of the sets is needed to cover "select".
     *	3. "list" minus any one set will still cover "select".
     * (3 is true because otherwise mustKeep would have put on the keep
     * list the set that is needed to cover.  2 is true because otherwise
     * dontNeed would have removed all the sets.  1 is true because
     * if there were only one, mustKeep would have kept it.  If there were
     * only two, dontNeed would have removed the one with higher cost.
     */

    /*
     * Remove a set from list.
     */
    if (g_greedyFlag)
	dropIndex = sl_firstGreedy(list, select);
    else if (g_gBNFlag)
	dropIndex = sl_1stGBN(list, select);
    else dropIndex = 0;
    dropSet = list->sl_sets[dropIndex];
    --list->sl_count;
    list->sl_sets[dropIndex] = list->sl_sets[list->sl_count];
    list->sl_cost -= dropSet->sv_cost;
	
    if (g_nodeId[level - 1] == '0') {
	prevBest = NULL;
    } else {
	/*
	 * Find minimum after keeping one set.
	 */
	if (g_nodeId[level - 1] == '\0') {
	    g_nodeId[level - 1] = '0';
	    g_nodeId[level] = '\0';
	}
	dropSelect = sv_copy(select);
	sv_subtract(select, dropSet);
	prevBest = sl_Rminimize(list,
			select, costLimit - keep->sl_cost - dropSet->sv_cost,
			costSoFar + keep->sl_cost + dropSet->sv_cost);
	sv_free(select);
	g_nodeId[level - 1] = '1';
    }

    /*
     * Find minimum after discarding one set.
     */
    select = dropSelect;
    if (prevBest == NULL) {
	best = sl_Rminimize(list, select, costLimit - keep->sl_cost,
			   costSoFar + keep->sl_cost);
	if (best == NULL) {
	    sv_free(select);
	    sl_free(list);
	    sl_free(keep);
	    g_nodeId[level - 1] = '\0';
	    --level;
	    return NULL;
	}
    } else {
	best = sl_Rminimize(list, select, prevBest->sl_cost,
			   costSoFar + keep->sl_cost);
	if (best == NULL) {
	    best = prevBest;
	    sl_append(keep, dropSet);
	} else {
	    sl_free(prevBest);
	}
    }
    g_nodeId[level - 1] = '\0';

    /*
     * Clean up.
     */
    sv_free(select);
    sl_free(list);

    /*
     * Combine minimum sub-list with "keep" list and return.
     */
    sl_combine(keep, best);
    --level;
    return keep;
}

/*
* sl_cost0:  Return a list of 0 cost sets removed from list.
*/
static setList *
sl_cost0(list)
setList	*list;
{
    register setVector	**sets;
    register setVector	**setsEnd;
    setList		*keep;

    keep = sl_create(list->sl_count);

    setsEnd = list->sl_sets + list->sl_count;
    sets = list->sl_sets;
    while (sets < setsEnd) {
	if ((*sets)->sv_cost == 0) {
	    sl_append(keep, *sets);
	    --list->sl_count;
	    *sets = list->sl_sets[list->sl_count];
	} else ++sets;
    }

    return keep;
}

main(argc, argv)
int	argc;
char	*argv[];
{
    FILE	*f;
    char	*p;
    int		i;
    unsigned long	costLimit;
    unsigned long	userCost;
    boolean	namesOnly = FALSE;
    boolean	cumPrint = FALSE;
    boolean	header = TRUE;
    boolean	compress = FALSE;
    boolean	keepFirst = FALSE;
    char	*restartNodeId = 0;
    setVector	*select;
    setList	*list;
    setList	*keep;
    setList	*greedy;
    setList	*gBN;
    setList	*cost0;
    setList	*reduceList;
    setList	*freeList;
    time_t	startTime;
    time_t	endTime;
    float	normFactor;
    int		setSize;

    g_visited = 0;
    g_maxLevel = 0;
    g_recursionLimit = 0;
    g_printFreq = ~0L;
    g_quiet = FALSE;

    userCost = ~0;		/* big number */

    /*
     * Collect options.
     */
    for (i = 1; i < argc; ++i) {
	p = argv[i];
	if (*p != '-') break;
	while (*++p) switch(*p)
	{
	case 'a':
	    /*
	     * This hack attempts to cause all minimums found to print.
	     */
	    g_allMin = 1;
	    break;
	case 'c':
	    if (*++p == '\0') {
		if (++i == argc) {
		    fprintf(stderr, "argument missing for -%c\n", *--p);
		    usage(argv[0]);
		    exit(1);
		}
		p = argv[i];
	    }
	    userCost = atoi(p);
	    p += strlen(p) - 1;	/* setup for next arg */
	    break;
	case 'C':
	    compress = TRUE;
	    break;
	case 'g':
	    g_greedyFlag = TRUE;
	    break;
	case 'G':
	    g_gBNFlag = TRUE;
	    break;
	case 'h':
	    header = FALSE;
	    break;
	case 'l':
	    if (*++p == '\0') {
		if (++i == argc) {
		    fprintf(stderr, "argument missing for -%c\n", *--p);
		    usage(argv[0]);
		    exit(1);
		}
		p = argv[i];
	    }
	    g_recursionLimit = atoi(p);
	    p += strlen(p) - 1;	/* setup for next arg */
	    break;
	case 'n':
	    namesOnly = TRUE;
	    break;
	case 'p':
	    if (*++p == '\0') {
		if (++i == argc) {
		    fprintf(stderr, "argument missing for -%c\n", *--p);
		    usage(argv[0]);
		    exit(1);
		}
		p = argv[i];
	    }
	    g_printFreq = (1 << atoi(p)) - 1;
	    fprintf(stderr, "print progress every %ld\n", g_printFreq);
	    p += strlen(p) - 1;	/* setup for next arg */
	    break;
	case 'q':
	    g_quiet = TRUE;
	    break;
	case 'r':
	    keepFirst = TRUE;
	    break;
	case 'R':
	    if (*++p == '\0') {
		if (++i == argc) {
		    fprintf(stderr, "argument missing for -%c\n", *--p);
		    usage(argv[0]);
		    exit(1);
		}
		p = argv[i];
	    }
	    restartNodeId = p;
	    p += strlen(p) - 1;	/* setup for next arg */
	    break;
	case 's':
	    cumPrint = TRUE;
	    namesOnly = TRUE;	/* don't dump sets */
	    break;
	case '?':
	    usage(argv[0]);
	    exit(1);
	    break;
	default:
	    fprintf(stderr, "unknown option: -%c\n", *p);
	    usage(argv[0]);
	    exit(1);
	    break;
	}
    }
	
    if (i + 1 < argc) {
	usage(argv[0]);
	exit(1);
    }

    costLimit = userCost;

    /*
     * Read in the set vectors.
     */
    if (i < argc) {
	f = fopen(argv[i], "r");
	if (f == NULL) {
	    perror(argv[i]);
	    usage(argv[0]);
	    exit(1);
	}
	list = sl_get(f, &setSize);
	fclose(f);
	++i;
    } else {
	list = sl_get(stdin, &setSize);
    }

    freeList = sl_copy(list);

    /*
     * Compress for efficiency.
     */
    select = sl_union(list, "select");
    sl_compress(list, select);
    sv_free(select);			/* Doesn't apply to compressed set. */

    /*
     * Cost 0 sets are always selected.
     */
    cost0 = sl_cost0(list);

    /*
     * Find sets that must be in the minimum.
     */
    select = sl_union(list, "select");
    reduceList = sl_reduce(list, select, list->sl_cost + 1);
    if (reduceList == NULL) {
	fprintf(stderr, "internal errror\n");
	exit(1);
    }

    /*
     * Compress again for sets removed by sl_cost0() and sl_reduce();
     */
    if (compress) {
	sl_compress(list, select);
	sv_free(select);		/* Doesn't apply to reduced set. */
	select = sl_union(list, "select");
    }

    normFactor = sl_normalizeCost(list);

    /*
    * Try greedy.
    */
    greedy = sl_create(list->sl_count);
    sl_greedy(list, select, greedy);
    if (!g_quiet) fprintf(stderr, "greedy cost: %d\n",
			  greedy->sl_cost + reduceList->sl_cost);
    if (costLimit > greedy->sl_cost)
	costLimit = greedy->sl_cost;

    /*
    * Try greedy on bottle necks (GBN).
    */
    gBN = sl_create(list->sl_count);
    sl_gBN(list, select, gBN);
    if (!g_quiet) fprintf(stderr, "GBN cost: %d\n",
			  gBN->sl_cost + reduceList->sl_cost);
    if (costLimit > gBN->sl_cost) {
	costLimit = gBN->sl_cost;
	sl_free(greedy);
	greedy = gBN;
    }

    /*
     * Set up node ID.
     */
    g_nodeId = (char *)ckMalloc(list->sl_count + 1);
    if (restartNodeId != NULL) {
	if (strlen(restartNodeId) > list->sl_count) {
	    fprintf(stderr, "%s: invalid restart node ID\n", restartNodeId);
	    exit(1);
	}
	for (p = restartNodeId; *p != '\0'; ++p) {
	    if (*p != '0' && *p != '1') {
		fprintf(stderr, "%s: invalid restart node ID\n", restartNodeId);
		exit(1);
	    }
	}
	strcpy(g_nodeId, restartNodeId);
    }
    else g_nodeId[0] = '\0';

    if (costLimit > list->sl_cost)
	costLimit = list->sl_cost;
    startTime = (time_t)time((time_t *)NULL);

    /*
     * Minimize the list of set vectors.
     */
    if (keepFirst) 
	keep = sl_Rminimize(list, select, costLimit, reduceList->sl_cost);
    else keep = sl_minimize(list, select, costLimit, reduceList->sl_cost);
    endTime = (time_t)time((time_t *)NULL);
    if (g_visited == g_recursionLimit) {
	if (g_recursionLimit != 0) {
	    fprintf(stderr, "recursion limit %d exceeded\n", g_recursionLimit);
	}
    }
    free(g_nodeId);
	
    sv_free(select);
    sl_free(list);

    if (keep == NULL && greedy->sl_cost < userCost) 
	keep = greedy;
    else sl_free(greedy);

    if (keep) {
    if (!g_quiet) fprintf(stderr, "cost: %d\n",
			  keep->sl_cost + reduceList->sl_cost);
	sl_combine(cost0, reduceList);
	sl_combine(cost0, keep);
	keep = cost0;
	if (!compress) {
	    greedy = sl_create(keep->sl_count);
	    select = sl_union(keep, "select");
	    sl_greedy(keep, select, greedy);
	    sv_free(select);
	    sl_free(keep);
	    keep = greedy;
	}
	if (cumPrint && !compress) {
	    sl_cumPrint(keep, setSize, normFactor, header);
	}
	else if (namesOnly || compress) {
	    sl_print(keep);
	} else {
	    sl_dump(keep);
	}
	sl_free(keep);
    } else {
	fprintf(stderr, "no cover less than %d\n", costLimit);
	sl_free(cost0);
	sl_free(reduceList);
    }

    if (!g_quiet) {
	fprintf(stderr, "%ld nodes visited\n", g_visited);
	fprintf(stderr, "%d levels visited\n", g_maxLevel);
	fprintf(stderr, "total recursion time: %ld\n", endTime - startTime);
	fprintf(stderr, "recursion time per node: %f\n",
		(endTime - startTime)/(float)g_visited);
    }

    startTime = (time_t)time((time_t *)NULL);

    /*
     * Clean up.
     */
    sl_freeAll(freeList);

    exit(0);
}
