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

static char trace_c[] =
	"$Header: /users/source/archives/atac.vcs/atacysis/RCS/trace.c,v 3.11 1994/08/03 12:34:27 saul Exp $";
/*
*-----------------------------------------------$Log: trace.c,v $
*-----------------------------------------------Revision 3.11  1994/08/03 12:34:27  saul
*-----------------------------------------------FROM_KEYS
*-----------------------------------------------
* Revision 3.11  94/08/03  12:34:27  saul
* Error introduced in revision 3.9 gives coverage too high.
* 
* Revision 3.10  94/07/22  11:17:29  saul
* atac -i fails when .h file modified.  Introduced in trace.c 2.13.
* 
* Revision 3.9  94/07/11  14:23:46  saul
* Don't let count wrap around -- partial solution.
* 
* Revision 3.8  94/04/04  10:50:46  jrh
* Add Release Copyright
* 
* Revision 3.7  93/11/02  11:36:34  saul
* Delete changes made by revision 3.5 (signed/unsigned) problem.
* 
* Revision 3.6  93/10/13  17:44:13  ewk
* selectList = NULL when no trace file or matching test cases
* 
* Revision 3.5  93/08/23  15:40:58  ewk
* Eliminated many casts for Solaris warnings by modifying type decls.
* 
* Revision 3.4  93/08/04  15:59:18  ewk
* Added MVS and solaris support.  Squelched some ANSI warnings.
* 
* Revision 3.3  93/04/29  07:22:38  saul
* Fix error when atac -n ... used with uncompressed trace.
* 
* Revision 3.2  93/02/09  14:00:24  saul
* Accept empty trace file when not in per trace mode.
* 
* Revision 3.1  92/12/03  08:51:42  saul
* Patch memory leak.
* 
* Revision 3.0  92/11/06  07:47:44  saul
* propagate to version 3.0
* 
* Revision 2.16  92/11/06  07:12:56  saul
* initialize "throw away" memory to keep purify happy
* 
* Revision 2.15  92/11/02  11:43:58  saul
* remove unused variables
* 
* Revision 2.14  92/10/30  09:55:51  saul
* include portable.h
* 
* Revision 2.13  92/10/28  15:56:32  saul
* atac -s -p missing test names for uncompressed tests
* 
* Revision 2.12  92/10/28  11:26:24  saul
* fix free(0) problem which coredumps on some systems
* 
* Revision 2.11  92/10/28  09:05:00  saul
* #include sys/types.h for portability
* 
* Revision 2.10  92/10/21  16:06:05  saul
* coredump when tracefile is empty.
* 
* Revision 2.9  92/10/08  10:08:55  saul
* change file time stamp checking to work with compression.  Selected only.
* 
* Revision 2.8  92/10/05  10:39:38  saul
* flat file reading erros due to ungetchar
* 
* Revision 2.7  92/09/30  11:19:36  saul
* Add cost field for atacMin
* 
* Revision 2.6  92/09/22  15:45:09  saul
* Trace compression.
* Fix ignoring module 0 error.
* 
* Revision 2.5  92/09/08  09:21:14  saul
* New trace file format.
* 
* Revision 2.4  92/07/10  10:47:43  saul
* remove obsolete T_DECIS references
* 
* Revision 2.3  92/03/17  15:27:17  saul
* copyright
* 
* Revision 2.2  91/08/30  13:47:48  saul
* Fix invalid test handling.
* 
* Revision 2.1  91/06/19  13:10:12  saul
* Propagate to version 2.0
* 
* Revision 1.1  91/06/12  16:58:39  saul
* Aug 1990 baseline
* 
*-----------------------------------------------end of log
*/
#include <stdio.h>
#include <ctype.h>
#include "portable.h"
#include "atacysis.h"

/* forward declarations */
void trace_data();
static int testNo();
static void getCompressed();
static void getPuseCov();
static void getCuseCov();
static void getBlkCov();
static void getSkip();
static void checkSrcStamp();
static void getMIndex();
static void getIndex();
static int skipFlatTest();
static int getFlatTest();

#define CHECK_MALLOC(p) if((p)==NULL)fprintf(stderr,"Out of memory\n"),exit(1)

#define MAX_COUNT LURSHIFT(~0,1)	/* 2^31 - 1 */
#define STREQ(s1,s2)	(strcmp((s1),(s2)) == 0)

#define FILENAME_SIZE 1024

typedef struct {
	int		file;
	T_MODULE	*module;
} M_INDEX;

