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
#ifndef atacysis_H
#define atacysis_H
static char atacysis_h[] = "$Header: /users/source/archives/atac.vcs/atacysis/RCS/atacysis.h,v 3.7 1994/04/04 10:24:48 jrh Exp $";
/*
*-----------------------------------------------$Log: atacysis.h,v $
*-----------------------------------------------Revision 3.7  1994/04/04 10:24:48  jrh
*-----------------------------------------------FROM_KEYS
*-----------------------------------------------
* Revision 3.7  94/04/04  10:24:48  jrh
* Add Release Copyright
* 
* Revision 3.6  93/08/23  15:37:21  ewk
* Changed type of "cost" fields from "int" to "long".
* 
* Revision 3.5  93/08/10  14:47:15  ewk
* Fixed definition of time_t for vms, MVS, and unix.
* 
* Revision 3.4  93/07/09  15:00:39  saul
* syntax error
* 
* Revision 3.3  93/07/09  12:19:24  saul
* change include for types.h for VMS
* 
* Revision 3.2  93/03/26  11:13:51  saul
* T_HEADER structure for binary .atac files.  Changed ints to shorts.
*
* Revision 3.1  92/12/03  08:45:50  saul
* CUMULATIVE and COST options.
* 
* Revision 3.0  92/11/06  07:48:03  saul
* propagate to version 3.0
* 
* Revision 2.7  92/10/30  09:53:50  saul
* include portable.h
* 
* Revision 2.6  92/10/08  10:08:25  saul
*  change file time stamp checking to work with compression
* 
* Revision 2.5  92/09/30  11:16:17  saul
* Add no header option.  Add cost field for atacMin.
* 
* Revision 2.4  92/09/08  09:15:08  saul
* New options.  New coverage vector data structure.  Test names.  Tabular  disp.
* 
* Revision 2.3  92/07/10  11:18:53  saul
* new POSITION struct;detected infeasable count fields;obsolete T_DECIS
* 
* Revision 2.2  92/03/17  15:26:58  saul
* copyright
* 
* Revision 2.1  91/06/19  13:09:54  saul
* Propagate to version 2.0
* 
* Revision 1.1  91/06/12  16:58:13  saul
* Aug 1990 baseline
* 
*-----------------------------------------------end of log
*/
#ifdef vms
#include <types.h>
#else /* not vms */
#ifdef MVS
#include <time.h>		/* for time_t */
#else /* not MVS */
#include <sys/types.h>		/* for time_t */
#endif /* not MVS */
#endif /* not vms */
#include "version.h"

#define DISPLAY_ALL	0x01	/* Display all uncovered constructs. */
				/* (Even those in uncovered blocks.) */
#define DISPLAY_COV	0x02	/* Highlight covered constructs. */
				/* Rather than uncovered constructs. */

#define OPTION_IGNORE_SRC_TIMESTAMP	0x0001
#define OPTION_BLOCK			0x0002
#define OPTION_DECIS			0x0004
#define OPTION_CUSE			0x0008
#define OPTION_PUSE			0x0010
#define OPTION_BY_TEST			0x0020
#define OPTION_DESELECT			0x0040
#define OPTION_FREQ			0x0080
#define OPTION_ALLUSE			0x0100
#define OPTION_F_ENTRY 			0x0200
#define OPTION_COUNTER_ATAC		0x0400
#define OPTION_NO_HEADER		0x0800
#define OPTION_CUMULATIVE		0x1000
#define OPTION_COST			0x2000

#define VALUE_SIZE	16

#define TIMESTAMP_SIZE (sizeof "1999/12/31-23:59:59")
#define TESTNAME_SIZE	50

typedef struct {
    short		file;
    short		col;
    unsigned short	line;
} POSITION;

typedef struct {
    POSITION		start;
    POSITION		end;
} SE_POSITION;

typedef struct {
	SE_POSITION	def;
	SE_POSITION	use;
	unsigned short	varno;
	unsigned short	blk1;
	unsigned short	blk2;
} T_CUSE;

typedef struct {
	SE_POSITION	def;
	SE_POSITION	use;
	unsigned short	varno;
	unsigned short	blk1;
	unsigned short	blk2;
	unsigned short	blk3;
	char		value[VALUE_SIZE];
} T_PUSE;

typedef struct {
	char		*vname;
	unsigned short	cstart;
	unsigned short	pstart;
} T_VAR;

typedef struct {
	SE_POSITION	pos;
} T_BLK;

typedef struct {
	SE_POSITION	pos;
	char		*fname;
	T_BLK		*blk;
	T_VAR		*var;
	T_CUSE		*cuse;
	T_PUSE		*puse;
	int		blkCovStart;
	int		cUseCovStart;
	int		pUseCovStart;
	short		decis_var;
	unsigned short	n_blk;
	unsigned short	n_var;
	unsigned short	n_cuse;
	unsigned short	formalN_cuse;
	unsigned short	n_puse;
	unsigned short	formalN_puse;
	short		ignore;		/* not interested in this function */
} T_FUNC;

typedef struct {
	char	*filename;
	time_t	chgtime;
} T_FILE;

#define DOT_ATAC_HEADING "A ATAC (.atac) file"

typedef struct {
    char		heading[sizeof DOT_ATAC_HEADING];
    char		version[sizeof VERSION];
    unsigned short	nFiles;
    unsigned short	nFuncs;
    unsigned int	fileOffset;
    unsigned int	funcOffset;
} T_HEADER;	/* Header for converted files. */

typedef struct {
        T_HEADER	*header;
	char		*atacfile;
	T_FILE		*file;
	T_FUNC		*func;
	unsigned short	n_file;
	unsigned short	n_func;
	short		ignore;		/* all in module func ignored */
} T_MODULE;

typedef struct {
    long	cost;
    int		freq;		/* Frequency counts present. */
    char	timeStamp[TIMESTAMP_SIZE];
    char	name[TESTNAME_SIZE];
} T_TEST;
    
typedef struct {
    int		*cov;
    long	cost;
    char	name[TESTNAME_SIZE];
} T_TESTLIST;

/** Shirley @ Purdue **/
#define DO_U_CUSE       1       /* for the hili display of undefined cuses */
#define DO_U_PUSE       2       /* for the hili display of undefined puses */
#define TD_NONE		0	/* no tabular display selected */
#define TD_BLOCK	1	/* tabular display of blocks */
#define TD_PUSE		2	/* tabular display of p-uses */
#define TD_CUSE		4	/* tabular display of c-uses */
#define TD_DECIS	8	/* tabular display of decisions */
#define TD_UNDEF	16	/* tabular display of undefined uses */
#define TD_NEITHER	0	/* neither decision branch uncovered */
#define TD_TRUE		1	/* true decision branch uncovered */
#define TD_FALSE	2	/* false decision branch uncovered */
#define TD_BOTH		3	/* both decision branches uncovered */ 
#define TD_CHECKED     -5       /* mark a =decis= p-use as checked */

#endif /* atacysis_H */
