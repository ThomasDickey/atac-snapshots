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

static char const highest_c[] = 
	"$Header: /users/source/archives/atac.vcs/atacysis/RCS/highest.c,v 3.5 1995/12/29 21:24:41 tom Exp $";
/*
* $Log: highest.c,v $
* Revision 3.5  1995/12/29 21:24:41  tom
* adjust headers, prototyped for autoconfig
*
*Revision 3.4  94/04/04  10:25:26  jrh
*Add Release Copyright
*
*Revision 3.3  93/09/03  09:50:14  saul
*Put back type casts (as in revision 3.1).  Keep other changes.
*
*Revision 3.2  93/08/23  15:42:31  ewk
*Eliminated many casts for Solaris warnings by modifying type decls.
*
*Revision 3.1  93/08/04  15:54:46  ewk
*Added MVS and solaris support.  Squelched some ANSI warnings.
*
*Revision 3.0  92/11/06  07:47:16  saul
*propagate to version 3.0
*
*Revision 2.5  92/11/02  11:43:04  saul
*remove unused variables
*
*Revision 2.4  92/10/30  09:54:20  saul
*include portable.h
*
*Revision 2.3  92/10/05  11:36:43  saul
*Change "total" to "all"
*
*Revision 2.2  92/09/30  11:12:51  saul
*Suppress header with -h option.
*
*Revision 2.1  92/09/08  09:18:55  saul
*Highest coverage summary feature
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

/* forward declarations */
static void perFilePerTest
	P_((T_MODULE *modules, int n_mod, int nCov, T_TESTLIST *covList, int
	covCount, int options));
static void perFuncPerTest
	P_((T_MODULE *modules, int n_mod, int nCov, T_TESTLIST *covList, int
	covCount, int options));
static void perTest
	P_((T_MODULE *modules, int n_mod, int nCov, T_TESTLIST *covList, int
	covCount, int options));
static void perFile
	P_((T_MODULE *modules, int n_mod, int *covVector, int covCount, int
	options));
static void perFunc
	P_((T_MODULE *modules, int n_mod, int *covVector, int covCount, int
	options, int byFile));
static void grandTotal
	P_((T_MODULE *modules, int n_mod, int *covVector, int covCount, int
	options));
static void doLine
	P_((T_MODULE *modules, int n_mod, int *covVector, int options, int
	iMod, int iFunc));
static void format
	P_((int cov));
static int *totVector
	P_((int nCov, T_TESTLIST *covList, int covCount));
static void decisCov
	P_((T_FUNC *func, int *covVector, int *cov));
static void pUseCov
	P_((T_FUNC *func, int *covVector, int *cov));
static void cUseCov
	P_((T_FUNC *func, int *covVector, int *cov));
static void blkCov
	P_((T_FUNC *func, int *covVector, int *cov));
static void fEntryCov
	P_((T_FUNC *func, int *covVector, int *cov));
static void heading
	P_((int options, char *label));

#define CHECK_MALLOC(p) if((p)==NULL)fprintf(stderr,"Out of memory\n"),exit(1)

#define LINEMAX	100

#define C_BLOCK	0
#define C_DECIS	1
#define C_C_USE 2
#define C_P_USE 3
#define C_F_ENTRY 4
#define C_COUNT 5

static void
heading(options, label)
int	options;
char	*label;
{
    int	fields;

    if (options & OPTION_NO_HEADER) return;

    fields = 0;

    if (options & OPTION_F_ENTRY) {
	printf("%-12s", "# functions");
	++fields;
    }
    if (options & OPTION_BLOCK) {
	printf("%-12s", "# blocks");
	++fields;
    }
    if (options & OPTION_DECIS) {
	printf("%-12s", "# decisions");
	++fields;
    }
    if (options & OPTION_CUSE) {
	printf("%-12s", "# C-Uses");
	++fields;
    }
    if (options & OPTION_PUSE) {
	printf("%-12s", "# P-Uses");
	++fields;
    }
    if (options & OPTION_ALLUSE) {
	printf("%-12s", "# All-Uses");
	++fields;
    }
    printf("%s\n", label);

    if (*label) {
	++fields;
    }
    while (fields--) {
	printf("%-11.11s ", "------------------------");
    }
    printf("\n");
}

