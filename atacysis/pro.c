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

static char pro_c[] = 
	"$Header: /users/source/archives/atac.vcs/atacysis/RCS/pro.c,v 3.3 1994/04/04 10:26:04 jrh Exp $";
/*
*-----------------------------------------------$Log: pro.c,v $
*-----------------------------------------------Revision 3.3  1994/04/04 10:26:04  jrh
*-----------------------------------------------FROM_KEYS
*-----------------------------------------------
*Revision 3.3  94/04/04  10:26:04  jrh
*Add Release Copyright
*
*Revision 3.2  93/08/04  15:57:56  ewk
*Added MVS and solaris support.  Squelched some ANSI warnings.
*
* Revision 3.1  93/03/26  11:02:19  saul
*Coverage vector packing. 
*
*Revision 3.0  92/11/06  07:47:29  saul
*propagate to version 3.0
*
*Revision 2.9  92/11/02  11:43:41  saul
*remove unused variables
*
*Revision 2.8  92/10/30  09:55:29  saul
*include portable.h
*
*Revision 2.7  92/10/29  14:03:20  saul
*atactm may loop on EOF
*
*Revision 2.6  92/10/28  13:49:48  saul
*Error in indexOnly mode when not compressed.
*
*Revision 2.5  92/10/28  09:04:38  saul
*remove strings.h for portability
*
*Revision 2.4  92/09/30  11:57:07  saul
*Optimize -l and -L by skipping coverage info.
*
*Revision 2.3  92/09/22  15:44:46  saul
*Trace compression.
*
*Revision 2.2  92/09/08  10:12:18  saul
*changed trace format and data structures
*
*Revision 2.1  92/09/08  09:59:40  saul
*Purdue trace management
*
*-----------------------------------------------end of log
*/
#include <stdio.h>
#include "portable.h"
#include "ramfile.h"
#include "man.h"

/* forward declarations */
void process_pipe();
int testNo();
static int pro_test();
static void pro_block();
static void pro_cuse();
static void pro_puse();
int pro_source();
static void pro_header();
static void extend_coverageVector();
static void extend_stampVector();
static int map_file();
static int mapSet();
static void mapReset();

extern struct pkPack	*pk_create();
extern void		pk_append();
extern unsigned long	pk_take();

extern void check_func();
extern void check_block();
extern void check_var();
extern void check_cuse();
extern void check_puse();

static int	*sourceMap = NULL;
static int	iMapCount = 0;

static void
mapReset()
{
    int i;

    for (i = 0; i < iMapCount; ++i) {
	sourceMap[i] = -1;
    }
}

static int
mapSet(iFileId, iFile)
int	iFileId;
int	iFile;
{
    int	i;
    int	newSize;

    for (i = 0; i < iMapCount; ++i) {
	if (sourceMap[i] == iFileId) {
	    return 0;			/* failure: already mapped */
	}
    }

    if (iMapCount <= iFile) {
	newSize = iFile + MAP_POOL_SIZE;
	if (iMapCount == 0) {
	    sourceMap = (int *)malloc(newSize * sizeof *sourceMap);
	} else {
	    sourceMap = (int *)realloc(sourceMap, newSize * sizeof *sourceMap);
	}
	if (sourceMap == NULL) {
	    memoryError("realloc map table");
	}

	for (i = iMapCount; i < newSize; ++i) {
	    sourceMap[i] = -1;
	}
	iMapCount = newSize;
    }

    if (sourceMap[iFile] != -1) {
	sourceMap[iFile] = iFileId;
	return 0;			/* failure: map conflict */
    }

    sourceMap[iFile] = iFileId;

    return 1;
}

static int
map_file(iFileId)
int	iFileId;
{
    static int iCache = 0;
    int	i;

    if (iMapCount == 0) {
	return -1;
    }

    if (sourceMap[iCache] == iFileId) {
	return iCache;
    }

    for (i = 0; i < iMapCount; ++i) {
	if (sourceMap[i] == iFileId) break;
    }
    if (i == iMapCount) {
	return -1;
    }

    iCache = i;
    return i;
}

static void
extend_stampVector(stampVector, lStampNext, iStampCount, iTestCase)
stampstype	**stampVector;
time_t	*lStampNext;
int	*iStampCount;
int	iTestCase;
{
    int i;

    if (iTestCase > *iStampCount) {
	if (*iStampCount == 0) {
	    *stampVector = pk_create();
	}
	pk_append(*stampVector, *lStampNext);
	*lStampNext = (time_t)0;
	for (i = *iStampCount + 1; i < iTestCase; ++i) {
	    pk_append(*stampVector, 0);
	}
	*iStampCount = iTestCase;
    }
}