/*
* getFlatTests: Read in an uncompressed trace and update cov vector.
* 	stop reading at EOF or t or f entry.  
*/
static int				/* Return next input character. */
getFlatTest(cf, filename, t_module, n_module, testName, cov, options, freq)
struct cfile	*cf;
char		*filename;
T_MODULE	*t_module;
int		n_module;
char		*testName;
int		*cov;
int		options;
int		*freq;
{
    M_INDEX	*m_index;	/* Maps file_id into module info. */
    int		n_m_index = 0;
    int		file;		/* File_id most recently used. */
    T_MODULE	*mod;		/* Module info for file_id: file */
    int		tfile;		/* Next file_id */
    int		funcNo;		/* function number input field */ 
    int		varNo;		/* var input field */
    int		b1;		/* block 1 input field */
    int		b2;		/* block 2 input field */
    int		b3;		/* block 3 input field */
    T_FUNC	*func;
    T_CUSE	*cuse;
    T_PUSE	*puse;
    int		i;
    int		c;
    char	srcfilename[FILENAME_SIZE];
    time_t	chgtime;

    *freq = 0;	/* Incomplete (or no) frequency counts so far. */

    m_index = (M_INDEX *)malloc(n_module * sizeof *m_index);
    CHECK_MALLOC(m_index);

    n_m_index = 0;
    file = -1;
    mod = NULL;

    while ((c = cf_getFirstChar(cf)) != EOF) {
	if (c == 't') return c;
	if (c == 'h') {
	    cf_getString(cf, srcfilename, sizeof srcfilename);
	    chgtime = cf_getLong(cf);
	    if (mod == NULL) continue;	/* Static info. missing. */
	    for (i = 0; i < (int)mod->n_file; ++i) {
		if (STREQ(mod->file[i].filename, srcfilename)) break;
	    }
	    if (i == mod->n_file || mod->file[i].chgtime != chgtime) {
		if (options & OPTION_IGNORE_SRC_TIMESTAMP) continue;
		fprintf(stderr,
			"Test %s.%s invalid due to modification of \"%s\"\n",
			filename, testName, srcfilename);
		exit(1);
	    }
	    continue;
	}
	tfile = cf_getLong(cf);
	if (tfile != file) {
	    if (tfile < n_m_index && tfile == m_index[tfile].file) {
		mod = m_index[tfile].module;
	    }
	    else {
		for (i = 0; i < n_m_index; ++i) {
		    if (tfile == m_index[i].file)
			break;
		}
		if (i < n_m_index) {
		    mod = m_index[i].module;
		} else {
		    mod = NULL;
		}
	    }
	    file = tfile;
	}
	switch (c)
	{
        case 'f':
	    *freq = 1;	/* Complete frequency counts */
	    break;
	case 's':
	    cf_getString(cf, srcfilename, sizeof srcfilename);
	    chgtime = cf_getLong(cf);
	    if (n_m_index == n_module) continue; /* Static info. missing. */
	    for (mod = t_module; mod < t_module + n_module; ++mod) {
		if (STREQ(mod->file[0].filename, srcfilename)) break;
	    }
	    if (mod >= t_module + n_module) {
		mod = NULL;
		file = -1;
		continue;	/* Static info. missing. */
	    }
	    if (!(options & OPTION_IGNORE_SRC_TIMESTAMP)
		&& mod->file[0].chgtime != chgtime)
	    {
		fprintf(stderr,
			"Test %s.%s invalid due to modification of \"%s\"\n",
			filename, testName, srcfilename);
		exit(1);
	    }
	    m_index[n_m_index].file = file;
	    m_index[n_m_index++].module = mod;
	    break;
	case 'b':
	    if (!(options & OPTION_BLOCK)) break;
	    if (mod == NULL) continue;	/* Static info. missing. */
	    funcNo = cf_getLong(cf);
	    b1 = cf_getLong(cf);
	    if (funcNo >= (int)mod->n_func) {
		trace_error(filename, cf_lineNo(cf));
	    }
	    func = mod->func + funcNo;
	    if (func->ignore) break;
	    if (b1 < 0 || b1 >= (int)func->n_blk) {
		trace_error(filename, cf_lineNo(cf));
	    }
	    c = cf_getLong(cf);
	    if (c == 0) c = 1;
	    if ((cov[b1 + func->blkCovStart] += c) <= 0)
		cov[b1 + func->blkCovStart] = MAX_COUNT;
	    break;
	case 'c':
	    if (!(options & OPTION_CUSE)) break;
	    if (mod == NULL) continue;	/* Static info. missing. */
	    funcNo = cf_getLong(cf);
	    varNo = cf_getLong(cf);
	    b1 = cf_getLong(cf);
	    b2 = cf_getLong(cf);
	    if (funcNo < 0 || funcNo >= (int)mod->n_func) {
		trace_error(filename, cf_lineNo(cf));
	    }
	    func = mod->func + funcNo;
	    if (func->ignore) break;
	    if (varNo < 0 || varNo >= (int)func->n_var) {
		trace_error(filename, cf_lineNo(cf));
	    }
	    if (b1 < 0 || b1 >= (int)func->n_blk) {
		trace_error(filename, cf_lineNo(cf));
	    }
	    if (b2 < 0 || b2 >= (int)func->n_blk) {
		trace_error(filename, cf_lineNo(cf));
	    }
	    for (i = func->var[varNo].cstart; i < (int)func->n_cuse; ++i) {
		cuse = func->cuse + i;
		if (cuse->varno != varNo) break;
		if (cuse->blk1 == b1 && cuse->blk2 == b2) {
		    c = cf_getLong(cf);
		    if (c == 0) c = 1;
		    if ((cov[func->cUseCovStart + i] += c) <= 0)
			cov[func->cUseCovStart + i] = MAX_COUNT;
		    break;
		}
	    }
	    break;
	case 'p':
	    if (!(options & OPTION_PUSE)) break;
	    if (mod == NULL) continue; 	/* Static info. missing. */
	    funcNo = cf_getLong(cf);
	    varNo = cf_getLong(cf);
	    b1 = cf_getLong(cf);
	    b2 = cf_getLong(cf);
	    b3 = cf_getLong(cf);
	    if (funcNo < 0 || funcNo >= (int)mod->n_func) {
		trace_error(filename, cf_lineNo(cf));
	    }
	    func = mod->func + funcNo;
	    if (func->ignore) break;
	    if (varNo < 0 || varNo >= (int)func->n_var) {
		trace_error(filename, cf_lineNo(cf));
	    }
	    if (b1 < 0 || b1 >= (int)func->n_blk) {
		trace_error(filename, cf_lineNo(cf));
	    }
	    if (b2 < 0 || b2 >= (int)func->n_blk) {
		trace_error(filename, cf_lineNo(cf));
	    }
	    if (b3 < 0 || b3 >= (int)func->n_blk) {
		trace_error(filename, cf_lineNo(cf));
	    }
	    for (i = func->var[varNo].pstart; i < (int)func->n_puse; ++i) {
		puse = func->puse + i;
		if (puse->varno != varNo) break;
		if (puse->blk1 == b1 && puse->blk2 == b2 && puse->blk3 == b3) {
		    c = cf_getLong(cf);
		    if (c == 0) c = 1;
		    if ((cov[func->pUseCovStart + i] += c) <= 0)
			cov[func->pUseCovStart + i] = MAX_COUNT;
		    break;
		}
	    }
	    break;
	default:
	    trace_error(filename, cf_lineNo(cf));
	}
    }

    return c;
}

