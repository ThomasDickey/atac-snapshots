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

#if HAVE_STDLIB_H
#include <stdlib.h>
#endif

#include <stdio.h>
#include "portable.h"
#include "atacysis.h"

static char const summary_c[] = "$Id: summary.c,v 3.10 2013/12/09 01:06:52 tom Exp $";
/*
* @Log: summary.c,v @
* Revision 3.8  1995/12/29 21:24:41  tom
* adjust headers, prototyped for autoconfig
*
* Revision 3.7  94/06/01  16:12:24  saul
* Error in total line for atac -p -f ...
* 
* Revision 3.6  94/04/04  10:26:23  jrh
* Add Release Copyright
* 
* Revision 3.5  93/11/02  11:50:04  saul
* Same as revision 3.3
* 
* Revision 3.3  93/08/04  15:58:47  ewk
* Added MVS and solaris support.  Squelched some ANSI warnings.
* 
* Revision 3.2  93/05/12  13:40:37  saul
* Round 99+% down to avoid 100% when some are uncovered.
* 
* Revision 3.1  92/12/03  08:49:22  saul
* CUMULATIVE and COST options
* 
* Revision 3.0  92/11/06  07:47:54  saul
* propagate to version 3.0
* 
* Revision 2.9  92/11/03  10:32:46  saul
* remove unused parameters
* 
* Revision 2.8  92/11/02  11:44:05  saul
* remove unused variables
* 
* Revision 2.7  92/10/30  09:55:42  saul
* include portable.h
* 
* Revision 2.6  92/09/30  11:14:01  saul
* Suppress header with -h option.
* 
* Revision 2.5  92/09/16  07:26:31  saul
* Change "== all ==" to "== total ==" for counter-atac
* 
* Revision 2.4  92/09/08  09:20:33  saul
* New options and coverage vector data structure.
* 
* Revision 2.3  92/07/10  11:17:18  saul
* adjust counts for detected infeasables; remove obsolete T_DECIS
* 
* Revision 2.2  92/03/17  15:27:16  saul
* copyright
* 
* Revision 2.1  91/06/19  13:10:11  saul
* Propagate to version 2.0
* 
* Revision 1.1  91/06/12  16:58:37  saul
* Aug 1990 baseline
* 
*-----------------------------------------------end of log
*/

#define CHECK_MALLOC(p) if((p)==NULL)fprintf(stderr,"Out of memory\n"),exit(1)

#define LINEMAX	100

#define C_BLOCK	0
#define C_DECIS	1
#define C_C_USE 2
#define C_P_USE 3
#define C_F_ENTRY 4
#define C_COUNT 5

static void
heading(int options,
	const char *label)
{
    int fields;
    int i;
    char type;

    if (options & OPTION_NO_HEADER)
	return;

    fields = 0;

    if (options & OPTION_COUNTER_ATAC)
	type = '#';
    else
	type = '%';

    if (options & OPTION_COST) {
	printf("%-6s ", "cost");
    }

    if (options & OPTION_F_ENTRY) {
	printf("%c%-13s", type, " functions");
	++fields;
    }
    if (options & OPTION_BLOCK) {
	printf("%c%-13s", type, " blocks");
	++fields;
    }
    if (options & OPTION_DECIS) {
	printf("%c%-13s", type, " decisions");
	++fields;
    }
    if (options & OPTION_CUSE) {
	printf("%c%-13s", type, " C-Uses");
	++fields;
    }
    if (options & OPTION_PUSE) {
	printf("%c%-13s", type, " P-Uses");
	++fields;
    }
    if (options & OPTION_ALLUSE) {
	printf("%c%-13s", type, " All-Uses");
	++fields;
    }
    printf("%s\n", label);

    if (options & OPTION_CUMULATIVE) {
	if (options & OPTION_COST) {
	    printf("%-6.6s ", "(cum)");
	}
	for (i = 0; i < fields; ++i) {
	    printf("%-13.13s ", "(cumulative)");
	}
	printf("\n");
    }

    if (options & OPTION_COST) {
	printf("%-6.6s ", "------------------------");
    }

    if (*label) {
	++fields;
    }

    for (i = 0; i < fields; ++i) {
	printf("%-13.13s ", "------------------------");
    }
    printf("\n");
}