static void
fEntryCov(func, covVector, cov)
T_FUNC	*func;
int	*covVector;
int	*cov;
{
    if (covVector[func->blkCovStart] == -1) {
	*cov = 0;
    } else {
	*cov = covVector[func->blkCovStart];
    }
}

static void
blkCov(func, covVector, cov)
T_FUNC	*func;
int	*covVector;
int	*cov;
{
    int	c;
    int t;
    int	j;
    int	*covPtr;

    c = 0;
    t = 0;
    covPtr = covVector + func->blkCovStart;
    for(j = 0; j < (int)func->n_blk; ++j) {
	if (covPtr[j] != -1) {
	    ++t;
	    if (c < covPtr[j]) c = covPtr[j];
	}
    }

    *cov = c;
}

static void
cUseCov(func, covVector, cov)
T_FUNC	*func;
int	*covVector;
int	*cov;
{
    int	c;
    int t;
    int	j;
    int	*covPtr;

    t = func->formalN_cuse;

    if (covVector[func->blkCovStart] == 1) {
	c = func->formalN_cuse - func->n_cuse;
    } else {
	c = 0;
    }

    covPtr = covVector + func->cUseCovStart;
    for(j = 0; j < (int)func->n_cuse; ++j) {
	if (covPtr[j] == -1) {
	    --t;
	}
	else {
	    if (c < covPtr[j])  c = covPtr[j];
	}
    }

    *cov = c;
}

static void
pUseCov(func, covVector, cov)
T_FUNC	*func;
int	*covVector;
int	*cov;
{
    int	c;
    int t;
    int	j;
    int	*covPtr;
    int	decis_var;

    decis_var = func->decis_var;

    t = func->formalN_puse;

    if (covVector[func->blkCovStart]) {
	c = func->formalN_puse - func->n_puse;
    } else {
	c = 0;
    }

    covPtr = covVector + func->pUseCovStart;
    for(j = 0; j < (int)func->n_puse; ++j) {
	if (func->puse[j].varno == decis_var || covPtr[j] == -1) {
	    --t;
	} else {
	    if (c < covPtr[j]) c = covPtr[j];
	}
    }

    *cov = c;
}

static void
decisCov(func, covVector, cov)
T_FUNC	*func;
int	*covVector;
int	*cov;
{
    int	c;
    int t;
    int	j;
    int	*covPtr;
    int	decis_var;

    decis_var = func->decis_var;

    c = 0;
    t = 0;

    covPtr = covVector + func->pUseCovStart;
    for(j = 0; j < (int)func->n_puse; ++j) {
	if (func->puse[j].varno != decis_var) break;
	if (covPtr[j] != -1) {
	    ++t;
	    if (c < covPtr[j]) c = covPtr[j];
	}
    }

    *cov = c;
}

static int *
totVector(nCov, covList, covCount)
int		nCov;
T_TESTLIST	*covList;
int		covCount;
{
    int	*cov;
    int	j;
    int	k;

    cov = (int *)malloc(covCount * sizeof *cov);
    CHECK_MALLOC(cov);

    for (j = 0; j < covCount; ++j) {
	cov[j] = 0;
    }

    for (k = 0; k < nCov; ++k) {
	for (j = 0; j < covCount; ++j) {
	    if (covList[k].cov[j] != -1) {
		cov[j] += covList[k].cov[j];
	    }
	}
    }

    return cov;
}

static void
format(cov)
int	cov;
{
    char	buf[20];
    sprintf(buf, "%6d ", cov);
    printf("%-11.11s ", buf);
}

