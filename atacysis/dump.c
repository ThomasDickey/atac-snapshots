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
#if HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef MVS
#include <mvapts.h>
MODULEID(%M%,%J%/%D%/%T%)
#endif /* MVS */

#include <stdio.h>

#include "portable.h"
#include "atacysis.h"
#include "pack.h"
#include "ramfile.h"
#include "man.h"
#include "version.h"

static char const dump_c[] = 
	"$Header: /users/source/archives/atac.vcs/atacysis/RCS/dump.c,v 3.8 1996/11/13 01:31:56 tom Exp $";
/*
* $Log: dump.c,v $
* Revision 3.8  1996/11/13 01:31:56  tom
* include <config.h> to declare 'const'
*
* Revision 3.7  1995/12/27 20:46:29  tom
* adjust headers, prototyped for autoconfig
* correct gcc warnings (casts)
*
* Revision 3.6  94/08/08  13:50:41  saul
* atactm -d and -e bug (problem with fix in revision 3.5)
*
* Revision 3.5  94/08/03  10:02:20  saul
* atactm -d and -e bug on non compressed trace fixed
*
* Revision 3.4  94/04/04  10:25:09  jrh
* Add Release Copyright
*
* Revision 3.3  93/08/04  15:53:38  ewk
* Added MVS and solaris support.  Squelched some ANSI warnings.
*
* Revision 3.2  93/03/29  11:38:04  saul
* Don't free coverage/timestamp vectors that have never been created.
*
* Revision 3.1  93/03/26  11:13:14  saul
* Coverage vector packing. 
*
* Revision 3.0  92/11/06  07:47:06  saul
* propagate to version 3.0
*
* Revision 2.5  92/10/30  09:54:08  saul
* include portable.h
*
* Revision 2.4  92/10/01  13:28:32  saul
* Remove debuging code.
*
* Revision 2.3  92/09/22  15:40:02  saul
* Trace compression.
*
* Revision 2.2  92/09/08  10:12:21  saul
* changed trace format and data structures
*
* Revision 2.1  92/09/08  09:59:50  saul
* Purdue trace management
*
*-----------------------------------------------end of log
*/

/* forward declarations */
static void dump_blocks
	P_((struct cfile *cf, RAMFILE *rf, int iTestCases, memberstype
	*members));
static void dump_members
	P_((struct cfile *cf, membertype *mems));
static void dump_cuses
	P_((struct cfile *cf, RAMFILE *rf, int iTestCases, memberstype
	*members));
static void dump_puses
	P_((struct cfile *cf, RAMFILE *rf, int iTestCases, memberstype
	*members));
static void dump_headers
	P_((struct cfile *cf, RAMFILE *rf, int iTestCases, memberstype
	*members));
static void dump_sources
	P_((struct cfile *cf, RAMFILE *rf));
static void dump_version
	P_((struct cfile *cf));
static void dump_coverage
	P_((struct cfile *cf, coveragetype *pCov, int iCovNext, int iCount, int
	iTestCases, memberstype *members));
static void dump_stamps
	P_((struct cfile *cf, stampstype *pStamps, time_t lStampNext, int
	iCount, int iTestCases, memberstype *members));

static void
dump_stamps(cf, pStamps, lStampNext, iCount, iTestCases, members)
struct cfile	*cf;
stampstype	*pStamps;
time_t		lStampNext;
int		iCount;
int		iTestCases;
memberstype	*members;
{
	int i;
	int	iTestCount;
	time_t	n;

	iTestCount = iCount;
	if (iTestCases < iTestCount) {
	    iTestCount = iTestCases;
	}
	for (i = 0; i < iTestCount; ++i) {
	    n = (time_t)pk_take((pkPack *)pStamps);
	    if (members[i].iDelete) continue;
	    cf_putLong(cf, n);
	}
	if (i < iTestCases) {
	    if (members[i].iDelete == 0)
		cf_putLong(cf, lStampNext);
	    ++i;
	}
	for (; i < iTestCases; ++i) {
	    if (members[i].iDelete) continue;
	    cf_putLong(cf, 0);
	}
	cf_putNewline(cf);
}

static void
dump_coverage(cf, pCov, iCovNext, iCount, iTestCases, members)
struct cfile	*cf;
coveragetype	*pCov;
int		iCovNext;
int		iCount;
int		iTestCases;
memberstype	*members;
{
	int i;
	int	n;
	int	iTestCount;

	iTestCount = iCount;
	if (iTestCases < iTestCount) {
	    iTestCount = iTestCases;
	}
	for (i = 0; i < iTestCount; ++i) {
	    n = pk_take((pkPack *)pCov);
	    if (members[i].iDelete) continue;
	    cf_putLong(cf, n);
	}
	if (i < iTestCases) {
	    if (members[i].iDelete == 0)
		cf_putLong(cf, iCovNext);
	    ++i;
	}
	for (; i < iTestCases; ++i) {
	    if (members[i].iDelete) continue;
	    cf_putLong(cf, 0);
	}
	cf_putNewline(cf);
}

