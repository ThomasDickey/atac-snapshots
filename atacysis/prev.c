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
#include <ctype.h>

#include "portable.h"
#include "atacysis.h"
#include "pack.h"
#include "ramfile.h"
#include "man.h"

static char const prev_c[] = 
	"$Header: /users/source/archives/atac.vcs/atacysis/RCS/prev.c,v 3.4 1995/12/29 21:24:41 tom Exp $";
/*
* $Log: prev.c,v $
* Revision 3.4  1995/12/29 21:24:41  tom
* adjust headers, prototyped for autoconfig
* fix compiler warnings (casts).
*
*Revision 3.3  94/04/04  10:25:58  jrh
*Add Release Copyright
*
*Revision 3.2  93/08/04  15:57:37  ewk
*Added MVS and solaris support.  Squelched some ANSI warnings.
*
* Revision 3.1  93/03/26  11:01:20  saul
*Coverage vector packing. 
*
*Revision 3.0  92/11/06  07:47:27  saul
*propagate to version 3.0
*
*Revision 2.9  92/11/03  10:32:27  saul
*remove unused variable
*
*Revision 2.8  92/11/02  11:43:22  saul
*remove unused variables
*
*Revision 2.7  92/10/30  09:55:23  saul
*include portable.h
*
*Revision 2.6  92/10/29  14:31:14  saul
*atactm may issue inappropriate corruption message.
*
*Revision 2.5  92/10/28  09:04:08  saul
*remove strings.h for portability
*
*Revision 2.4  92/09/30  11:56:29  saul
*Optimize -l and -L by skipping coverage info.
*
*Revision 2.3  92/09/22  15:44:33  saul
*Trace compression
*
*Revision 2.2  92/09/08  10:12:10  saul
*changed trace format and data structures
*
*Revision 2.1  92/09/08  09:59:10  saul
*Purdue trace management
*
*-----------------------------------------------end of log
*/

/* forward declarations */
static void skipCompressed
	P_((struct cfile *cf, char *tracefile, tablestype *tables));
static void getCompressed
	P_((struct cfile *cf, char *tracefile, tablestype *tables));
static void getPCompressed
	P_((struct cfile *cf, char *tracefile, int iTestCount, RAMFILE *rf));
static void getCCompressed
	P_((struct cfile *cf, char *tracefile, int iTestCount, RAMFILE *rf));
static void getBCompressed
	P_((struct cfile *cf, char *tracefile, int iTestCount, RAMFILE *rf));
static void getSCompressed
	P_((struct cfile *cf, char *tracefile, int iTestCount, RAMFILE *rf));
static void getMCompressed
	P_((struct cfile *cf, char *tracefile, RAMFILE *rf));
static void getICompressed
	P_((struct cfile *cf, char *tracefile, membertype *mems));
static void prev_block
	P_((int iBlock, int iTestCount, coveragetype *pCov, functype *func));
static void prev_puse
	P_((int iDef, int iUse, int iTo, int iTestCount, coveragetype *pCov,
	vartype *var));
static void prev_cuse
	P_((int iDef, int iUse, int iTestCount, coveragetype *pCov, vartype
	*var));
static struct pkPack *load_stampVector
	P_((struct cfile *cf, int iCount));
static struct pkPack *load_coverage
	P_((struct cfile *cf, int iCount));

void
prev_source(pName,rf)
char *pName;
RAMFILE	*rf;
{
	int i;

	i = rf->iFileCount;
	check_file(rf, i);

	if (NULL == (rf->files[i].pName = (char *)malloc(strlen(pName)+1))) {
		memoryError("malloc file name");
	}
	strcpy(rf->files[i].pName,pName);
}

void
prev_header(pPath,iStampCount,stampVector,hdr)
char *pPath;
int iStampCount;
stampstype *stampVector;
headertype *hdr;
{
	int i;

	i = hdr->iHeaderCount;
	check_header(hdr, i);

	if (NULL == (hdr->headers[i].pPath = (char *)malloc(strlen(pPath)+1))) {
		memoryError("malloc header name");
	}

	strcpy(hdr->headers[i].pPath,pPath);
	hdr->headers[i].iStampCount = iStampCount;
	hdr->headers[i].stampVector = stampVector;
	hdr->headers[i].lStampNext = 0;
}