static void
putCost(long cost)
{
    printf("%-6ld ", cost);
}

static void
fEntryCov(T_FUNC * func,
	  int *covVector,
	  int *cov,
	  int *tot)
{
    if (covVector[func->blkCovStart] == -1) {
	*tot = 0;
	*cov = 0;
    } else {
	*tot = 1;
	*cov = covVector[func->blkCovStart];
    }
}

static void
blkCov(T_FUNC * func,
       int *covVector,
       int *cov,
       int *tot)
{
    int c;
    int t;
    int j;
    int *covPtr;

    c = 0;
    t = 0;
    covPtr = covVector + func->blkCovStart;
    for (j = 0; j < (int) func->n_blk; ++j) {
	if (covPtr[j] != -1) {
	    ++t;
	    c += covPtr[j];
	}
    }

    *cov = c;
    *tot = t;
}

static void
cUseCov(T_FUNC * func,
	int *covVector,
	int *cov,
	int *tot)
{
    int c;
    int t;
    int j;
    int *covPtr;

    t = func->formalN_cuse;

    if (covVector[func->blkCovStart] == 1) {
	c = func->formalN_cuse - func->n_cuse;
    } else {
	c = 0;
    }

    covPtr = covVector + func->cUseCovStart;
    for (j = 0; j < (int) func->n_cuse; ++j) {
	if (covPtr[j] == -1) {
	    --t;
	} else {
	    c += covPtr[j];
	}
    }

    *cov = c;
    *tot = t;
}

static void
pUseCov(T_FUNC * func,
	int *covVector,
	int *cov,
	int *tot)
{
    int c;
    int t;
    int j;
    int *covPtr;
    int decis_var;

    decis_var = func->decis_var;

    t = func->formalN_puse;

    if (covVector[func->blkCovStart]) {
	c = func->formalN_puse - func->n_puse;
    } else {
	c = 0;
    }

    covPtr = covVector + func->pUseCovStart;
    for (j = 0; j < (int) func->n_puse; ++j) {
	if (func->puse[j].varno == decis_var || covPtr[j] == -1) {
	    --t;
	} else {
	    c += covPtr[j];
	}
    }

    *cov = c;
    *tot = t;
}

static void
decisCov(T_FUNC * func,
	 int *covVector,
	 int *cov,
	 int *tot)
{
    int c;
    int t;
    int j;
    int *covPtr;
    int decis_var;

    decis_var = func->decis_var;

    c = 0;
    t = 0;

    covPtr = covVector + func->pUseCovStart;
    for (j = 0; j < (int) func->n_puse; ++j) {
	if (func->puse[j].varno != decis_var)
	    break;
	if (covPtr[j] != -1) {
	    ++t;
	    c += covPtr[j];
	}
    }

    *cov = c;
    *tot = j;
}

static int *
mkVector(int covCount)
{
    int *cov;
    int j;

    cov = (int *) malloc((size_t) covCount * sizeof *cov);
    CHECK_MALLOC(cov);

    for (j = 0; j < covCount; ++j) {
	cov[j] = 0;
    }

    return cov;
}

static void
addVector(int *v1,
	  int *v2,
	  int covCount)
{
    int j;

    for (j = 0; j < covCount; ++j) {
	if (v2[j] == 1) {
	    v1[j] = 1;
	}
    }
}

static void
totAddVector(int *v1,
	     int *v2,
	     int covCount)
{
    int j;

    for (j = 0; j < covCount; ++j) {
	if (v2[j] != -1) {
	    v1[j] += v2[j];
	}
    }
}