/*
* skipFlatTest: Read past an uncompressed trace.  Stop reading at EOF or t
* 	entry.
*/
static int				/* Return next input character. */
skipFlatTest(cf, filename)
struct cfile	*cf;
char		*filename;
{
    int	c;

    while ((c = cf_getFirstChar(cf)) != EOF) {
	switch (c)
	{
	case 't':
	    return c;
        case 'f':
	case 's':
	case 'h':
	case 'b':
	case 'c':
	case 'p':
	    break;
	default:
	    trace_error(filename, cf_lineNo(cf));
	}
    }

    return c;
}

/*
* getIndex:  Read next nTests "I" lines from f and fill in tests[] data.
*/
static void
getIndex(cf, filename, tests, nTests)
struct cfile	*cf;
char		*filename;
T_TEST		*tests;
int		nTests;
{
    int		c;
    int		i;
    char	buf[20];

    for (i = 0; i < nTests; ++i) {
	c = cf_getFirstChar(cf);
	if (c != 'I') {
	    trace_error(filename, cf_lineNo(cf));
	}
	cf_getString(cf, tests[i].timeStamp, TIMESTAMP_SIZE);
	cf_getString(cf, tests[i].name, TESTNAME_SIZE);
	cf_getString(cf, NULL, 0);
	tests[i].cost = cf_getLong(cf);
	cf_getString(cf, buf, sizeof buf);
	tests[i].freq = STREQ(buf, "frequency");
    }

    return;
}

/*
* getMIndex:  Read next nModIndex "M" lines from f and fill in modIndex[].
*	modIndex maps a module number to a pointer to module static data.
*/
static void
getMIndex(cf, filename, t_module, n_module, modIndex, nModIndex)
char		*filename;
struct cfile	*cf;
T_MODULE	*t_module;
int		n_module;
T_MODULE	*modIndex[];
int		nModIndex;
{
    int		i;
    T_MODULE	*mod;		/* Module info for file_id: file */
    int		c;
    char	srcfilename[FILENAME_SIZE];

    for (i = 0; i < nModIndex; ++i) {
	c = cf_getFirstChar(cf);
	if (c != 'M') {
	    trace_error(filename, cf_lineNo(cf));
	}
	cf_getString(cf, srcfilename, sizeof srcfilename);
	for (mod = t_module; mod < t_module + n_module; ++mod) {
	    if (STREQ(mod->file[0].filename, srcfilename)) break;
	}
	if (mod < t_module + n_module) {
	    modIndex[i] = mod;
	} else {
	    modIndex[i] = NULL;
	}
    }

    return;
}

