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

static char vector_c[] =
	"$Header: /users/source/archives/atac.vcs/atacysis/RCS/vector.c,v 3.5 1994/04/04 10:26:34 jrh Exp $";
/*
*-----------------------------------------------$Log: vector.c,v $
*-----------------------------------------------Revision 3.5  1994/04/04 10:26:34  jrh
*-----------------------------------------------FROM_KEYS
*-----------------------------------------------
*Revision 3.5  94/04/04  10:26:34  jrh
*Add Release Copyright
*
*Revision 3.4  93/11/02  11:51:02  saul
*Same as revision 3.2
*
*Revision 3.2  93/08/04  15:59:27  ewk
*Added MVS and solaris support.  Squelched some ANSI warnings.
*
*Revision 3.1  93/06/30  15:12:19  saul
*fix input to minimize (atacmin) for -m d, c, and p
*
*Revision 3.0  92/11/06  07:48:13  saul
*propagate to version 3.0
*
*Revision 2.6  92/10/30  09:55:54  saul
*include portable.h
*
*Revision 2.5  92/09/08  08:47:40  saul
*Simplified by new coverage vector data structure.
*
*Revision 2.4  92/07/10  11:23:24  saul
*-bcdp options for minimization vector
*
*Revision 2.3  92/03/17  15:27:18  saul
*copyright
*
*Revision 2.2  91/08/14  11:54:50  saul
*missing blocks bug
*
*Revision 2.1  91/08/14  09:10:13  saul
*new. currently blocks only
*
*-----------------------------------------------end of log
*/
#include <stdio.h>
#include "portable.h"
#include "atacysis.h"

/* forward declarations */
void vectorPut();
static void decisCov();
static void pUseCov();
static void cUseCov();
static void blkCov();
static void fEntryCov();
static void putBitEnd();

#define putBit(b) ((b)?putchar('x'):putchar(' '))

static void
putBitEnd()
{
    putchar('\n');
}

static void
fEntryCov(func, covVector)
T_FUNC	*func;
int	*covVector;
{
    if (covVector[func->blkCovStart] != -1) {
	putBit(covVector[func->blkCovStart]);
    }
}

static void
blkCov(func, covVector)
T_FUNC	*func;
int	*covVector;
{
    int	j;
    int	*covPtr;

    covPtr = covVector + func->blkCovStart;
    for (j = 0; j < (int)func->n_blk; ++j) {
	if (covPtr[j] != -1) {
	    putBit(covPtr[j]);
	}
    }
}

static void
cUseCov(func, covVector)
T_FUNC	*func;
int	*covVector;
{
    int	c;
    int	j;
    int	*covPtr;

    c = func->formalN_cuse - func->n_cuse;

    if (covVector[func->blkCovStart] == 1) {
	for (j = 0; j < c; ++j)
	    putBit(1);
    } else {
	for (j = 0; j < c; ++j)
	    putBit(0);
    }

    covPtr = covVector + func->cUseCovStart;
    for(j = 0; j < (int)func->n_cuse; ++j) {
	if (covPtr[j] != -1) {
	    putBit(covPtr[j]);
	}
    }
}

static void
pUseCov(func, covVector)
T_FUNC	*func;
int	*covVector;
{
    int	c;
    int	j;
    int	*covPtr;
    int	decis_var;

    decis_var = func->decis_var;

    c = func->formalN_puse - func->n_puse;

    if (covVector[func->blkCovStart]) {
	for (j = 0; j < c; ++j)
	    putBit(1);
    } else {
	for (j = 0; j < c; ++j)
	    putBit(0);
    }

    covPtr = covVector + func->pUseCovStart;
    for(j = 0; j < (int)func->n_puse; ++j) {
	if (func->puse[j].varno != decis_var && covPtr[j] != -1) {
	    putBit(covPtr[j]);
	}
    }
}

static void
decisCov(func, covVector)
T_FUNC	*func;
int	*covVector;
{
    int	c;
    int	j;
    int	*covPtr;
    int	decis_var;

    decis_var = func->decis_var;

    covPtr = covVector + func->pUseCovStart;
    for(j = 0; j < (int)func->n_puse; ++j) {
	if (func->puse[j].varno != decis_var) break;
	if (covPtr[j] != -1) {
	    putBit(covPtr[j]);
	}
    }
}

void
vectorPut(modules, n_mod, covVector, options)
T_MODULE	*modules;
int		n_mod;
int		*covVector;
int		options;
{
    T_MODULE	*mod;
    T_MODULE	*modEnd;
    T_FUNC	*func;
    T_FUNC	*funcEnd;

    mod = modules;
    modEnd = modules + n_mod;
    for (; mod < modEnd; ++mod) {
	if (mod->ignore) continue;
	func = mod->func;
	funcEnd = mod->func + mod->n_func;
	for (; func < funcEnd; ++func) {
	    if (func->ignore) continue;
	    /*
	     * Function entry.
	     */
	    if (options & OPTION_F_ENTRY) {
		fEntryCov(func, covVector);
	    }

	    /*
	     * Blocks.
	     */
	    if (options & OPTION_BLOCK) {
		blkCov(func, covVector);
	    }

	    /*
	     * Decisions
	     */
	    if (options & (OPTION_DECIS)) {
		decisCov(func, covVector);
	    }

	    /*
	     * C-Uses.
	     */
	    if (options & (OPTION_CUSE | OPTION_ALLUSE)) {
		cUseCov(func, covVector);
	    }

	    /*
	     * P-Uses.
	     */
	    if (options & (OPTION_PUSE | OPTION_ALLUSE)) {
		pUseCov(func, covVector);
	    }
	}
    }
    putBitEnd();
}
