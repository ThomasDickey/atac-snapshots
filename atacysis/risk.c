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

static char risk_c[] = 
	"$Header: /users/source/archives/atac.vcs/atacysis/RCS/risk.c,v 3.6 1994/04/04 10:26:11 jrh Exp $";
/*
*-----------------------------------------------$Log: risk.c,v $
*-----------------------------------------------Revision 3.6  1994/04/04 10:26:11  jrh
*-----------------------------------------------FROM_KEYS
*-----------------------------------------------
*Revision 3.6  94/04/04  10:26:11  jrh
*Add Release Copyright
*
*Revision 3.5  93/11/02  11:49:35  saul
*Same
*
*Revision 3.3  93/08/04  15:58:06  ewk
*Added MVS and solaris support.  Squelched some ANSI warnings.
*
* Revision 3.2  93/03/29  11:19:38  saul
*Don't core dump when nothing is covered.
*
*Revision 3.1  93/02/23  10:52:52  saul
*new internal risk data format
*
*Revision 3.0  92/11/06  07:48:18  saul
*propagate to version 3.0
*
*Revision 2.4  92/10/30  09:55:33  saul
*include portable.h
*
*Revision 2.3  92/10/28  09:06:21  saul
*remove enum for portability
*
*Revision 2.2  92/09/08  08:48:45  saul
*New coverage vector data structure and options formats.
*
*Revision 2.1  92/07/10  13:46:47  saul
*new
*
*-----------------------------------------------end of log
*/
#include <stdio.h>
#include "portable.h"
#include "atacysis.h"

/* forward declarations */
void risk();
static void doSum();
static void doBlk();

#define CHECK_MALLOC(p) if((p)==NULL)fprintf(stderr,"Out of memory\n"),exit(1)

typedef int COVTYPE;
#define COV_BLOCK	((COVTYPE)0)
#define	COV_DECIS	((COVTYPE)1)
#define COV_CUSE	((COVTYPE)2)
#define COV_PUSE	((COVTYPE)3)
#define COV_N		4

#define AMT_BLOCK	1.0
#define AMT_DPRED	1.0
#define AMT_DTARG	1.0
#define AMT_CDEF	1.0
#define AMT_CUSE	1.0
#define AMT_PDEF	1.0
#define AMT_PPRED	1.0
#define AMT_PTARG	1.0

static void
doBlk(blk, mark, maxLine, total, covered, covType, amount)
T_BLK	*blk;
int	mark;
int	*maxLine;
float	(**total)[COV_N];
float	(**covered)[COV_N];
COVTYPE	covType;
float	amount;
{
    int		sLine;
    int		eLine;
    int		max;
    float	(*t)[COV_N];
    float	(*c)[COV_N];
    int		i;
    int		j;
    float	fraction;

    if (blk->pos.start.file != 0) return;
    if (blk->pos.end.file != 0) return;
    sLine = blk->pos.start.line;
    eLine = blk->pos.end.line;
    if (eLine < sLine) {
	sLine = eLine;
	eLine = blk->pos.start.line;
    }
    if (sLine < 1) return;
    max = *maxLine;
    t = *total;
    c = *covered;
    if (eLine > max) {
	if (max == 0) {
	    t = (float (*)[COV_N])malloc((eLine + 1) * sizeof *t * COV_N);
	    c = (float (*)[COV_N])malloc((eLine + 1) * sizeof *c * COV_N);
	} else {
	    t = (float (*)[COV_N])realloc(t, (eLine + 1) * sizeof *t * COV_N);
	    c = (float (*)[COV_N])realloc(c, (eLine + 1) * sizeof *c * COV_N);
	}
	CHECK_MALLOC(t);
	CHECK_MALLOC(c);
	for (++max; max <= eLine; ++max) {
	    for (j = 0; j < COV_N; ++j) {
		t[max][j] = 0;
		c[max][j] = 0;
	    }
	}
	*maxLine = max;
	*total = t;
	*covered = c;
    }
    
    fraction = amount/(eLine - sLine + 1);

    for (i = sLine; i <= eLine; ++i)
	t[i][covType] += fraction;
    if (mark) for (i = sLine; i <= eLine; ++i)
	c[i][covType] += fraction;
}