/*
* checkSrcStamp:  Read next nSource "S" lines from f and fill in and check
*	that the time stamp matches the one in the static data for the
*	module.
*/
static void
checkSrcStamp(cf, filename, tests, nTests, modIndex, nModIndex,
	      nSource, selectPattern, deselect)
struct cfile	*cf;
char		*filename;
T_TEST		*tests;
int		nTests;
T_MODULE	*modIndex[];
int		nModIndex;
int		nSource;
char		*selectPattern;
int		deselect;
{
    int		i;
    int		j;
    int		k;
    T_MODULE	*mod;
    int		modNumber;
    char	srcfilename[FILENAME_SIZE];
    int		c;
    time_t	chgtime;
    int		exitFlag;

    exitFlag = 0;			/* 1 means abort at return */

    for (i = 0; i < nSource; ++i) {
	c = cf_getFirstChar(cf);
	if (c != 'S') {
	    trace_error(filename, cf_lineNo(cf));
	}
	modNumber = cf_getLong(cf);
	if (modNumber < 0 || modNumber >= nModIndex) {
	    trace_error(filename, cf_lineNo(cf));
	}
	mod = modIndex[modNumber];
	if (mod == NULL) {	/* Static info. missing for this module. */
	    continue;
	}
	cf_getString(cf, srcfilename, sizeof srcfilename);
	for (k = 0; k < (int)mod->n_file; ++k) {
	    if (STREQ(mod->file[k].filename, srcfilename)) break;
	}
	if (k == mod->n_file) continue;

	if (mod->file[k].chgtime == 0) continue;	/* no time stamp info */

	for (j = 0; j < nTests; ++j) {
	    chgtime = cf_getLong(cf);
	    if (chgtime == 0) {	    /* Source file not entered by this test. */
		continue;
	    }
	    if (mod->file[k].chgtime != chgtime) {
		if (!patMatch(selectPattern, tests[j].name, deselect)) continue;
		fprintf(stderr, "%s %s: test is older than \"%s\".\n",
			filename, tests[j].name, srcfilename);
		exitFlag = 1;
	    }
	}
    }

    if (exitFlag) exit(1);
}

/*
* getSkip:  Skip the next nSkip input lines.  Validate that they are of
*	the given type.
*/
static void
getSkip(cf, filename, nSkip, type)
struct cfile	*cf;
char		*filename;
int		nSkip;
char		type;
{
    int		i;
    int		c;

    for (i = 0; i < nSkip; ++i) {
	c = cf_getFirstChar(cf);
	if (c != type) {
	    trace_error(filename, cf_lineNo(cf));
	}
    }
}

/*
* Read the next nBlk B lines.  Update appropriate cov vector in testCov list.
*/
static void
getBlkCov(cf, filename, nBlk, modIndex, nModIndex, testCov, nTests)
struct cfile	*cf;
char		*filename;
int		nBlk;		/* number of records to read */
T_MODULE	*modIndex[];
int		nModIndex;
int		**testCov;
int		nTests;
{
    int		i;
    int		k;
    int		modNumber;
    T_MODULE	*mod;
    T_FUNC	*func;
    int		funcNo;		/* function number input field */ 
    int		b1;
    int		covIndex;
    int		c;

    for (i = 0; i < nBlk; ++i) {
	c = cf_getFirstChar(cf);
	if (c != 'B') {
	    trace_error(filename, cf_lineNo(cf));
	}
	modNumber = cf_getLong(cf);
	if (modNumber < 0 || modNumber >= nModIndex) {
	    trace_error(filename, cf_lineNo(cf));
	}
    	mod = modIndex[modNumber];
	if (mod == NULL) {	/* Static info. missing for this module. */
	    continue;
	}
	if (mod->ignore) {	/* All functions in this module ignored */
	    continue;
	}	
	funcNo = cf_getLong(cf);
	if (funcNo < 0 || funcNo >= (int)mod->n_func) {
	    trace_error(filename, cf_lineNo(cf));
	}
	func = mod->func + funcNo;
	if (func->ignore) {	/* Not interested in this function */
	    continue;
	}	

	b1 = cf_getLong(cf);
	if (b1 < 0 || b1 >= (int)func->n_blk) {
	    trace_error(filename, cf_lineNo(cf));
	}
	covIndex = b1 + func->blkCovStart;
	for (k = 0; k < nTests; ++k) {
	    if ((testCov[k][covIndex] += cf_getLong(cf)) < 0)
		testCov[k][covIndex] = MAX_COUNT;
	}
    }
}

