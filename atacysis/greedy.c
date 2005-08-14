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

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include "portable.h"
#include "atacysis.h"

static char const gree_c[] = 
	"$Header: /users/source/archives/atac.vcs/atacysis/RCS/greedy.c,v 3.5 2005/08/14 13:47:59 tom Exp $";
/*
* $Log: greedy.c,v $
* Revision 3.5  2005/08/14 13:47:59  tom
* gcc warnings
*
* Revision 3.4  1995/12/29 21:24:41  tom
* adjust headers, prototyped for autoconfig
*
*Revision 3.3  94/04/04  10:25:23  jrh
*Add Release Copyright
*
*Revision 3.2  93/08/04  15:54:34  ewk
*Added MVS and solaris support.  Squelched some ANSI warnings.
*
* Revision 3.1  92/12/03  08:47:14  saul
*Sort tests into coverage order.
*
*-----------------------------------------------end of log
*/

/* forward declarations */
static void unionCov
	P_((int *covSoFar, int *covVector, int covCount));
static int countCov
	P_((int *covVector, int *covSoFar, int covCount));

#define CHECK_MALLOC(p) if((p)==NULL)fprintf(stderr,"Out of memory\n"),exit(1)

#ifdef DEBUG
static void
dumpVector(cov, covCount)
int		*cov;
int		covCount;
{
    int	i;

    for (i = 0; i < covCount; ++i) {
	switch (cov[i])
	{
	case -1:
	    putc('-', stderr);
	    break;
	case 0:
	    putc(' ', stderr);
	    break;
	default:
	    putc('x', stderr);
	}
    }
}

static void
dumpAll(nCov, covList, covCount)
int		nCov;
T_TESTLIST	*covList;
int		covCount;
{
    int	i;

    for (i = 0; i < nCov; ++i) {
	dumpVector(covList[i].cov, covCount);
	fprintf(stderr, ":%d:%s\n", covList[i].cost, covList[i].name);
    }
}
#endif /* DEBUG */

/*
* countCov:  Return number of items covered by covVector that were not already
* covered by covSoFar.
*/
static int
countCov(covSoFar, covVector, covCount)
int		*covVector;
int		*covSoFar;
int		covCount;
{
    int	i;
    int	count;

    count = 0;

    for (i = 0; i < covCount; ++i) {
	if (covSoFar[i] == 0 && covVector[i] != 0)
	    ++count;
    }

    return count;
}

/*
* unionCov:  Add to covSoFar items covered in covVector.
*/
static void
unionCov(covSoFar, covVector, covCount)
int		*covSoFar;
int		*covVector;
int		covCount;
{
    int	i;

    for (i = 0; i < covCount; ++i) {
	if (covSoFar[i] == 0 && covVector[i] != 0)
	    covSoFar[i] = 1;
    }

    return;
}

/*
* greedyOrder: Sort the nCov entries in covList into greedy coverage order.
* 	In case of equivalence, preserve original order.  If
*	covList[j].cov[i] is -1 for any j, disregard item i for all j.
*	If covList[j].cov[i] is 0 item i is not covered by covList[j].
*	Otherwise item i is covered.
*/
void
greedyOrder(nCov, covList, covCount)
int		nCov;
T_TESTLIST	*covList;
int		covCount;
{
    int		*covSoFar;
    int		i;
    int		j;
    float	bestCostPer;
    float	costPer;
    float	count;
    int		best = 0;
    int		*order;
    T_TESTLIST	swapper;

    /*
    * Create vector of items covered so far.  Disregarded items are considered
    * covered.
    */
    covSoFar = (int *)malloc(covCount * sizeof *covSoFar);
    CHECK_MALLOC(covSoFar);
    for (i = 0; i < covCount; ++i) {
	covSoFar[i] = 0;
	for (j = 0; j < nCov; ++j) {
	    if (covList[j].cov[i] == -1)
		covSoFar[i] = 1;
	}
    }

    /*
    * Create order[] index.  Order[j] is the original index of covList[j]
    * for unsorted vectors (j >= i).  For sorted vectors (j < i) order[]
    * is garbage.  In case of equivalence, this is used to preserve original
    * order.
    */
    order = (int *)malloc(covCount * sizeof *order);
    CHECK_MALLOC(order);
    for (i = 0; i < nCov; ++i) {
	order[i] = i;
    }

    for (i = 0; i < nCov - 1; ++i) {
	bestCostPer = 1E15;	/* large number */
	/*
	* Find j such that cost per uncovered item covered by covList[j]
	* is minimized.  In case of tie, pick j such that order[j] is minimized.
	*/
	for (j = i; j < nCov; ++j) {
	    count = countCov(covSoFar, covList[j].cov, covCount);
	    if (count == 0.0)
		count = 0.5;	/* less than 1 */
	    costPer = covList[j].cost / count;
	    if (costPer < bestCostPer ||
		(costPer == bestCostPer && order[j] < order[best]))
	    {
		bestCostPer = costPer;
		best = j;
	    }
	}

	/*
	* Add items covered by covList[j].cov to covSoFar.
	*/
	unionCov(covSoFar, covList[best].cov, covCount);

	/*
	 * Move covList[best] up to first unsorted position (covList[i])
	 * by swapping.
	 */
	if (best != i) {
	    memcpy(&swapper, covList + i, sizeof swapper);
	    memcpy(covList + i, covList + best, sizeof swapper);
	    memcpy(covList + best, &swapper, sizeof *covList);
	    order[best] = order[i];
	}
    }

    free(order);
    free(covSoFar);

    return;
}