static void
doLine(modules, n_mod, covVector, options, iMod, iFunc)
T_MODULE	*modules;
int		n_mod;
int		*covVector;
int		options;
int		iMod;
int		iFunc;
{
    int		i;
    int		cov;
    int		c_cov[C_COUNT];	/* per test coverage */
    T_MODULE	*mod;
    T_MODULE	*modEnd;
    T_FUNC	*func;
    T_FUNC	*funcEnd;

    for (i = 0; i < C_COUNT; ++i) { 
	c_cov[i] = 0;
    }

    if (iMod >= 0) {
	mod = modules + iMod;
	modEnd = mod + 1;
    } else {
	mod = modules;
	modEnd = modules + n_mod;
    }
    for (; mod < modEnd; ++mod) {
	if (mod->ignore) continue;
	if (iFunc >= 0) {
	    func = mod->func + iFunc;
	    funcEnd = func + 1;
	} else {
	    func = mod->func;
	    funcEnd = mod->func + mod->n_func;
	}
	for (; func < funcEnd; ++func) {
	    if (func->ignore) continue;
	    /*
	     * Function entry.
	     */
	    if (options & OPTION_F_ENTRY) {
		fEntryCov(func, covVector, &cov);
		if (c_cov[C_F_ENTRY] < cov) c_cov[C_F_ENTRY] = cov;
	    }

	    /*
	     * Blocks.
	     */
	    if (options & OPTION_BLOCK) {
		blkCov(func, covVector, &cov);
		if (c_cov[C_BLOCK] < cov) c_cov[C_BLOCK] = cov;
	    }

	    /*
	     * Decisions
	     */
	    if (options & (OPTION_DECIS)) {
		decisCov(func, covVector, &cov);
		if (c_cov[C_DECIS] < cov) c_cov[C_DECIS] = cov;
	    }

	    /*
	     * C-Uses.
	     */
	    if (options & (OPTION_CUSE | OPTION_ALLUSE)) {
		cUseCov(func, covVector, &cov);
		if (c_cov[C_C_USE] < cov) c_cov[C_C_USE] = cov;
	    }

	    /*
	     * P-Uses.
	     */
	    if (options & (OPTION_PUSE | OPTION_ALLUSE)) {
		pUseCov(func, covVector, &cov);
		if (c_cov[C_P_USE] < cov) c_cov[C_P_USE] = cov;
	    }
	}
    }

    if (options & OPTION_F_ENTRY) {
	format(c_cov[C_F_ENTRY]);
    }
    if (options & OPTION_BLOCK) {
	format(c_cov[C_BLOCK]);
    }
    if (options & OPTION_DECIS) {
	format(c_cov[C_DECIS]);
    }
    if (options & OPTION_CUSE) {
	format(c_cov[C_C_USE]);
    }
    if (options & OPTION_PUSE) {
	format(c_cov[C_P_USE]);
    }
    if (options & OPTION_ALLUSE) {
	format(c_cov[C_C_USE] + c_cov[C_P_USE]);
    }
}

static void
grandTotal(modules, n_mod, covVector, covCount, options)
T_MODULE	*modules;
int		n_mod;
int		*covVector;
int		covCount;
int		options;
{
    heading(options, "");
    doLine(modules, n_mod, covVector, options, -1, -1);
    printf("== all ==\n"); 
}

static void
perFunc(modules, n_mod, covVector, covCount, options, byFile)
T_MODULE	*modules;
int		n_mod;
int		*covVector;
int		covCount;
int		options;
int		byFile;
{
    int i;
    int j;
    int nLine;

    heading(options, "function");

    nLine = 0;
    for (i = 0; i < n_mod; ++i) {
	if (modules[i].ignore) continue;
	if (byFile) {
	    if (i != 0) {
		printf("\n");
		nLine = 0;
	    }
	    printf("file: %s\n", modules[i].file[0].filename);
	}
	for (j = 0; j < (int)modules[i].n_func; ++j) {
	    if (modules[i].func[j].ignore) continue;
	    doLine(modules, n_mod, covVector, options, i, j);
	    printf("%s\n", modules[i].func[j].fname);
	    ++nLine;
	}
	if (byFile && nLine > 1) {
	    doLine(modules, n_mod, covVector, options, i, -1);
	    printf("== all ==\n"); 
	}
    }

    if (!byFile && nLine > 1) {
	doLine(modules, n_mod, covVector, options, -1, -1);
	printf("== all ==\n"); 
    }
}
		