/*
* Read the next nCuse C lines.  Update appropriate cov vector in testCov list.
*/
static void
getCuseCov(cf, filename, nCuse, modIndex, nModIndex, testCov, nTests)
struct cfile	*cf;
char		*filename;
int		nCuse;		/* number of records to read */
T_MODULE	*modIndex[];
int		nModIndex;
int		**testCov;
int		nTests;
{
    int		i;
    int		j;
    int		k;
    int		modNumber;
    T_MODULE	*mod;
    T_FUNC	*func;
    int		funcNo;		/* function number input field */ 
    int		b1;
    int		b2;
    int		varNo;
    int		covIndex;
    T_CUSE	*cuse;
    int		c;

    for (i = 0; i < nCuse; ++i) {
	c = cf_getFirstChar(cf);
	if (c != 'C') {
	    trace_error(filename, cf_lineNo(cf));
	}
	modNumber = cf_getLong(cf);
	if (modNumber < 0 || modNumber >= nModIndex) {
	    trace_error(filename, cf_lineNo(cf));
	}
    	mod = modIndex[modNumber];
	if (mod == NULL) {	/* Static info. missing for this module. */
	    continue;
	}
	if (mod->ignore) {	/* All functions in this module ignored */
	    continue;
	}	
	funcNo = cf_getLong(cf);
	if (funcNo < 0 || funcNo >= (int)mod->n_func) {
	    trace_error(filename, cf_lineNo(cf));
	}
	func = mod->func + funcNo;
	if (func->ignore) {	/* Not interested in this function */
	    continue;
	}	

	varNo = cf_getLong(cf);
	if (varNo < 0 || varNo >= (int)func->n_var) {
	    trace_error(filename, cf_lineNo(cf));
	}
	b1 = cf_getLong(cf);
	if (b1 < 0 || b1 >= (int)func->n_blk) {
	    trace_error(filename, cf_lineNo(cf));
	}
	b2 = cf_getLong(cf);
	if (b1 < 0 || b1 >= (int)func->n_blk) {
	    trace_error(filename, cf_lineNo(cf));
	}
	covIndex = func->cUseCovStart;
	for (j = func->var[varNo].cstart; j < (int)func->n_cuse; ++j) {
	    cuse = func->cuse + j;
	    if (cuse->varno != varNo) break;
	    if (cuse->blk1 == b1 && cuse->blk2 == b2) {
		for (k = 0; k < nTests; ++k) {
		    if ((testCov[k][covIndex+j] += cf_getLong(cf)) < 0)
			testCov[k][covIndex+j] = MAX_COUNT;
		} /* k loop */
		break;	/* Get out of j loop. */
	    }
	} /* j loop */
    } /* i loop */
}

/*
* Read the next nPuse P lines.  Update appropriate cov vector in testCov list.
*/
static void
getPuseCov(cf, filename, nPuse, modIndex, nModIndex, testCov, nTests)
struct cfile	*cf;
char		*filename;
int		nPuse;		/* number of records to read */
T_MODULE	*modIndex[];
int		nModIndex;
int		**testCov;
int		nTests;
{
    int		i;
    int		j;
    int		k;
    int		modNumber;
    T_MODULE	*mod;
    T_FUNC	*func;
    int		funcNo;		/* function number input field */ 
    int		b1;
    int		b2;
    int		b3;
    int		varNo;
    int		covIndex;
    T_PUSE	*puse;
    int		c;

    for (i = 0; i < nPuse; ++i) {
	c = cf_getFirstChar(cf);
	if (c != 'P') {
	    trace_error(filename, cf_lineNo(cf));
	}
	modNumber = cf_getLong(cf);
	if (modNumber < 0 || modNumber >= nModIndex) {
	    trace_error(filename, cf_lineNo(cf));
	}
    	mod = modIndex[modNumber];
	if (mod == NULL) {	/* Static info. missing for this module. */
	    continue;
	}
	if (mod->ignore) {	/* All functions in this module ignored */
	    continue;
	}	
	funcNo = cf_getLong(cf);
	if (funcNo < 0 || funcNo >= (int)mod->n_func) {
	    trace_error(filename, cf_lineNo(cf));
	}
	func = mod->func + funcNo;
	if (func->ignore) {	/* Not interested in this function */
	    continue;
	}	

	varNo = cf_getLong(cf);
	if (varNo < 0 || varNo >= (int)func->n_var) {
	    trace_error(filename, cf_lineNo(cf));
	}
	b1 = cf_getLong(cf);
	if (b1 < 0 || b1 >= (int)func->n_blk) {
	    trace_error(filename, cf_lineNo(cf));
	}
	b2 = cf_getLong(cf);
	if (b2 < 0 || b2 >= (int)func->n_blk) {
	    trace_error(filename, cf_lineNo(cf));
	}
	b3 = cf_getLong(cf);
	if (b3 < 0 || b3 >= (int)func->n_blk) {
	    trace_error(filename, cf_lineNo(cf));
	}
	covIndex = func->pUseCovStart;
	for (j = func->var[varNo].pstart; j < (int)func->n_puse; ++j) {
	    puse = func->puse + j;
	    if (puse->varno != varNo) break;
	    if (puse->blk1 == b1 && puse->blk2 == b2 && puse->blk3 == b3) {
		for (k = 0; k < nTests; ++k) {
		    if ((testCov[k][covIndex+j] += cf_getLong(cf)) < 0)
			testCov[k][covIndex+j] = MAX_COUNT;
		} /* k loop */
		break;	/* Get out of j loop. */
	    }
	} /* j loop */
    }
}