static coveragetype *
load_coverage(cf,iCount)
struct cfile	*cf;
int		iCount;
{
        coveragetype *pCov;
	int	i;

	pCov = (coveragetype *)pk_create();

	for (i = 0; i < iCount; ++i) {
	    /* cf_getLong() returns 0 if no more */
	    pk_append((pkPack *)pCov, (unsigned long)cf_getLong(cf));
	}

	return pCov;
}

static stampstype *
load_stampVector(cf,iCount)
struct cfile	*cf;
int		iCount;
{
        stampstype  *pStamp;
	int	i;

	pStamp = (stampstype *)pk_create();

	for (i = 0; i < iCount; ++i) {
	    /* cf_getLong() returns 0 if no more */
	    pk_append((pkPack *)pStamp, (unsigned long)cf_getLong(cf));
	}

	return pStamp;
}

static void
prev_cuse(iDef,iUse,iTestCount,pCov,var)
int iDef,iUse;
int iTestCount;
coveragetype *pCov;
vartype	*var;
{
	int iCurrent;
	cusetype *cuses;

	iCurrent = var->iCuseCount;
	check_cuse(var, iCurrent);
	cuses = var->cuses;

	cuses[iCurrent].iDefine = iDef;
	cuses[iCurrent].iUse    = iUse;
	cuses[iCurrent].iTestCount = iTestCount;
	cuses[iCurrent].coverage = pCov;
	cuses[iCurrent].iCovNext = 0;
}

static void
prev_puse(iDef,iUse,iTo,iTestCount,pCov,var)
int iDef,iUse,iTo;
int iTestCount;
coveragetype *pCov;
vartype	*var;
{
	int iCurrent;
	pusetype *puses;

	iCurrent = var->iPuseCount;
	check_puse(var, iCurrent);
	puses = var->puses;

	puses[iCurrent].iDefine = iDef;
	puses[iCurrent].iUse    = iUse;
	puses[iCurrent].iTo     = iTo;
	puses[iCurrent].iTestCount = iTestCount;
	puses[iCurrent].coverage = pCov;
	puses[iCurrent].iCovNext = 0;
}

int
prev_member(pDate,pVersion,iFamily,pName,iCost,iFreqFlag,iCorrupted, mems)
char *pDate;
char *pVersion;
int iFamily;
char *pName;
int iCost;
int iFreqFlag;
int iCorrupted;
membertype *mems;
{
	int i;

	i = mems->iMemberCount;
	check_member(mems, i);

	if (NULL==(mems->members[i].pDate = (char *)malloc(strlen(pDate)+1)))
		memoryError("malloc member");

	if (NULL==(mems->members[i].pVersion = (char *)malloc(
	    strlen(pVersion)+1)))
	{
	    memoryError("malloc member");
	}

	if (NULL== (mems->members[i].pName = (char *)malloc(strlen(pName)+1)))
		memoryError("malloc member");

	strcpy(mems->members[i].pDate,pDate);
	strcpy(mems->members[i].pVersion,pVersion);
	strcpy(mems->members[i].pName,pName);
	mems->members[i].iFamily = iFamily;	
	mems->members[i].iCost = iCost;	
	mems->members[i].iFreqFlag = iFreqFlag;	
	mems->members[i].iCorrupted = iCorrupted;	
	mems->members[i].iDelete = 0;	

	return i;
}

static void
prev_block(iBlock, iTestCount, pCov, func)
int iBlock;
int iTestCount;
coveragetype *pCov;
functype	*func;
{
        blocktype *b;

	check_block(func, iBlock);

	b = &func->blocks[iBlock];
	b->coverage = pCov;
	b->iTestCount = iTestCount;
	b->iCovNext = 0;
}