static void
dump_version(cf)
struct cfile	*cf;
{
    cf_putFirstChar(cf, 'V');
    cf_putString(cf, VERSION);
    cf_putNewline(cf);
}

static void
dump_sources(cf, rf)
struct cfile	*cf;
RAMFILE		*rf;
{
	int i;

	cf_putFirstChar(cf, 'M');
	cf_putLong(cf, rf->iFileCount);
	cf_putNewline(cf);
	for (i = 0; i < rf->iFileCount; ++i) {
	    cf_putFirstChar(cf, 'M');
	    cf_putString(cf, rf->files[i].pName);
	    cf_putNewline(cf);
	}
}

static void
dump_headers(cf, rf, iTestCases, members)
struct cfile	*cf;
RAMFILE	 	*rf;
int		iTestCases;
memberstype	*members;
{
	int i;
	int j;
	int n;

	n = 0;
	for (i = 0; i < rf->iFileCount; ++i) {
	    n += rf->files[i].hdr.iHeaderCount;
	}
	cf_putFirstChar(cf, 'S');
	cf_putLong(cf, n);
	cf_putNewline(cf);

	for (i = 0; i < rf->iFileCount; ++i) {
	    for (j = 0; j < rf->files[i].hdr.iHeaderCount; ++j) {
		cf_putFirstChar(cf, 'S');
		cf_putLong(cf, i);
		cf_putString(cf, rf->files[i].hdr.headers[j].pPath);
		dump_stamps(cf, rf->files[i].hdr.headers[j].stampVector,
			    rf->files[i].hdr.headers[j].lStampNext,
			    rf->files[i].hdr.headers[j].iStampCount,
			    iTestCases, members);
		if (rf->files[i].hdr.headers[j].stampVector != NULL) {
		    /* clean up */
		    pk_free((pkPack *)(rf->files[i].hdr.headers[j].stampVector));
		    rf->files[i].hdr.headers[j].stampVector = NULL;
		}
	    }
	}
}

static void
dump_puses(cf, rf, iTestCases, members)
struct cfile	*cf;
RAMFILE		*rf;
int		iTestCases;
memberstype	*members;
{
	int	i;
	int	j;
	int	k;
	int	m;
	int	n;
	vartype *var;

	n = 0;
	for (i = 0; i < rf->iFileCount; ++i) {
	    for (j = 0; j < rf->files[i].iFuncCount; ++j) {
		for (k = 0; k < rf->files[i].funcs[j].iVarCount; ++k) {
		    n += rf->files[i].funcs[j].vars[k].iPuseCount;
		}
	    }
	}
	cf_putFirstChar(cf, 'P');
	cf_putLong(cf, n);
	cf_putNewline(cf);

	for (i = 0; i < rf->iFileCount; ++i) {
	    for (j = 0; j < rf->files[i].iFuncCount; ++j) {
		for (k = 0; k < rf->files[i].funcs[j].iVarCount; ++k) {
		    var = &rf->files[i].funcs[j].vars[k];
		    for (m = 0; m < var->iPuseCount; ++m) {
			cf_putFirstChar(cf, 'P');
			cf_putLong(cf, i);
			cf_putLong(cf, j);
			cf_putLong(cf, k);
			cf_putLong(cf, var->puses[m].iDefine);
			cf_putLong(cf, var->puses[m].iUse);
			cf_putLong(cf, var->puses[m].iTo);
			dump_coverage(cf, var->puses[m].coverage,
				      var->puses[m].iCovNext,
				      var->puses[m].iTestCount, iTestCases,
				      members);
			if (var->puses[m].coverage != NULL) {
			    /* clean up */
			    pk_free((pkPack *)(var->puses[m].coverage));
			    var->puses[m].coverage = NULL;
			}
		    }
		}
	    }
	}
}