/*
* getCompressed:  Read compressed part of tracefile starting after V line.
* Return pointer to list of coverage vectors for selected tests in
* selectListPtr.  The caller is responsible for freeing the list and all the
* coverage vectors in it.  See trace_data() below.  testsPtr is returned
* with a full list of tests. nTests is returned the number of tests.
* The caller is responsible for freeing this too.
*/
static void
getCompressed(cf, filename, t_module, n_module, nCov, options,
	      selectPattern, selectListPtr, nSelectPtr, testsPtr, nTestsPtr)
struct cfile	*cf;
char		*filename;
T_MODULE	*t_module;	/* static data */
int		n_module;
int		nCov;		/* Total coverage items in static data */
int		options;
char		*selectPattern;	/* trace name selection pattern */
T_TESTLIST	**selectListPtr;   	/* return selectList */
int		*nSelectPtr;
T_TEST		**testsPtr;
int		*nTestsPtr;
{
    int		i;			/* index for nTests */
    int		j;			/* index for nCov */
    int		nRecs;			/* Num of input records of given type */
    int		nTests;			/* number of compressed tests */
    T_TEST	*tests = NULL;		/* index of compressed tests */
    int		**covIndex = NULL;	/* coverage of compressed tests */
    int		nModIndex;		/* num modules used in compressed ... */
    T_MODULE	**modIndex = NULL;	/* index of modules in compressed ... */
    int		nSelect;		/* number of tests selected */
    T_TESTLIST	*selectList;		/* list of tests selected */
    int		*throwAway;		/* coverage vector for unselected ... */
    int		*keep;			/* coverage vector for selected ... */
    int		c;
    
    /*
    * Read Index.
    */
    c = cf_getFirstChar(cf);
    if (c != 'I') {
	trace_error(filename, cf_lineNo(cf));
    }
    nTests = cf_getLong(cf);
    if (nTests != 0) {
	tests = (T_TEST *)malloc(nTests * sizeof *tests);
	CHECK_MALLOC(tests);
	getIndex(cf, filename, tests, nTests);
    }

    /*
    * Read Modules.
    */
    c = cf_getFirstChar(cf);
    if (c != 'M') {
	trace_error(filename, cf_lineNo(cf));
    }
    nModIndex = cf_getLong(cf);
    if (nModIndex) {
	modIndex  = (T_MODULE **)malloc(nModIndex * sizeof *modIndex);
	CHECK_MALLOC(modIndex);
	getMIndex(cf, filename, t_module, n_module, modIndex, nModIndex);
    }

    /*
    * Read or skip source file list.
    */
    c = cf_getFirstChar(cf);
    if (c != 'S') {
	trace_error(filename, cf_lineNo(cf));
    }
    nRecs = cf_getLong(cf);
    if (options & OPTION_IGNORE_SRC_TIMESTAMP) {
	getSkip(cf, filename, nRecs, 'S');
    } else {
	checkSrcStamp(cf, filename, tests, nTests, modIndex, nModIndex,
		      nRecs, selectPattern, options & OPTION_DESELECT);
    }

    /*
    * Allocate an array of pointers to coverage vectors.  If results are
    * needed by test, allocate a coverage vector for each test.  Otherwise
    * allocate a single vector and assign it to all selected tests.
    * If any tests are deselected, assign them a throw away vector.
    */

    if (selectPattern && nCov != 0) {
	throwAway = (int *)malloc(nCov * sizeof *throwAway);
	CHECK_MALLOC(throwAway);
	for (j = 0; j < nCov; ++j) {	/* keep purify happy */
	    throwAway[j] = 0;
	}
    } else {
	throwAway = NULL;
    }

    if (!(options & OPTION_BY_TEST) && nCov != 0) {
	keep = (int *)malloc(nCov * sizeof *keep);
	CHECK_MALLOC(keep);
	for (j = 0; j < nCov; ++j) {
	    keep[j] = 0;
	}
    } else {
	keep = NULL;
    }
	
    if (nTests) {
	covIndex = (int **)malloc(nTests * sizeof *covIndex);
	CHECK_MALLOC(covIndex);
    }
    nSelect = 0;
    for (i = 0; i < nTests; ++i) {
	if ((!(options & OPTION_FREQ) || tests[i].freq) &&
	    patMatch(selectPattern, tests[i].name, options & OPTION_DESELECT))
	{
	    ++nSelect;
	    if ((options & OPTION_BY_TEST)) {
		if (nCov != 0) {
		    keep = (int *)malloc(nCov * sizeof *keep);
		    CHECK_MALLOC(keep);
		    for (j = 0; j < nCov; ++j) {
		      keep[j] = 0;
		    }
		} else {
		    keep = NULL;
		}
	    }
	    covIndex[i] = keep;
	} else {
	    covIndex[i] = throwAway;
	}
    }

    /*
    * Read or skip blocks.
    */
    c = cf_getFirstChar(cf);
    if (c != 'B') {
	trace_error(filename, cf_lineNo(cf));
    }
    nRecs = cf_getLong(cf);
    if ((options & OPTION_BLOCK) && nSelect != 0) {
	getBlkCov(cf, filename, nRecs, modIndex, nModIndex, covIndex,
		  nTests);
    } else {
	getSkip(cf, filename, nRecs, 'B');
    }

    /*
    * Read or skip C-uses.
    */
    c = cf_getFirstChar(cf);
    if (c != 'C') {
	trace_error(filename, cf_lineNo(cf));
    }
    nRecs = cf_getLong(cf);
    if ((options & OPTION_CUSE) && nSelect != 0) {
	getCuseCov(cf, filename, nRecs, modIndex, nModIndex, covIndex, nTests);
    } else {
	getSkip(cf, filename, nRecs, 'C');
    }

    /*
    * Read or skip P-uses.
    */
    c = cf_getFirstChar(cf);
    if (c != 'P') {
	trace_error(filename, cf_lineNo(cf));
    }
    nRecs = cf_getLong(cf);
    if ((options & OPTION_PUSE) && nSelect != 0) {
	getPuseCov(cf, filename, nRecs, modIndex, nModIndex, covIndex, nTests);
    } else {
	getSkip(cf, filename, nRecs, 'P');
    }

    /*
    * Allocate structure to return coverage vectors.  Only vectors for
    * selected tests.  Include the test name.  This structure will need
    * expansion if uncompressed tests follow and are selected BY_TEST.
    */
    if (options & OPTION_BY_TEST) {
        if (nSelect) {
	    selectList = (T_TESTLIST *)malloc(nSelect * sizeof *selectList);
	    CHECK_MALLOC(selectList);
	    j = 0;
	    for (i = 0; i < nTests; ++i) {
		if (covIndex[i] != throwAway) {
		    strcpy(selectList[j].name, tests[i].name);
		    selectList[j].cost = tests[i].cost;
		    selectList[j].cov = covIndex[i];
		    ++j;
		}
	    }
	} else {
	    selectList = NULL;
	}
    } else {
	selectList = (T_TESTLIST *)malloc(sizeof *selectList);
	CHECK_MALLOC(selectList);
	strcpy(selectList[0].name, "all");
	selectList[0].cost = 0;
	selectList[0].cov = keep;
    }

    /*
    * Free compressed trace test index, module index, covIndex, and throwAway
    * coverage vector.
    */
    if (nModIndex != 0) free(modIndex);
    if (nTests != 0) free(covIndex);
    if (selectPattern && nCov != 0) free(throwAway);

    /*
    * Return coverage vector(s) for selected traces.
    */
    *selectListPtr = selectList;
    if (options & OPTION_BY_TEST) {
	*nSelectPtr = nSelect;
    } else {
	*nSelectPtr = 1;
    }

    /*
    * Return tests list.
    */
    *nTestsPtr = nTests;
    *testsPtr = tests;
}