/*
* format: Print what percent cov is of tot (rounded) and print cov and tot.
*/
static void
format(int cov,
       int tot,
       int counterAtac)
{
    int z;
    char buf[20];

    if (counterAtac) {
	sprintf(buf, "%d(%d)", cov, tot);
    } else {
	if (tot == 0) {
	    z = 100;		/* 0 of 0 is 100% */
	} else {
	    z = (cov * 100 + tot / 2) / tot;	/* % of tot rounded. */
	    if (z == 100 && cov != tot)		/* 99% < x < 100% round down. */
		z = 99;
	}
	if (z < 100)
	    sprintf(buf, "%2d(%d/%d)", z, cov, tot);
	else
	    sprintf(buf, "%d(%d)", z, tot);
    }
    printf("%-13.13s ", buf);
}

static void
doLine(T_MODULE * modules,
       int n_mod,
       int *covVector,
       int options,
       int iMod,
       int iFunc)
{
    int i;
    int cov;
    int tot;
    int c_cov[C_COUNT];		/* per test coverage */
    int c_tot[C_COUNT];		/* per test totals */
    T_MODULE *mod;
    T_MODULE *modEnd;
    T_FUNC *func;
    T_FUNC *funcEnd;
    int counterAtac;

    for (i = 0; i < C_COUNT; ++i) {
	c_cov[i] = 0;
	c_tot[i] = 0;
    }

    if (iMod >= 0) {
	mod = modules + iMod;
	modEnd = mod + 1;
    } else {
	mod = modules;
	modEnd = modules + n_mod;
    }
    for (; mod < modEnd; ++mod) {
	if (mod->ignore)
	    continue;
	if (iFunc >= 0) {
	    func = mod->func + iFunc;
	    funcEnd = func + 1;
	} else {
	    func = mod->func;
	    funcEnd = mod->func + mod->n_func;
	}
	for (; func < funcEnd; ++func) {
	    if (func->ignore)
		continue;
	    /*
	     * Function entry.
	     */
	    if (options & OPTION_F_ENTRY) {
		fEntryCov(func, covVector, &cov, &tot);
		c_cov[C_F_ENTRY] += cov;
		c_tot[C_F_ENTRY] += tot;
	    }

	    /*
	     * Blocks.
	     */
	    if (options & OPTION_BLOCK) {
		blkCov(func, covVector, &cov, &tot);
		c_cov[C_BLOCK] += cov;
		c_tot[C_BLOCK] += tot;
	    }

	    /*
	     * Decisions
	     */
	    if (options & (OPTION_DECIS)) {
		decisCov(func, covVector, &cov, &tot);
		c_cov[C_DECIS] += cov;
		c_tot[C_DECIS] += tot;
	    }

	    /*
	     * C-Uses.
	     */
	    if (options & (OPTION_CUSE | OPTION_ALLUSE)) {
		cUseCov(func, covVector, &cov, &tot);
		c_cov[C_C_USE] += cov;
		c_tot[C_C_USE] += tot;
	    }

	    /*
	     * P-Uses.
	     */
	    if (options & (OPTION_PUSE | OPTION_ALLUSE)) {
		pUseCov(func, covVector, &cov, &tot);
		c_cov[C_P_USE] += cov;
		c_tot[C_P_USE] += tot;
	    }
	}
    }

    if (options & OPTION_COUNTER_ATAC)
	counterAtac = 1;
    else
	counterAtac = 0;

    if (options & OPTION_F_ENTRY) {
	format(c_cov[C_F_ENTRY], c_tot[C_F_ENTRY], counterAtac);
    }
    if (options & OPTION_BLOCK) {
	format(c_cov[C_BLOCK], c_tot[C_BLOCK], counterAtac);
    }
    if (options & OPTION_DECIS) {
	format(c_cov[C_DECIS], c_tot[C_DECIS], counterAtac);
    }
    if (options & OPTION_CUSE) {
	format(c_cov[C_C_USE], c_tot[C_C_USE], counterAtac);
    }
    if (options & OPTION_PUSE) {
	format(c_cov[C_P_USE], c_tot[C_P_USE], counterAtac);
    }
    if (options & OPTION_ALLUSE) {
	format(c_cov[C_C_USE] + c_cov[C_P_USE],
	       c_tot[C_C_USE] + c_tot[C_P_USE], counterAtac);
    }
}