static void
dump_cuses(cf, rf, iTestCases, members)
struct cfile	*cf;
RAMFILE		*rf;
int		iTestCases;
memberstype	*members;
{
	int	i;
	int	j;
	int	k;
	int	m;
	int	n;
	vartype *var;

	n = 0;
	for (i = 0; i < rf->iFileCount; ++i) {
	    for (j = 0; j < rf->files[i].iFuncCount; ++j) {
		for (k = 0; k < rf->files[i].funcs[j].iVarCount; ++k) {
		    n += rf->files[i].funcs[j].vars[k].iCuseCount;
		}
	    }
	}
	cf_putFirstChar(cf, 'C');
	cf_putLong(cf, n);
	cf_putNewline(cf);

	for (i = 0; i < rf->iFileCount; ++i) {
	    for (j = 0; j < rf->files[i].iFuncCount; ++j) {
		for (k = 0; k < rf->files[i].funcs[j].iVarCount; ++k) {
		    var = &rf->files[i].funcs[j].vars[k];
		    for (m = 0; m < var->iCuseCount; ++m) {
			cf_putFirstChar(cf, 'C');
			cf_putLong(cf, i);
			cf_putLong(cf, j);
			cf_putLong(cf, k);
			cf_putLong(cf, var->cuses[m].iDefine);
			cf_putLong(cf, var->cuses[m].iUse);
			dump_coverage(cf, var->cuses[m].coverage,
				      var->cuses[m].iCovNext,
				      var->cuses[m].iTestCount, iTestCases,
				      members);
			if (var->cuses[m].coverage != NULL) {
			    /* clean up */
			    pk_free((pkPack *)(var->cuses[m].coverage));
			    var->cuses[m].coverage = NULL;
			}
		    }
		}
	    }
	}
}

static void
dump_members(cf, mems)
struct cfile	*cf;
membertype	*mems;
{
	int i;
	int n;

	n = 0;
	for (i = 0; i < mems->iMemberCount; ++i) {
	    if (mems->members[i].iDelete == 0) {
		++n;
	    }
	}

	cf_putFirstChar(cf, 'I');
	cf_putLong(cf, n);
	cf_putNewline(cf);
	for (i = 0; i < mems->iMemberCount; ++i) {
	    if (mems->members[i].iDelete) continue;
	    cf_putFirstChar(cf, 'I');
	    cf_putString(cf, mems->members[i].pDate);
	    cf_putString(cf, mems->members[i].pName);
	    cf_putString(cf, mems->members[i].pVersion);
	    cf_putLong(cf, mems->members[i].iCost);
	    if (mems->members[i].iFreqFlag) {
		cf_putString(cf, "frequency");
	    } else {
		cf_putString(cf, "nofrequency");
	    }
	    if (mems->members[i].iCorrupted) {
		cf_putString(cf, "corrupted");
	    } else {
		cf_putString(cf, "okay");
	    }
	    cf_putNewline(cf);
	}
}

static void
dump_blocks(cf, rf, iTestCases, members)
struct cfile	*cf;
RAMFILE		*rf;
int		iTestCases;
memberstype	*members;
{
	int	i;
	int	j;
	int	k;
	int	n;

	n = 0;
	for (i = 0; i < rf->iFileCount; ++i) {
	    for (j = 0; j < rf->files[i].iFuncCount; ++j) {
		n += rf->files[i].funcs[j].iBlockCount;
	    }
	}
	cf_putFirstChar(cf, 'B');
	cf_putLong(cf, n);
	cf_putNewline(cf);

	for (i = 0; i < rf->iFileCount; ++i) {
	    for (j = 0; j < rf->files[i].iFuncCount; ++j) {
		for (k = 0; k < rf->files[i].funcs[j].iBlockCount; ++k) {
		    cf_putFirstChar(cf, 'B');
		    cf_putLong(cf, i);
		    cf_putLong(cf, j);
		    cf_putLong(cf, k);
		    dump_coverage(cf, rf->files[i].funcs[j].blocks[k].coverage,
				  rf->files[i].funcs[j].blocks[k].iCovNext,
				  rf->files[i].funcs[j].blocks[k].iTestCount,
				  iTestCases, members);
		    if (rf->files[i].funcs[j].blocks[k].coverage != NULL) {
			/* clean up*/
			pk_free((pkPack *)(rf->files[i].funcs[j].blocks[k].coverage));
			rf->files[i].funcs[j].blocks[k].coverage = NULL;
		    }
		}
	    }
	}
}

void
dump(cf, tables)
struct cfile	*cf;
tablestype	*tables;
{
	dump_version(cf);
	dump_members(cf, &tables->mems);
	dump_sources(cf, &tables->rf);
	dump_headers(cf, &tables->rf, tables->mems.iMemberCount,
		     tables->mems.members);
	dump_blocks(cf, &tables->rf, tables->mems.iMemberCount,
		     tables->mems.members);
	dump_cuses(cf, &tables->rf,  tables->mems.iMemberCount,
		     tables->mems.members);
	dump_puses(cf, &tables->rf,  tables->mems.iMemberCount,
		     tables->mems.members);
}