static void
getICompressed(cf, tracefile, mems)
struct cfile	*cf;
char		*tracefile;
membertype	*mems;
{
    int		c;
    int		i;
    int		n;
    char	acDate[50];
    char	acVersion[50];
    char	acName[BUFFER_SIZE];
    char	acFreq[20];
    int		iFreqFlag;
    int		iCorrupted;
    int		iCost;

    c = cf_getFirstChar(cf);

    if (c != 'I') {
	traceError(tracefile, cf_lineNo(cf), 0);
	exit(1);
    }
    n = cf_getLong(cf);
    for (i = 0; i < n; ++i) {
	c = cf_getFirstChar(cf);
	if (c != 'I') {
	    traceError(tracefile, cf_lineNo(cf), 0);
	    exit(1);
	}
	cf_getString(cf, acDate, sizeof acDate);
	cf_getString(cf,acName, sizeof acName);
	cf_getString(cf,acVersion, sizeof acVersion);
	iCost = cf_getLong(cf);
	if (n < 0) {
	    traceError(tracefile, cf_lineNo(cf), 0);
	    exit(1);
	}
	cf_getString(cf,acFreq, sizeof acFreq);
	if (strcmp(acFreq,"frequency") == 0) {
	    iFreqFlag = 1;
	} else {
	    iFreqFlag = 0;
	}
	cf_getString(cf,acFreq, sizeof acFreq);
	if (strcmp(acFreq,"corrupted") == 0) {
	    iCorrupted = 1;
	} else {
	    iCorrupted = 0;
	}
	prev_member(acDate,acVersion,0,acName,iCost,iFreqFlag,iCorrupted, mems);
    }
}

static void
getMCompressed(cf, tracefile, rf)
struct cfile	*cf;
char		*tracefile;
RAMFILE		*rf;
{
    int		c;
    int		i;
    int		n;
    char	acName[BUFFER_SIZE];

    c = cf_getFirstChar(cf);

    if (c != 'M') {
	traceError(tracefile, cf_lineNo(cf), 0);
	exit(1);
    }
    n = cf_getLong(cf);
    for (i = 0; i < n; ++i) {
	c = cf_getFirstChar(cf);
	if (c != 'M') {
	    traceError(tracefile, cf_lineNo(cf), 0);
	    exit(1);
	}
	cf_getString(cf,acName, sizeof acName);
	prev_source(acName, rf);
    }
}

static void
getSCompressed(cf, tracefile, iTestCount, rf)
struct cfile	*cf;
char		*tracefile;
int		iTestCount;
RAMFILE		*rf;
{
    int		c;
    int		i;
    int		n;
    char	acName[BUFFER_SIZE];
    stampstype	*pStampVector;
    int		iModule;

    c = cf_getFirstChar(cf);

    if (c != 'S') {
	traceError(tracefile, cf_lineNo(cf), 0);
	exit(1);
    }
    n = cf_getLong(cf);
    for (i = 0; i < n; ++i) {
	c = cf_getFirstChar(cf);
	if (c != 'S') {
	    traceError(tracefile, cf_lineNo(cf), 0);
	    exit(1);
	}
	iModule = cf_getLong(cf);
	if (iModule >= rf->iFileCount) {
	    traceError(tracefile, cf_lineNo(cf), 0);
	    exit(1);
	}
	cf_getString(cf,acName, sizeof acName);
	pStampVector = load_stampVector(cf, iTestCount);
	prev_header(acName, iTestCount, pStampVector, &rf->files[iModule].hdr);
    }
}

static void
getBCompressed(cf, tracefile, iTestCount, rf)
struct cfile	*cf;
char		*tracefile;
int		iTestCount;
RAMFILE		*rf;
{
    int			c;
    int			i;
    int			n;
    int			iModule;
    int			iFunc;
    int			iBlk;
    coveragetype	*pCov;
    
    c = cf_getFirstChar(cf);

    if (c != 'B') {
	traceError(tracefile, cf_lineNo(cf), 0);
	exit(1);
    }
    n = cf_getLong(cf);
    for (i = 0; i < n; ++i) {
	c = cf_getFirstChar(cf);
	if (c != 'B') {
	    traceError(tracefile, cf_lineNo(cf), 0);
	    exit(1);
	}
	iModule = cf_getLong(cf);
	iFunc = cf_getLong(cf);
	iBlk = cf_getLong(cf);
	pCov = load_coverage(cf, iTestCount);
	if (iModule >= rf->iFileCount) {
	    traceError(tracefile, cf_lineNo(cf), 0);
	    exit(1);
	}
	check_func(&rf->files[iModule], iFunc);
	prev_block(iBlk, iTestCount, pCov, &rf->files[iModule].funcs[iFunc]);
    }
}