static void
grandTotal(T_MODULE * modules,
	   int n_mod,
	   int *covVector,
	   int options)
{
    heading(options, "");
    doLine(modules, n_mod, covVector, options, -1, -1);
    printf("== total ==\n");
}

static void
perFunc(T_MODULE * modules,
	int n_mod,
	int *covVector,
	int options,
	int byFile)
{
    int i;
    int j;
    int nLine;

    heading(options, "function");

    nLine = 0;
    for (i = 0; i < n_mod; ++i) {
	if (modules[i].ignore)
	    continue;
	if (byFile) {
	    if (i != 0) {
		printf("\n");
		nLine = 0;
	    }
	    printf("file: %s\n", modules[i].file[0].filename);
	}
	for (j = 0; j < (int) modules[i].n_func; ++j) {
	    if (modules[i].func[j].ignore)
		continue;
	    doLine(modules, n_mod, covVector, options, i, j);
	    printf("%s\n", modules[i].func[j].fname);
	    ++nLine;
	}
	if (byFile && nLine > 1) {
	    doLine(modules, n_mod, covVector, options, i, -1);
	    printf("== total ==\n");
	}
    }

    if (!byFile && nLine > 1) {
	doLine(modules, n_mod, covVector, options, -1, -1);
	printf("== total ==\n");
    }
}

static void
perFile(T_MODULE * modules,
	int n_mod,
	int *covVector,
	int options)
{
    int i;
    int nLine;

    heading(options, "source file");

    nLine = 0;
    for (i = 0; i < n_mod; ++i) {
	if (modules[i].ignore)
	    continue;
	doLine(modules, n_mod, covVector, options, i, -1);
	printf("%s\n", modules[i].file[0].filename);
	++nLine;
    }

    if (nLine > 1) {
	doLine(modules, n_mod, covVector, options, -1, -1);
	printf("== total ==\n");
    }
}

static void
perTest(T_MODULE * modules,
	int n_mod,
	int nCov,
	T_TESTLIST * covList,
	int covCount,
	int options)
{
    int k;
    int nLine;
    int *covVector;
    long cost;

    heading(options, "test");

    cost = 0;
    covVector = mkVector(covCount);

    nLine = 0;
    for (k = 0; k < nCov; ++k) {
	cost += covList[k].cost;
	if (options & OPTION_COUNTER_ATAC)
	    totAddVector(covVector, covList[k].cov, covCount);
	else
	    addVector(covVector, covList[k].cov, covCount);
	if (options & OPTION_CUMULATIVE) {
	    if (options & OPTION_COST)
		putCost(cost);
	    doLine(modules, n_mod, covVector, options, -1, -1);
	} else {
	    if (options & OPTION_COST)
		putCost(covList[k].cost);
	    doLine(modules, n_mod, covList[k].cov, options, -1, -1);
	}
	printf("%s\n", covList[k].name);
	++nLine;
    }

    if (nLine > 1 && !(options & OPTION_CUMULATIVE)) {
	if (options & OPTION_COST)
	    putCost(cost);
	doLine(modules, n_mod, covVector, options, -1, -1);
	if (options & OPTION_COUNTER_ATAC)
	    printf("== total ==\n");
	else
	    printf("== all ==\n");
    }

    free(covVector);
}

