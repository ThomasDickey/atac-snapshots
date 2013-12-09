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
#ifndef ramfile_H
#define ramfile_H
static char const ramfile_h[] = "$Id: ramfile.h,v 3.7 2013/12/09 00:57:02 tom Exp $";
/*
* @Log: ramfile.h,v @
* Revision 3.5  1995/12/27 20:55:44  tom
* adjust headers, prototyped for autoconfig
*
*Revision 3.4  94/04/04  10:26:08  jrh
*Add Release Copyright
*
*Revision 3.3  93/08/10  14:48:23  ewk
*Fixed definition of time_t for vms, MVS, and unix.
*
*Revision 3.2  93/07/09  13:59:07  saul
*Change include for types.h for VMS
*
*Revision 3.1  1993/03/26  11:00:17  saul
*Coverage vector packing.
*
*Revision 3.0  92/11/06  07:47:34  saul
*propagate to version 3.0
*
*Revision 2.3  92/09/08  10:21:42  saul
*Missing #endif.
*
*Revision 2.2  92/09/08  10:12:29  saul
*changed trace format and data structures
*
*Revision 2.1  92/09/08  10:02:50  saul
*Purdue trace management
*
*-----------------------------------------------end of log
*/
/***********************************************
*
*  Programmers 	:	David B. Boardman
*
*  Date		:	April 2, 1992
*  Description	:
*		
*	This file contains the data structures required
*	by the trace manager.
*
**************************************************/

/**
***	Overview statements
***
***	for all blocks, puses, cuses, gcuses, and gpuses
***	there is a coverage list which contains the test
***	cases and frequences.  Each of the mentioned also
***	contains iActive, iTestCount, and iPoolSize.
***	These variable are for management of the coverage list.
***
***	iActive - This value is non-negative
***		if the given b,c,p,P,C, has been encountered by
***		the current test case.  This non negative value
***		is a direct index into the coverage list of the
***		current test case.
***
***	iTestCount - 
***		the number of test cases in the coverage list
***
***	iPoolSize - the size of the current coverage list
***
**/

/**
***	coveragetype contains the test case and it's related
***	frequency
**/

#include "atacysis.h"

typedef struct pkPack coveragetype;
typedef struct pkPack stampstype;

/**
***	cusetype
**/
typedef struct {
    coveragetype *coverage;
    int iCovNext;		/* coverage for test number iTestCount */
    int iTestCount;
    int iDefine;
    int iUse;
} cusetype;

/**
*** pusetype
**/
typedef struct {
    coveragetype *coverage;
    int iCovNext;		/* coverage for test number iTestCount */
    int iTestCount;
    int iDefine;
    int iUse;
    int iTo;
} pusetype;

/**
***	blocktype
**/
typedef struct {
    coveragetype *coverage;
    int iCovNext;		/* coverage for test number iTestCount */
    int iTestCount;
} blocktype;

/**
***	vartype
***
***	For each variable a list of puses and cuses is created
***
**/
typedef struct {
    cusetype *cuses;
    int iCuseCount;
    int iCusePool;

    pusetype *puses;
    int iPuseCount;
    int iPusePool;

} vartype;

/**
***	functype
***
***	For each function there is an array of blocks,
***	global c and puses, and local c and p uses.
**/
typedef struct {
    blocktype *blocks;
    int iBlockCount;
    int iBlockPool;

    vartype *vars;
    int iVarCount;
    int iVarPool;
} functype;

/**
*** 	source & header files
**/

typedef struct {
    char *pPath;
    stampstype *stampVector;
    time_t lStampNext;		/* stamp for test number iTestCount */
    int iStampCount;
} headerstype;

typedef struct {
    headerstype *headers;
    int iHeaderCount;
    int iHeaderPool;
} headertype;

/**
***	filetype
***
***	For each file there is an array of functions
***
**/

typedef struct {
    functype *funcs;
    int iFuncCount;
    int iFuncPool;
    char *pName;
    headertype hdr;
} filetype;

/**
***	RAMFILE - consists of files.
**/
typedef struct {
    filetype *files;
    int iFilePool;
    int iFileCount;
} RAMFILE;

/**
***	family definitions refer to N entry storage
**/

typedef struct {
    char *family;
} familiestype;

typedef struct {
    familiestype *families;
    int iFamilyCount;
    int iFamilyPool;
} familytype;

/**
***	member definitions refer to t entry storage
**/

typedef struct {
    char *pDate;
    int iFamily;
    char *pName;
    char *pVersion;
    int iCost;
    int iFreqFlag;
    int iCorrupted;
    int iDelete;
} memberstype;

typedef struct {
    memberstype *members;
    int iMemberCount;
    int iMemberPool;
} membertype;

typedef struct {
    RAMFILE rf;
    familytype fams;
    membertype mems;
} tablestype;

/* interface of 'dump.c' */
void dump(struct cfile *cf, tablestype * tables);

/* interface of 'init.c' */
extern void init(tablestype * tables);

/* interface of 'mem.c' */

extern void check_block(functype * func, int iBlock);
extern void check_cuse(vartype * var, int iCuse);
extern void check_family(familytype * fam, int iFamily);
extern void check_file(RAMFILE * rf, int iFile);
extern void check_func(filetype * file, int iFunc);
extern void check_header(headertype * hdr, int iHeader);
extern void check_member(membertype * mem, int iMember);
extern void check_puse(vartype * var, int iPuse);
extern void check_var(functype * func, int iVar);

/* interface of 'prev.c' */
extern void load_prev(char *tracefile,
		      struct cfile *cf,
		      tablestype * tables,
		      int indexOnly);
extern int prev_member(const char *pDate,
		       const char *pVersion,
		       int iFamily,
		       const char *pName,
		       int iCost,
		       int iFreqFlag,
		       int iCorrupted,
		       membertype * mems);
extern void prev_header(char *pPath, int iStampCount,
			stampstype * stampVector,
			headertype * hdr);
extern void prev_source(const char *pName, RAMFILE * rf);

/* interface of 'pro.c' */
extern void process_pipe(struct cfile *cf,
			 char *tracefile,
			 tablestype * tables,
			 int initC,
			 int indexOnly);
extern int testNo(membertype * mems, const char *testName);

#endif /* ramfile_H */