static void
getCCompressed(cf, tracefile, iTestCount, rf)
struct cfile	*cf;
char		*tracefile;
int		iTestCount;
RAMFILE		*rf;
{
    int			c;
    int			i;
    int			n;
    int			iModule;
    int			iFunc;
    int			iVar;
    int			iDefine;
    int			iUse;
    coveragetype	*pCov;
    
    c = cf_getFirstChar(cf);

    if (c != 'C') {
	traceError(tracefile, cf_lineNo(cf), 0);
	exit(1);
    }
    n = cf_getLong(cf);
    for (i = 0; i < n; ++i) {
	c = cf_getFirstChar(cf);
	if (c != 'C') {
	    traceError(tracefile, cf_lineNo(cf), 0);
	    exit(1);
	}
	iModule = cf_getLong(cf);
	iFunc = cf_getLong(cf);
	iVar = cf_getLong(cf);
	iDefine = cf_getLong(cf);
	iUse = cf_getLong(cf);
	pCov = load_coverage(cf, iTestCount);
	if (iModule >= rf->iFileCount) {
	    traceError(tracefile, cf_lineNo(cf), 0);
	    exit(1);
	}
	check_func(&rf->files[iModule], iFunc);
	check_var(&rf->files[iModule].funcs[iFunc], iVar);
	prev_cuse(iDefine, iUse, iTestCount, pCov,
		  &rf->files[iModule].funcs[iFunc].vars[iVar]);
    }
}

static void
getPCompressed(cf, tracefile, iTestCount, rf)
struct cfile	*cf;
char		*tracefile;
int		iTestCount;
RAMFILE		*rf;
{
    int			c;
    int			i;
    int			n;
    int			iModule;
    int			iFunc;
    int			iVar;
    int			iDefine;
    int			iUse;
    int			iTo;
    coveragetype	*pCov;
    
    c = cf_getFirstChar(cf);

    if (c != 'P') {
	    traceError(tracefile, cf_lineNo(cf), 0);
	    exit(1);
    }
    n = cf_getLong(cf);
    for (i = 0; i < n; ++i) {
	c = cf_getFirstChar(cf);
	if (c != 'P') {
	    traceError(tracefile, cf_lineNo(cf), 0);
	    exit(1);
	}
	iModule = cf_getLong(cf);
	iFunc = cf_getLong(cf);
	iVar = cf_getLong(cf);
	iDefine = cf_getLong(cf);
	iUse = cf_getLong(cf);
	iTo = cf_getLong(cf);
	pCov = load_coverage(cf, iTestCount);
	if (iModule >= rf->iFileCount) {
	    traceError(tracefile, cf_lineNo(cf), 0);
	    exit(1);
	}
	check_func(&rf->files[iModule], iFunc);
	check_var(&rf->files[iModule].funcs[iFunc], iVar);
	prev_puse(iDefine, iUse, iTo, iTestCount, pCov,
		  &rf->files[iModule].funcs[iFunc].vars[iVar]);
    }
}

static void
getCompressed(cf, tracefile, tables)
struct cfile	*cf;
char		*tracefile;
tablestype	*tables;
{
    int		iTestCount;

    getICompressed(cf, tracefile, &tables->mems);
    iTestCount = tables->mems.iMemberCount;
    getMCompressed(cf, tracefile, &tables->rf);
    getSCompressed(cf, tracefile, iTestCount, &tables->rf);
    getBCompressed(cf, tracefile, iTestCount, &tables->rf);
    getCCompressed(cf, tracefile, iTestCount, &tables->rf);
    getPCompressed(cf, tracefile, iTestCount, &tables->rf);
}

static void
skipCompressed(cf, tracefile, tables)
struct cfile	*cf;
char		*tracefile;
tablestype	*tables;
{
    int i, j, n;

    getICompressed(cf, tracefile, &tables->mems);

    for (j = 0; j < 5; ++j) {	/* for M, S, B, C, P */
	cf_getFirstChar(cf);
	n = cf_getLong(cf);
	for (i = 0; i < n; ++i) {
	    cf_getFirstChar(cf);
	}
    }
}

void
load_prev(tracefile, cf, tables, indexOnly)
char		*tracefile;
struct cfile	*cf;
tablestype	*tables;
int		indexOnly;
{
	int c;
	char acBuffer[50];

	c = cf_getFirstChar(cf);
	if (c == 'V') {
	    cf_getString(cf,acBuffer, sizeof acBuffer);
	    /* ignore VERSION */
	    if (indexOnly) 
		skipCompressed(cf, tracefile, tables);
	    else getCompressed(cf, tracefile, tables);
	    c = cf_getFirstChar(cf);
	}
	
	process_pipe(cf, tracefile, tables, c, indexOnly);
}