/*
* testNo: Return the next available .n suffix for testName.
*/
static int
testNo(tests, nTests, testName)
T_TEST		*tests;
int		nTests;
char		*testName;
{
    int		i;
    int		len;
    int		max;
    char	*pName;
    int		n;

    max = 0;
    len = strlen(testName);
    
    for (i = 0; i < nTests; ++i) {
	pName = tests[i].name;
	if (strncmp(pName, testName, len) == 0 && pName[len] == '.') {
	    n = atoi(pName + len + 1);
	    if (n > max) {
		max = n;
	    }
	}
    }

    return max + 1;
}

/*
* trace_data:  Read tracefile and return pointer to list of coverage vectors
* for selected tests in selectListPtr.  The caller is responsible for freeing
* the list and all the coverage vectors in it.  If selectPattern is not NULL,
* coverage vectors are selected only for tests with name matched by
* selectPattern.  Otherwise all are selected.  If options include DESELECT,
* only names that donot match selectPattern are selected.  If options include
* FREQ only tests with frequency counts are selected.  If options include
* BY_TEST coverage vectors are returned for all selected tests (possibly 0).
* Otherwise, a single vector is returned containing the sum of all selected
* tests.  If 0 tests are selected, there is no list for the caller to free.
*/
void
trace_data(cf, filename, t_module, n_module, nCov, options, selectPattern,
	   selectListPtr, nSelectPtr)