static void
extend_coverageVector(coverageVector, iCovNext, iCoverageCount, iTestCase)
coveragetype	**coverageVector;
int		*iCovNext;
int		*iCoverageCount;
int		iTestCase;
{
    int i;

    if (iTestCase > *iCoverageCount) {
	if (*iCoverageCount == 0) {
	    *coverageVector = pk_create();
	}
	pk_append(*coverageVector, *iCovNext);
	*iCovNext = 0;
	for (i = *iCoverageCount + 1; i < iTestCase; ++i) {
	    pk_append(*coverageVector, 0);
	}
	*iCoverageCount = iTestCase;
    }
}

static void
pro_header(cf, hdr, pPath,iStamp, iTestCase, tracefile, testName, corrupted)
struct cfile	*cf;
char	 *pPath;
time_t	iStamp;
headertype	*hdr;
int	iTestCase;
char	*tracefile;
char	*testName;
int	*corrupted;
{
	int i;

	for (i = 0; i < hdr->iHeaderCount; ++i) {
		if (0 == strcmp(pPath,hdr->headers[i].pPath)) {
		    break;
		}
	}

	if (i == hdr->iHeaderCount) {
	    prev_header(pPath, 0, NULL, hdr);
	}

	extend_stampVector(&hdr->headers[i].stampVector,
			   &hdr->headers[i].lStampNext,
			   &hdr->headers[i].iStampCount,
			   iTestCase);
	if (hdr->headers[i].lStampNext != (time_t)0) {
	    if (!*corrupted) {
		traceError(tracefile, cf_lineNo(cf), testName);
		*corrupted = 1;
	    }
	}
	hdr->headers[i].lStampNext = iStamp;
}

int
pro_source(rf, pName)
RAMFILE *rf;
char *pName;
{
    int i;

    /** check out when the stamp should be changed **/

    for (i = 0; i < rf->iFileCount; ++i) {
	if (strcmp(pName, rf->files[i].pName) == 0) break;
    }

    if (i == rf->iFileCount) {
	prev_source(pName, rf);
    }

    return i;
}

static void
pro_puse(var, iDefine, iUse, iTo, iTestCase, iFreq)
vartype	*var;
int iDefine,iUse,iTo;
int	iTestCase;
int	iFreq;
{
    int		 i;
    pusetype	*puse = NULL;

    for (i = 0; i < var->iPuseCount; ++i) {
	puse = &var->puses[i];
	if ( (puse->iDefine == iDefine) && 
	     (puse->iUse    == iUse) &&
	     (puse->iTo     == iTo )   )
	{
	    break;
	}
    }
    if (i == var->iPuseCount) {
	check_puse(var, i);
	puse = &var->puses[i];	/* var->puses may be moved by check_puse */
	puse->iDefine = iDefine;
	puse->iUse    = iUse;
	puse->iTo     = iTo;
	puse->iTestCount = 0;
	puse->coverage = NULL;
    }
    if (puse->iTestCount <= iTestCase) {
	extend_coverageVector(&puse->coverage, &puse->iCovNext,
			      &puse->iTestCount, iTestCase);
    }
    puse->iCovNext += iFreq;
}

static void
pro_cuse(var, iDefine, iUse, iTestCase, iFreq)
vartype	*var;
int iDefine,iUse;
int	iTestCase;
int	iFreq;
{
    int		 i;
    cusetype	*cuse = NULL;

    for (i = 0; i < var->iCuseCount; ++i) {
	cuse = &var->cuses[i];
	if ( (cuse->iDefine == iDefine) && 
	     (cuse->iUse    == iUse))
	{
	    break;
	}
    }
    if (i == var->iCuseCount) {
	check_cuse(var, i);
	cuse = &var->cuses[i];	/* var->cuses may be moved by check_cuse */
	cuse->iDefine = iDefine;
	cuse->iUse    = iUse;
	cuse->iTestCount = 0;
	cuse->coverage = NULL;
    }
    if (cuse->iTestCount <= iTestCase) {
	extend_coverageVector(&cuse->coverage, &cuse->iCovNext,
			      &cuse->iTestCount, iTestCase);
    }
    cuse->iCovNext += iFreq;
}

static void
pro_block(block, iTestCase, iFreq)
blocktype	*block;
int		iTestCase;
int		iFreq;
{
    if (block->iTestCount <= iTestCase) {
	extend_coverageVector(&block->coverage, &block->iCovNext,
			      &block->iTestCount, iTestCase);
    }
    block->iCovNext += iFreq;
}