static void
doSum(perLine, sum, start, end)
float	(*perLine)[COV_N];
float	*sum;
int	start;
int	end;
{
    int i;
    int j;

    for (i = 0; i < COV_N; ++i) {
	sum[i] = 0.0;
    }

    if (perLine == NULL) return;

    for (i = start; i <= end; ++i) {
	for (j = 0; j < COV_N; ++j)
	    sum[j] += perLine[i][j];
    }
}

void
risk(modules, n_mod, covVector, options)
T_MODULE	*modules;
int		n_mod;
int		*covVector;
int		options;
{
    int		i;
    int		j;
    int		cov;
    T_MODULE	*mod;
    int		decis_var;
    int		maxLine;
    float	(*total)[COV_N];
    float	(*covered)[COV_N];
    float	sum[COV_N];
    T_FUNC	*func;
    int		nFunc;
    int		start;
    int		end;

    total = NULL;
    covered = NULL;

    for (mod = modules; mod < modules + n_mod; ++mod) {
	if (mod->n_file < 1) continue;
	maxLine = 0;
	nFunc = 0;
	for (i = 0; i < (int)mod->n_func; ++i) {
	    func = mod->func + i;
	    if (func->ignore) continue;
	    ++nFunc;
	    /*
	     * Blocks.
	     */
	    if (options & OPTION_BLOCK) for(j = 1; j < (int)func->n_blk; ++j) {
		cov = covVector[func->blkCovStart + j];
		if (cov == -1)
		    cov = 0;
		doBlk(func->blk + j, cov,
		      &maxLine, &total, &covered, COV_BLOCK, AMT_BLOCK);
	    }

	    /*
	     * Decisions.
	     */
	    if (options & OPTION_DECIS) for(j = 0; j < (int)func->n_puse; ++j) {
		decis_var = func->decis_var;
		if (func->puse[j].varno != decis_var) break;
		cov = covVector[func->pUseCovStart + j];
		if (cov == -1)
		    cov = 0;
		doBlk(func->blk + func->puse[j].blk2, cov,
		      &maxLine, &total, &covered, COV_DECIS, AMT_DPRED);
		doBlk(func->blk + func->puse[j].blk3, cov,
		      &maxLine, &total, &covered, COV_DECIS, AMT_DTARG);
	    }

	    /*
	     * C-uses
	     */
	    if (options & OPTION_CUSE) for(j = 0; j < (int)func->n_cuse; ++j) {
		cov = covVector[func->cUseCovStart + j];
		if (cov == -1)
		    cov = 0;
		doBlk(func->blk + func->cuse[j].blk1, cov,
		      &maxLine, &total, &covered, COV_CUSE, AMT_CDEF);
		doBlk(func->blk + func->cuse[j].blk2, cov,
		      &maxLine, &total, &covered, COV_CUSE, AMT_CUSE);
	    }

	    /*
	     * P-uses
	     */
	    if (options & OPTION_PUSE) for(j = 0; j < (int)func->n_puse; ++j) {
		decis_var = func->decis_var;
		if (func->puse[j].varno == decis_var) continue;
		cov = (covVector[func->pUseCovStart + j]);
		if (cov == -1)
		    cov = 0;
		doBlk(func->blk + func->puse[j].blk1, cov,
		      &maxLine, &total, &covered, COV_PUSE, AMT_PDEF);
		doBlk(func->blk + func->puse[j].blk2, cov,
		      &maxLine, &total, &covered, COV_PUSE, AMT_PPRED);
		doBlk(func->blk + func->puse[j].blk3, cov,
		      &maxLine, &total, &covered, COV_PUSE, AMT_PTARG);
	    }
	}

	/*
	* F filename #units #lines { cov types } { totals } { covered }
	*/
	printf("F %s %d %d {", mod->file[0].filename, nFunc, maxLine);
	if (options & OPTION_BLOCK) printf(" b");
	if (options & OPTION_CUSE) printf(" c");
	if (options & OPTION_DECIS) printf(" d");
	if (options & OPTION_PUSE) printf(" p");
	printf(" } {");

	/*
	 * Totals:
	 */
	doSum(total, sum, 1, maxLine);
	if (options & OPTION_BLOCK) printf(" %.1f", sum[COV_BLOCK]);
	if (options & OPTION_CUSE) printf(" %.1f", sum[COV_CUSE]);
	if (options & OPTION_DECIS) printf(" %.1f", sum[COV_DECIS]);
	if (options & OPTION_PUSE) printf(" %.1f", sum[COV_PUSE]);

	/*
	 * Covered:
	 */
	printf(" } {");
	doSum(covered, sum, 1, maxLine);
	if (options & OPTION_BLOCK) printf(" %.1f", sum[COV_BLOCK]);
	if (options & OPTION_CUSE) printf(" %.1f", sum[COV_CUSE]);
	if (options & OPTION_DECIS) printf(" %.1f", sum[COV_DECIS]);
	if (options & OPTION_PUSE) printf(" %.1f", sum[COV_PUSE]);
	printf(" }\n");

	for (i = 0; i < (int)mod->n_func; ++i) {
	    func = mod->func + i;
	    if (func->ignore) continue;
	    start = func->blk[0].pos.start.line;
	    end = func->blk[0].pos.end.line;
	    if (start > end) {
		start = end;
		end = func->blk[0].pos.start.line;
	    }

	    /*
	     * U funcname #funcs beginLine endLine { totals } { covered }
	     */
	    printf("U %s %d %d {", func->fname, start, end);
	    /*
	     * Totals:
	     */
	    doSum(total, sum, start, end);
	    if (options & OPTION_BLOCK) printf(" %.1f", sum[COV_BLOCK]);
	    if (options & OPTION_CUSE) printf(" %.1f", sum[COV_CUSE]);
	    if (options & OPTION_DECIS) printf(" %.1f", sum[COV_DECIS]);
	    if (options & OPTION_PUSE) printf(" %.1f", sum[COV_PUSE]);

	    /*
	     * Covered:
	     */
	    printf(" } {");
	    doSum(covered, sum, start, end);
	    if (options & OPTION_BLOCK) printf(" %.1f", sum[COV_BLOCK]);
	    if (options & OPTION_CUSE) printf(" %.1f", sum[COV_CUSE]);
	    if (options & OPTION_DECIS) printf(" %.1f", sum[COV_DECIS]);
	    if (options & OPTION_PUSE) printf(" %.1f", sum[COV_PUSE]);
	    printf(" }\n");

	    /*
	     * L { totals } { covered }
	     */
	    if (total == NULL || covered == NULL)
		continue;
	    for (j = start; j <= end; ++j) {
		printf("L {");
		if (options & OPTION_BLOCK)
		    printf(" %.1f", total[j][COV_BLOCK]);
		if (options & OPTION_CUSE)
		    printf(" %.1f", total[j][COV_CUSE]);
		if (options & OPTION_DECIS)
		    printf(" %.1f", total[j][COV_DECIS]);
		if (options & OPTION_PUSE)
		    printf(" %.1f", total[j][COV_PUSE]);
		printf(" } {");
		if (options & OPTION_BLOCK)
		    printf(" %.1f", covered[j][COV_BLOCK]);
		if (options & OPTION_CUSE)
		    printf(" %.1f", covered[j][COV_CUSE]);
		if (options & OPTION_DECIS)
		    printf(" %.1f", covered[j][COV_DECIS]);
		if (options & OPTION_PUSE)
		    printf(" %.1f", covered[j][COV_PUSE]);
		printf(" }\n");
	    }
	}

	free(total);
	free(covered);
    }
}