static void
perFile(modules, n_mod, covVector, covCount, options)
T_MODULE	*modules;
int		n_mod;
int		*covVector;
int		covCount;
int		options;
{
    int i;
    int nLine;

    heading(options, "source file");

    nLine = 0;
    for (i = 0; i < n_mod; ++i) {
	if (modules[i].ignore) continue;
	doLine(modules, n_mod, covVector, options, i, -1);
	printf("%s\n", modules[i].file[0].filename);
	++nLine;
    }

    if (nLine > 1) {
	doLine(modules, n_mod, covVector, options, -1, -1);
	printf("== all ==\n"); 
    }
}
		
static void
perTest(modules, n_mod, nCov, covList, covCount, options)
T_MODULE	*modules;
int		n_mod;
int		nCov;
T_TESTLIST	*covList;
int		covCount;
int		options;
{
    int		k;
    int		nLine;
    int		*covVector;

    heading(options, "test");

    nLine = 0;
    for (k = 0; k < nCov; ++k) {
	doLine(modules, n_mod, covList[k].cov, options, -1, -1);
	printf("%s\n", covList[k].name);
	++nLine;
    }

    if (nLine > 1) {
	covVector = totVector(nCov, covList, covCount);
	doLine(modules, n_mod, covVector, options, -1, -1);
	printf("== all ==\n"); 
	free(covVector);
    }
}

static void
perFuncPerTest(modules, n_mod, nCov, covList, covCount, options)
T_MODULE	*modules;
int		n_mod;
int		nCov;
T_TESTLIST	*covList;
int		covCount;
int		options;
{
    int 	i;
    int		j;
    int		k;
    int		*covVector = NULL;

    if (nCov > 1) {
	covVector = totVector(nCov, covList, covCount);
    }

    for (i = 0; i < n_mod; ++i) {
	if (modules[i].ignore) continue;
	for (j = 0; j < (int)modules[i].n_func; ++j) {
	    if (modules[i].func[j].ignore) continue;
	    if (i + j != 0) {
		printf("\n");
	    }
	    printf("function: %s\n", modules[i].func[j].fname);
	    heading(options, "test");
	    for (k = 0; k < nCov; ++k) {
		doLine(modules, n_mod, covList[k].cov, options, i, j);
		printf("%s\n", covList[k].name);
	    }
	    if (nCov > 1) {
		doLine(modules, n_mod, covVector, options, i, j);
		printf("== all ==\n"); 
	    }
	}
    }

    if (nCov > 1) {
	free(covVector);
    }
}

static void
perFilePerTest(modules, n_mod, nCov, covList, covCount, options)
T_MODULE	*modules;
int		n_mod;
int		nCov;
T_TESTLIST	*covList;
int		covCount;
int		options;
{
    int 	i;
    int		k;
    int		*covVector = NULL;

    if (nCov > 1) {
	covVector = totVector(nCov, covList, covCount);
    }

    for (i = 0; i < n_mod; ++i) {
	if (modules[i].ignore) continue;
	if (i != 0) {
	    printf("\n");
	}
	printf("file: %s\n", modules[i].file[0].filename);
	heading(options, "test");
	for (k = 0; k < nCov; ++k) {
	    doLine(modules, n_mod, covList[k].cov, options, i, -1);
	    printf("%s\n", covList[k].name);
	}
	if (nCov > 1) {
	    doLine(modules, n_mod, covVector, options, i, -1);
	    printf("== all ==\n"); 
	}
    }

    if (nCov > 1) {
	free(covVector);
    }
}

void
highest(modules, n_mod, nCov, covList, covCount, byFunc, byFile, options)
T_MODULE	*modules;
int		n_mod;
int		nCov;
T_TESTLIST	*covList;
int		covCount;
int		byFunc;
int		byFile;
int		options;
{
    if (nCov == 1) {
	if (byFunc) {
	    perFunc(modules, n_mod, covList[0].cov, covCount, options, byFile);
	} else if (byFile) {
	    perFile(modules, n_mod, covList[0].cov, covCount, options);
	} else {
	    grandTotal(modules, n_mod, covList[0].cov, covCount, options);
	}
    }
    else if (byFunc) {
	perFuncPerTest(modules, n_mod, nCov, covList, covCount, options);
    } else if (byFile) {
	perFilePerTest(modules, n_mod, nCov, covList, covCount, options);
    } else {
	perTest(modules, n_mod, nCov, covList, covCount, options);
    }
}