static int
pro_test(mems, pDate, pVersion, testName, corrupted)
membertype	*mems;
char		*pDate;
char		*pVersion;
char		*testName;
int		corrupted;
{
    mapReset();
    return prev_member(pDate, pVersion, 0, testName, 100, 0, corrupted, mems);
}

/*
* testNo: Return the next available .n suffix for testName.
*/
int
testNo(mems, testName)
membertype	*mems;
char		*testName;
{
    int		i;
    int		len;
    int		max;
    char	*pName;
    int		n;

    max = 0;
    len = strlen(testName);
    
    for (i = 0; i < mems->iMemberCount; ++i) {
	pName = mems->members[i].pName;
	if (strncmp(pName, testName, len) == 0 && pName[len] == '.') {
	    n = atoi(pName + len + 1);
	    if (n > max) {
		max = n;
	    }
	}
    }

    return max + 1;
}

void
process_pipe(cf, tracefile, tables, initC, indexOnly)
struct cfile	*cf;
char		*tracefile;
tablestype	*tables;
int		initC;
int		indexOnly;
{
    int c;
    int iFile,iFunc,iBlock,iVar,iDef,iUse,iTo;
    time_t lTime;
    char acBuffer[BUFFER_SIZE];
    char acDate[50];
    char acVersion[50];
    char acName[50];
    int	iTestCase;
    int iFileId;
    int iFreq;
    int	corrupted;
    int *pCorrupted = NULL;

    iTestCase = -1;
    iFile = -1;

    c = initC;

    while (c != EOF) {
	switch (c)
	{
 	case 't':
	    corrupted = 0;
	    cf_getString(cf, acDate, sizeof acDate);
	    cf_getString(cf, acVersion, sizeof acVersion);
	    cf_getString(cf, acName, sizeof acName);
	    if (acName[0] == '\0') {
		strcpy(acName, "T");
	    }
	    sprintf(acName + strlen(acName), ".%d",
		    testNo(&tables->mems, acName));
	    if (acDate[0] == '\0') {
		traceError(tracefile, cf_lineNo(cf), acName);
		strcpy(acDate, "?");
		corrupted = 1;
	    }
	    else if (acVersion[0] == '\0') {
		    strcpy(acVersion, "old");
	    }
	    iTestCase = pro_test(&tables->mems, acDate, acVersion, acName,
				 corrupted);
	    pCorrupted = &tables->mems.members[iTestCase].iCorrupted;
	    iFile = -1;
	    break;
	case 'f':
	    if (iTestCase == -1) {
		sprintf(acName, "E.%d", testNo(&tables->mems, "E"));
		traceError(tracefile, cf_lineNo(cf), acName);
		iTestCase = pro_test(&tables->mems, "unknown", "unknown",
				     acName, 1);
		pCorrupted = &tables->mems.members[iTestCase].iCorrupted;
		iFile = -1;
	    }
	    tables->mems.members[iTestCase].iFreqFlag = 1;
	    break;
	case 's':
	    if (iTestCase == -1) {
		sprintf(acName, "E.%d", testNo(&tables->mems, "E"));
		traceError(tracefile, cf_lineNo(cf), acName);
		iTestCase = pro_test(&tables->mems, "unknown", "unknown",
				     acName, 1);
		pCorrupted = &tables->mems.members[iTestCase].iCorrupted;
		iFile = -1;
	    }
	    if (indexOnly) break;
	    iFileId = cf_getLong(cf);
	    cf_getString(cf, acBuffer, sizeof acBuffer);
	    lTime = (time_t)cf_getLong(cf);

	    iFile = pro_source(&tables->rf, acBuffer);
	    if (mapSet(iFileId, iFile) == 0) {
		if (!*pCorrupted) {
		    traceError(tracefile, cf_lineNo(cf), acName);
		    *pCorrupted = 1;
		}
	    }
	    pro_header(cf, &tables->rf.files[iFile].hdr, acBuffer, lTime,
		       iTestCase, tracefile, acName, pCorrupted);
	    break;
	case 'h':
	    if (iTestCase == -1) {
		sprintf(acName, "E.%d", testNo(&tables->mems, "E"));
		traceError(tracefile, cf_lineNo(cf), acName);
		iTestCase = pro_test(&tables->mems, "unknown","unknown", 
				     acName, 1);
		iFile = pro_source(&tables->rf, "=dummy=");
		pCorrupted = &tables->mems.members[iTestCase].iCorrupted;
	    } 
	    if (indexOnly) break;
	    if (iFile == -1) {
		if (!*pCorrupted) {
		    traceError(tracefile, cf_lineNo(cf), acName);
		    *pCorrupted = 1;
		}
		iFile = pro_source(&tables->rf, "=dummy=");
	    }
	    cf_getString(cf, acBuffer, sizeof acBuffer);
	    lTime = (time_t)cf_getLong(cf);
	    pro_header(cf, &tables->rf.files[iFile].hdr, acBuffer,lTime,
		       iTestCase, tracefile, acName, pCorrupted);
	    break;
	case 'b':
	    if (iTestCase == -1) {
		sprintf(acName, "E.%d", testNo(&tables->mems, "E"));
		traceError(tracefile, cf_lineNo(cf), acName);
		iTestCase = pro_test(&tables->mems, "unknown", "unknown",
				     acName, 1);
		pCorrupted = &tables->mems.members[iTestCase].iCorrupted;
		iFile = -1;
	    }
	    if (indexOnly) break;
	    iFile = map_file(cf_getLong(cf));
	    if (iFile == -1) {
		if (!*pCorrupted) {
		    traceError(tracefile, cf_lineNo(cf), acName);
		    *pCorrupted = 1;
		}
		break;
	    }
	    iFunc = cf_getLong(cf);
	    iBlock= cf_getLong(cf);
	    iFreq = cf_getLong(cf);
	    if (iFreq == 0) iFreq = 1;
				
	    check_func(&tables->rf.files[iFile], iFunc);
	    check_block(&tables->rf.files[iFile].funcs[iFunc], iBlock);
	    pro_block(&tables->rf.files[iFile].funcs[iFunc].blocks[iBlock],
		      iTestCase, iFreq);
	    break;
	case 'c':
	    if (iTestCase == -1) {
		sprintf(acName, "E.%d", testNo(&tables->mems, "E"));
		traceError(tracefile, cf_lineNo(cf), acName);
		iTestCase = pro_test(&tables->mems, "unknown", "unknown",
				     acName, 1);
		pCorrupted = &tables->mems.members[iTestCase].iCorrupted;
		iFile = -1;
	    }
	    if (indexOnly) break;
	    iFile = map_file(cf_getLong(cf));
	    if (iFile == -1) {
		if (!*pCorrupted) {
		    traceError(tracefile, cf_lineNo(cf), acName);
		    *pCorrupted = 1;
		}
		break;
	    }
	    iFunc = cf_getLong(cf);
	    iVar  = cf_getLong(cf);
	    iDef  = cf_getLong(cf);
	    iUse  = cf_getLong(cf);
	    iFreq = cf_getLong(cf);
	    if (iFreq == 0) iFreq = 1;

	    check_func(&tables->rf.files[iFile], iFunc);
	    check_var(&tables->rf.files[iFile].funcs[iFunc], iVar);
	    pro_cuse(&tables->rf.files[iFile].funcs[iFunc].vars[iVar],
		      iDef, iUse, iTestCase, iFreq);
	    break;
	case 'p':
	    if (iTestCase == -1) {
		sprintf(acName, "E.%d", testNo(&tables->mems, "E"));
		traceError(tracefile, cf_lineNo(cf), acName);
		iTestCase = pro_test(&tables->mems, "unknown", "unknown",
				     acName, 1);
		pCorrupted = &tables->mems.members[iTestCase].iCorrupted;
		iFile = -1;
	    }
	    if (indexOnly) break;
	    iFile = map_file(cf_getLong(cf));
	    if (iFile == -1) {
		if (!*pCorrupted) {
		    traceError(tracefile, cf_lineNo(cf), acName);
		    *pCorrupted = 1;
		}
		break;
	    }
	    iFunc = cf_getLong(cf);
	    iVar  = cf_getLong(cf);
	    iDef  = cf_getLong(cf);
	    iUse  = cf_getLong(cf);
	    iTo   = cf_getLong(cf);
	    iFreq = cf_getLong(cf);
	    if (iFreq == 0) iFreq = 1;

	    check_func(&tables->rf.files[iFile], iFunc);
	    check_var(&tables->rf.files[iFile].funcs[iFunc], iVar);
	    pro_puse(&tables->rf.files[iFile].funcs[iFunc].vars[iVar],
		      iDef, iUse, iTo, iTestCase, iFreq);
	    break;
	default :
	    if (iTestCase == -1) {
		sprintf(acName, "E.%d", testNo(&tables->mems, "E"));
		traceError(tracefile, cf_lineNo(cf), acName);
		iTestCase = pro_test(&tables->mems, "unknown", "unknown",
				     acName, 1);
		pCorrupted = &tables->mems.members[iTestCase].iCorrupted;
		iFile = -1;
	    }
	    else if (!*pCorrupted) {
		traceError(tracefile, cf_lineNo(cf), acName);
		*pCorrupted = 1;
	    }
	    break;
	}

	c = cf_getFirstChar(cf);
    }
}