struct cfile	*cf;
char		*filename;
T_MODULE	*t_module;	/* static data */
int		n_module;
int		nCov;		/* Total coverage items in static data */
int		options;
char		*selectPattern;	/* trace name selection pattern */
T_TESTLIST	**selectListPtr;   	/* return selectList */
int		*nSelectPtr;
{
    int		j;			/* index for nCov */
    int		nSelect;		/* number of tests selected */
    T_TESTLIST	*selectList;		/* list of tests selected */
    T_TESTLIST	*newSelectList;		/* reallocation for selectList */
    int		*uncompressed = NULL;	/* coverage vector for uncompressed ..*/
    char	testName[TESTNAME_SIZE];	/* uncompressed test name */
    T_TEST	*tests;
    int		nTests;
    int		c;
    
    /*
    * Read compressed traces.
    */
    c = cf_getFirstChar(cf);
    if (c == 'V') {
	getCompressed(cf, filename, t_module, n_module, nCov, options,
		      selectPattern, &selectList, &nSelect, &tests, &nTests);
	c = cf_getFirstChar(cf);
    } else {
	if (!(options & OPTION_BY_TEST)) {
	    selectList = (T_TESTLIST *)malloc(sizeof *selectList);
	    CHECK_MALLOC(selectList);
	    strcpy(selectList[0].name, "all");
	    selectList[0].cost = 0;
	    if (nCov != 0) {
		selectList[0].cov = (int *)malloc(nCov * sizeof(int *));
		CHECK_MALLOC(selectList[0].cov);
 		for (j = 0; j < nCov; ++j) {
		    selectList[0].cov[j] = 0;
		}
	    } else {
		selectList[0].cov = NULL;
	    }
	} else {
	    selectList = NULL;
	}
	nSelect = 0;
	nTests = 0;
	tests = NULL;
    }
	
    /*
    * Read uncompressed traces.
    */
    while (c != EOF) {
	if (c != 't') {
	    fprintf(stderr, "%s: invalid trace file; <%c> line %d\n",
		    filename, c, cf_lineNo(cf));
	    exit(1);
	}

	++nTests;
	if (nTests == 1) {
	    tests = (T_TEST *)malloc(sizeof *tests);
	} else {
	    tests = (T_TEST *)realloc(tests, sizeof *tests * (nTests));
	}
	CHECK_MALLOC(tests);
	cf_getString(cf, tests[nTests - 1].timeStamp, TIMESTAMP_SIZE);
	cf_getString(cf, NULL, 0);
	cf_getString(cf, testName, sizeof testName);
	if (*testName == '\0')
	    strcpy(testName, "T");
	sprintf(testName + strlen(testName), ".%d",
		testNo(tests, nTests - 1, testName));
	strcpy(tests[nTests - 1].name, testName);
	tests[nTests - 1].cost = 100;
	tests[nTests - 1].freq = 0;

	if (patMatch(selectPattern, testName, options & OPTION_DESELECT)) {
	    if (nCov != 0) {
		uncompressed = (int *)malloc(nCov * sizeof *uncompressed);
		CHECK_MALLOC(uncompressed);
		for (j = 0; j < nCov; ++j) {
		    uncompressed[j] = 0;
		}
	    }
	    c = getFlatTest(cf, filename, t_module, n_module, testName,
			 uncompressed, options, &tests[nTests - 1].freq);
	    if ((options & OPTION_FREQ) && !tests[nTests - 1].freq) {
		free(uncompressed);
		continue;
	    }
	    if (options & OPTION_BY_TEST) {
		newSelectList =	(T_TESTLIST *)malloc((nSelect + 1)
						     * sizeof *newSelectList);
		CHECK_MALLOC(newSelectList);
		if (selectList) {
		    memcpy(newSelectList, selectList,
			   nSelect * sizeof *selectList);
		    free(selectList);
		}
		selectList = newSelectList;
		strcpy(selectList[nSelect].name, testName);
		selectList[nSelect].cost = tests[nTests - 1].cost;
		selectList[nSelect].cov = uncompressed;
		++nSelect;
	    } else {
		for (j = 0; j < nCov; ++j) {
		    if ((selectList[0].cov[j] += uncompressed[j]) < 0)
			selectList[0].cov[j] = MAX_COUNT;
		}
		free(uncompressed);
		nSelect = 1;
	    }
	} else {
	    c = skipFlatTest(cf, filename);
	}
    }

    if (nTests != 0)
	free(tests);	/* Used for computing test names & suffixes */

    /*
    * Return coverage vector(s) for selected traces.
    */
    if (options & OPTION_BY_TEST) {
	if (nSelect) {
	    *selectListPtr = selectList;
	    *nSelectPtr = nSelect;
	} else {
	    *selectListPtr = NULL;
	    *nSelectPtr = 0;
	}
    } else {
	*selectListPtr = selectList;
	*nSelectPtr = 1;
    }
}