static void
perFuncPerTest(T_MODULE * modules,
	       int n_mod,
	       int nCov,
	       T_TESTLIST * covList,
	       int covCount,
	       int options)
{
    int i;
    int j;
    int k;
    int *covVector;
    long cost;

    for (i = 0; i < n_mod; ++i) {
	if (modules[i].ignore)
	    continue;
	for (j = 0; j < (int) modules[i].n_func; ++j) {
	    if (modules[i].func[j].ignore)
		continue;
	    if (i + j != 0) {
		printf("\n");
	    }
	    printf("function: %s\n", modules[i].func[j].fname);
	    heading(options, "test");
	    cost = 0;
	    covVector = mkVector(covCount);
	    for (k = 0; k < nCov; ++k) {
		cost += covList[k].cost;
		if (options & OPTION_COUNTER_ATAC)
		    totAddVector(covVector, covList[k].cov, covCount);
		else
		    addVector(covVector, covList[k].cov, covCount);
		if (options & OPTION_CUMULATIVE) {
		    if (options & OPTION_COST)
			putCost(cost);
		    doLine(modules, n_mod, covVector, options, i, j);
		} else {
		    if (options & OPTION_COST)
			putCost(covList[k].cost);
		    doLine(modules, n_mod, covList[k].cov, options, i, j);
		}
		printf("%s\n", covList[k].name);
	    }
	    if (nCov > 1 && !(options & OPTION_CUMULATIVE)) {
		if (options & OPTION_COST)
		    putCost(cost);
		doLine(modules, n_mod, covVector, options, i, j);
		if (options & OPTION_COUNTER_ATAC)
		    printf("== total ==\n");
		else
		    printf("== all ==\n");
	    }
	    free(covVector);
	}
    }
}

static void
perFilePerTest(T_MODULE * modules,
	       int n_mod,
	       int nCov,
	       T_TESTLIST * covList,
	       int covCount,
	       int options)
{
    int i;
    int k;
    int *covVector;
    long cost;

    for (i = 0; i < n_mod; ++i) {
	if (modules[i].ignore)
	    continue;
	if (i != 0) {
	    printf("\n");
	}
	printf("file: %s\n", modules[i].file[0].filename);
	heading(options, "test");
	cost = 0;
	covVector = mkVector(covCount);
	for (k = 0; k < nCov; ++k) {
	    cost += covList[k].cost;
	    if (options & OPTION_COUNTER_ATAC)
		totAddVector(covVector, covList[k].cov, covCount);
	    else
		addVector(covVector, covList[k].cov, covCount);
	    if (options & OPTION_CUMULATIVE) {
		if (options & OPTION_COST)
		    putCost(cost);
		doLine(modules, n_mod, covVector, options, i, -1);
	    } else {
		if (options & OPTION_COST)
		    putCost(covList[k].cost);
		doLine(modules, n_mod, covList[k].cov, options, i, -1);
	    }
	    printf("%s\n", covList[k].name);
	}
	if (nCov > 1 && !(options & OPTION_CUMULATIVE)) {
	    if (options & OPTION_COST)
		putCost(cost);
	    doLine(modules, n_mod, covVector, options, i, -1);
	    if (options & OPTION_COUNTER_ATAC)
		printf("== total ==\n");
	    else
		printf("== all ==\n");
	}
	free(covVector);
    }
}

void
summary(T_MODULE * modules,
	int n_mod,
	int nCov,
	T_TESTLIST * covList,
	int covCount,
	int byFunc,
	int byFile,
	int options)
{
    if (nCov == 1) {
	if (byFunc) {
	    perFunc(modules, n_mod, covList[0].cov, options, byFile);
	} else if (byFile) {
	    perFile(modules, n_mod, covList[0].cov, options);
	} else {
	    grandTotal(modules, n_mod, covList[0].cov, options);
	}
    } else if (byFunc) {
	perFuncPerTest(modules, n_mod, nCov, covList, covCount, options);
    } else if (byFile) {
	perFilePerTest(modules, n_mod, nCov, covList, covCount, options);
    } else {
	perTest(modules, n_mod, nCov, covList, covCount, options);
    }
}
