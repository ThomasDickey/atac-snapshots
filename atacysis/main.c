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
#include <string.h>
#include <ctype.h>

#include "portable.h"
#include "atacysis.h"

static char const main_c[] = "$Id: main.c,v 3.15 2013/12/09 01:10:41 tom Exp $";
static char const bellcoreCopyRight[] =
"Copyright (c) 1993 Bell Communications Research, Inc. (Bellcore)";

/*
* @Log: main.c,v @
* Revision 3.13  1995/12/29 21:24:41  tom
* adjust headers, prototyped for autoconfig
* corrected gcc warnings (wrong # of params for print_mod)
*
* Revision 3.12  94/04/04  13:51:25  saul
* Fix binary copyright.
* 
* Revision 3.11  94/04/04  10:25:36  jrh
* Add Release Copyright
* 
* Revision 3.10  93/12/14  09:15:55  saul
* fix -mu (didn't work in display mode)
* 
* Revision 3.9  93/09/03  09:08:35  saul
* Put type casts back (as in 3.7).  Keep other fix.
* 
* Revision 3.8  93/08/23  15:44:42  ewk
* Eliminated many casts for Solaris warnings by modifying type decls.
* 
* Revision 3.7  93/08/09  12:36:39  saul
* in -n feature check return from evalExpr
* 
* Revision 3.6  1993/08/04  15:55:28  ewk
* Added MVS and solaris support.  Squelched some ANSI warnings.
*
* Revision 3.5  93/06/30  15:18:33  saul
* atac -N experimental feature
* fix input to minimize (atacmin) for -m d, c, and p
* 
* Revision 3.4  93/04/27  15:40:54  ewk
* Removed predecrement in error messages when f and g
* flags are used together.
* 
* Revision 3.3  93/03/29  11:19:09  saul
* Make -a default when -k used (risk).
* 
* Revision 3.2  93/01/15  12:36:31  saul
* accept .ata, .at, .a for .atac
* 
* Revision 3.1  92/12/03  08:51:01  saul
* CUMULATIVE, COST, SORT options.  Also memory leak patch.
* 
* Revision 3.0  92/11/06  07:47:47  saul
* propagate to version 3.0
* 
* Revision 2.13  92/10/30  09:54:28  saul
* include portable.h
* 
* Revision 2.12  92/10/29  11:25:45  saul
* Changed default for -s to -mbdcp.
* 
* Revision 2.11  92/10/29  11:03:10  saul
* bad fclose() may cause core dump on atac -c
* 
* Revision 2.10  92/10/01  13:30:23  saul
* Force -a behaivour whenever byTest.  (Min problem.)
* 
* Revision 2.9  92/09/30  11:18:57  saul
* Add cost field for atacMin.
* 
* Revision 2.8  92/09/22  15:47:06  saul
* Trace compression.
* 
* Revision 2.7  92/09/08  09:17:44  saul
* New options, features, data structures, ...
* 
* Revision 2.6  92/07/10  11:25:01  saul
* new options for minimization and risk; unlimmited trace and atac files
* new p-use and c-use options; usage message
* 
* Revision 2.5  92/03/17  15:27:09  saul
* copyright
* 
* Revision 2.4  91/08/30  13:48:10  saul
* Fix invalid trace handling.
* 
* Revision 2.3  91/08/14  11:52:10  saul
* put trace name with -m
* 
* Revision 2.2  91/08/14  09:12:24  saul
* -m (minimize) option
* 
* Revision 2.1  91/06/19  13:10:02  saul
* Propagate to version 2.0
* 
* Revision 1.1  91/06/12  16:58:27  saul
* Aug 1990 baseline
* 
*-----------------------------------------------end of log
*/

#define CHECK_MALLOC(p) if((p)==NULL)fprintf(stderr,"Out of memory\n"),exit(1)

static void
usage(char *cmd)
{
    fprintf(stderr,
	    "Usage: %s [-afgikpqrsxzCHKMSTZ] [-c tests] [-m measure] [-n tests] [-F function] [-t coverage-threshold] [-N set-expression] [file.trace ...] file.atac ...\n",
	    cmd);
    fprintf(stderr,
	    "\t-a (all) include constructs covered by a weaker measure\n");
    fprintf(stderr,
	    "\t-c compare selected tests with these tests (not with -p)\n");
    fprintf(stderr, "\t-f coverage by function (used with -s)\n");
    fprintf(stderr, "\t-g coverage by source file (used with -s)\n");
    fprintf(stderr,
	    "\t-i (ignore) do not detect modified source files (risky)\n");
    fprintf(stderr, "\t-k risk browser input\n");
    fprintf(stderr,
	    "\t-m measure is one of the following coverage measures:");
    fprintf(stderr, "\n\t  ");
    fprintf(stderr, " b block,");
    fprintf(stderr, " c C-use,");
    fprintf(stderr, " d decision,");
    fprintf(stderr, " e function entry,");
    fprintf(stderr, " p P-use,");
    fprintf(stderr, "\n\t  ");
    fprintf(stderr, " u alluse,");
    fprintf(stderr, " default -mbdcp\n");
    fprintf(stderr,
	    "\t-n (names) select tests matching comma separated patterns (using ?*[])\n");
    fprintf(stderr, "\t-p (per test) coverage by test (used with -s)\n");
    fprintf(stderr, "\t-q cumulative coverage by test (used with -s)\n");
    fprintf(stderr, "\t-r (reverse) display covered rather than uncovered\n");
    fprintf(stderr, "\t-s summary statistics\n");
    fprintf(stderr, "\t-S sort\n");
    fprintf(stderr, "\t-t coverage threshold\n");
    fprintf(stderr, "\t-x exclude tests selected with -n\n");
    fprintf(stderr, "\t-z dynamic dump\n");
    fprintf(stderr, "\t-C count summary\n");
    fprintf(stderr, "\t-H highest count summary\n");
    fprintf(stderr, "\t-K include cost in summary (used with -s)\n");
    fprintf(stderr,
	    "\t-F select functions matched by comma separated patterns (using ?*[])\n");
    fprintf(stderr, "\t-M minimization vector\n");
    fprintf(stderr, "\t-N set selection expression\n");
    fprintf(stderr, "\t-T tabular display\n");
    fprintf(stderr, "\t-Z static dump\n");
}

/*
* CovThreshold: Adjust each covVector in selectList for threshold.
* Set every entry to 1 (covered) or 0 (not).
*/
void
covThreshold(int *cov,
	     int covCount,
	     int threshold)
{
    int j;

    for (j = 0; j < covCount; ++j) {
	if (cov[j] >= threshold) {
	    cov[j] = 1;
	} else {
	    cov[j] = 0;
	}
    }
}

static void
covReverse(int *cov,
	   int covCount)
{
    int j;

    for (j = 0; j < covCount; ++j) {
	if (cov[j] != -1) {
	    cov[j] = !cov[j];
	}
    }
}

void
covWeaker(T_MODULE * modules,
	  int n_mod,
	  int *covVector,
	  int options)
{
    int i;
    int j;
    T_FUNC *func;
    T_MODULE *mod;
    T_PUSE *puse;
    T_PUSE *d = NULL;
    T_CUSE *cuse;
    int decis_var;

    for (mod = modules; mod < modules + n_mod; ++mod) {
	for (func = mod->func; func < mod->func + mod->n_func; ++func) {
	    if (func->ignore)
		continue;
	    if (options & OPTION_BLOCK) {
		if (covVector[func->blkCovStart] == 0) {
		    for (i = 1; i < (int) func->n_blk; ++i) {
			covVector[func->blkCovStart + i] = -1;
		    }
		}
	    }
	    if (options & OPTION_CUSE) {
		for (i = 0; i < (int) func->n_cuse; ++i) {
		    cuse = func->cuse + i;
		    if (covVector[func->blkCovStart + cuse->blk1] != 1) {
			covVector[func->cUseCovStart + i] = -1;
			continue;
		    }
		    if (covVector[func->blkCovStart + cuse->blk2] != 1) {
			covVector[func->cUseCovStart + i] = -1;
			continue;
		    }
		}
	    }
	    if (options & OPTION_PUSE) {
		decis_var = func->decis_var;
		for (i = 0; i < (int) func->n_puse; ++i) {
		    puse = func->puse + i;
		    if (covVector[func->blkCovStart + puse->blk2] != 1) {
			covVector[func->pUseCovStart + i] = -1;
			continue;
		    }
		    if (covVector[func->blkCovStart + puse->blk3] != 1) {
			covVector[func->pUseCovStart + i] = -1;
			continue;
		    }
		    /*
		     * For P-uses check blk1 or corresponding DECIS not covered.
		     */
		    if (puse->varno != decis_var) {
			if (covVector[func->blkCovStart + puse->blk1] != 1) {
			    covVector[func->pUseCovStart + i] = -1;
			    continue;
			}
			for (j = 0; j < (int) func->n_puse; ++j) {
			    d = func->puse + j;
			    if (d->varno != decis_var)
				break;
			    if (d->blk2 == puse->blk2
				&& d->blk3 == puse->blk3)
				break;
			}
			if (d->varno == decis_var &&
			    covVector[func->pUseCovStart + j] != 1) {
			    covVector[func->pUseCovStart + i] = -1;
			    continue;
			}
		    }
		}
	    }
	}
    }
}

int
main(int argc,
     char *argv[])
{
    T_MODULE *mod;
    int i;
    int j;
    char *p;
    int nSelect;
    int nCmp;
    int n_static = 0;
    char **static_v;
    struct cfile *cf;
    const char **traces;
    int covCount;
    T_TESTLIST *selectList;
    T_TESTLIST *cmpList;
    int options;

    /*
     * options
     */
    char *testSelect = NULL;
    char *cmpSelect = NULL;
    char *exprSelect = NULL;
    char *funcSelect = NULL;
    int covOptions = 0;
    int threshold = 1;
    int n_traces = 0;
    int dumpFlag = 0;
    int byFunc = 0;
    int byFile = 0;
    int doSummary = 0;
    int staticDump = 0;
    int ignoreFlag = 0;
    int displayMode = 0;
    int riskFlag = 0;
    int minimizeFlag = 0;
    int deselectFlag = 0;
    int freqFlag = 0;
    int byTest = 0;
    int tabularDisplay = 0;
    int counterAtac = 0;
    int doHighest = 0;
    int sortGreedy = 0;

    /*
     * Allocate lists for trace, and .atac file names.
     */
    traces = (const char **) malloc((size_t) argc * sizeof *traces);
    CHECK_MALLOC(traces);
    static_v = (char **) malloc((size_t) argc * sizeof *static_v);
    CHECK_MALLOC(static_v);

    /*
     * Collect options.
     */
    for (i = 1; i < argc; ++i) {
	p = argv[i];
	if (*p != '-')
	    break;
	while (*++p)
	    switch (*p) {
	    case 'a':
		displayMode |= DISPLAY_ALL;
		break;
	    case 'b':		/* obsolete */
		covOptions |= OPTION_BLOCK;
		break;
	    case 'c':
		if (*++p == '\0') {
		    if (++i == argc) {
			fprintf(stderr, "argument missing for -%c\n", *--p);
			usage(argv[0]);
			exit(1);
		    }
		    p = argv[i];
		}
		if (cmpSelect == NULL) {
		    cmpSelect = (char *) malloc(strlen(p) + 1);
		    CHECK_MALLOC(cmpSelect);
		    strcpy(cmpSelect, p);
		} else {	/* make a comma separated list */
		    cmpSelect = (char *) realloc(cmpSelect,
						 strlen(cmpSelect) +
						 strlen(p) + 2);
		    CHECK_MALLOC(cmpSelect);
		    strcat(cmpSelect, ",");
		    strcat(cmpSelect, p);
		}
		p += strlen(p) - 1;	/* setup for next arg */
		break;
	    case 'C':
		counterAtac = 1;
		doSummary = 1;
		break;
	    case 'd':		/* obsolete */
		displayMode |= DISPLAY_ALL;
		covOptions |= OPTION_DECIS;
		break;
	    case 'D':		/* obsolete */
		covOptions |= OPTION_DECIS;
		break;
	    case 'f':
		if (byFile) {
		    fprintf(stderr, "-%c cannot be combined with -%c\n", *p, 'g');
		    exit(1);
		}
		byFunc = 1;
		break;
	    case 'F':
		if (*++p == '\0') {
		    if (++i == argc) {
			fprintf(stderr, "argument missing for -%c\n", *--p);
			usage(argv[0]);
			exit(1);
		    }
		    p = argv[i];
		}
		if (funcSelect == NULL) {
		    funcSelect = (char *) malloc(strlen(p) + 1);
		    CHECK_MALLOC(funcSelect);
		    strcpy(funcSelect, p);
		} else {	/* make a comma separated list */
		    funcSelect = (char *) realloc(funcSelect,
						  strlen(funcSelect) +
						  strlen(p) + 2);
		    CHECK_MALLOC(funcSelect);
		    strcat(funcSelect, ",");
		    strcat(funcSelect, p);
		}
		p += strlen(p) - 1;	/* setup for next arg */
		break;
	    case 'g':
		if (byFunc) {
		    fprintf(stderr, "-%c cannot be combined with -%c\n", *p, 'f');
		    exit(1);
		}
		byFile = 1;
		break;
	    case 'h':
		covOptions |= OPTION_NO_HEADER;
		break;
	    case 'H':
		counterAtac = 1;
		doHighest = 1;
		doSummary = 1;
		break;
	    case 'i':
		ignoreFlag = 1;
		break;
	    case 'k':
		riskFlag = 1;
		displayMode |= DISPLAY_ALL;
		break;
	    case 'K':
		doSummary = 1;
		byTest = 1;
		covOptions |= OPTION_COST;
		break;
	    case 'm':
		if (*++p == '\0') {
		    if (++i == argc) {
			fprintf(stderr, "argument missing for -%c\n", *--p);
			usage(argv[0]);
			exit(1);
		    }
		    p = argv[i];
		}
		do
		    switch (*p) {
		    case 'b':
			covOptions |= OPTION_BLOCK;
			break;
		    case 'c':
			covOptions |= OPTION_CUSE;
			break;
		    case 'd':
			covOptions |= OPTION_DECIS;
			break;
		    case 'e':
			covOptions |= OPTION_F_ENTRY;
			break;
		    case 'p':
			covOptions |= OPTION_PUSE;
			break;
		    case 'u':
			covOptions |= OPTION_ALLUSE;
			break;
		    default:
			fprintf(stderr, "invalid coverage option: %c\n", *p);
			usage(argv[0]);
			exit(1);
		} while (*++p);
		--p;		/* setup for next arg */
		break;
	    case 'M':
		minimizeFlag = 1;
		byTest = 1;
		break;
	    case 'n':
		if (*++p == '\0') {
		    if (++i == argc) {
			fprintf(stderr, "argument missing for -%c\n", *--p);
			usage(argv[0]);
			exit(1);
		    }
		    p = argv[i];
		}
		if (testSelect == NULL) {
		    testSelect = (char *) malloc(strlen(p) + 1);
		    CHECK_MALLOC(testSelect);
		    strcpy(testSelect, p);
		} else {	/* make a comma separated list */
		    testSelect = (char *) realloc(testSelect,
						  strlen(testSelect) +
						  strlen(p) + 2);
		    CHECK_MALLOC(testSelect);
		    strcat(testSelect, ",");
		    strcat(testSelect, p);
		}
		p += strlen(p) - 1;	/* setup for next arg */
		break;
	    case 'N':
		if (*++p == '\0') {
		    if (++i == argc) {
			fprintf(stderr, "argument missing for -%c\n", *--p);
			usage(argv[0]);
			exit(1);
		    }
		    p = argv[i];
		}
		if (exprSelect != NULL) {
		    fprintf(stderr, "multiple -%c's\n", *--p);
		    usage(argv[0]);
		    exit(1);
		}
		exprSelect = (char *) malloc(strlen(p) + 1);
		CHECK_MALLOC(exprSelect);
		strcpy(exprSelect, p);
		p += strlen(p) - 1;	/* setup for next arg */
		break;
	    case 'p':
		doSummary = 1;
		byTest = 1;
		break;
	    case 'q':
		doSummary = 1;
		byTest = 1;
		covOptions |= OPTION_CUMULATIVE;
		break;
	    case 'r':
		displayMode |= DISPLAY_COV;
		break;
	    case 's':
		doSummary = 1;
		break;
	    case 'S':
		doSummary = 1;
		byTest = 1;
		sortGreedy = 1;
		break;
	    case 't':
		if (*++p == '\0') {
		    if (++i == argc) {
			fprintf(stderr, "argument missing for -%c\n", *--p);
			usage(argv[0]);
			exit(1);
		    }
		    p = argv[i];
		}
		threshold = atoi(p);
		for (; *p; ++p) {
		    if (!isdigit(*p)) {
			fprintf(stderr, "invalid argument for -t\n");
			exit(1);
		    }
		}
		--p;		/* setup for next arg */
		break;
	    case 'T':
		tabularDisplay = 1;
		break;
	    case 'x':
		deselectFlag = 1;
		break;
	    case 'z':
		dumpFlag = 1;
		break;
	    case 'Z':
		staticDump = 1;
		break;
	    case '?':
		usage(argv[0]);
		exit(1);
		break;
	    default:
		fprintf(stderr, "unknown option: -%c\n", *p);
		usage(argv[0]);
		exit(1);
		break;
	    }
    }

    /*
     * If no coverage measure selected, default to standard four.
     */
    if (!(covOptions & (OPTION_F_ENTRY | OPTION_BLOCK | OPTION_DECIS |
			OPTION_PUSE | OPTION_CUSE | OPTION_ALLUSE))) {
	covOptions |= OPTION_BLOCK | OPTION_DECIS | OPTION_CUSE | OPTION_PUSE;
    }

    if (exprSelect && (testSelect || cmpSelect || byTest || counterAtac)) {
	fprintf(stderr,
		"-N option not compatible with -C, -H, -c, -m, -n, or -p\n");
	usage(argv[0]);
	exit(1);
    }

    if (byTest) {
	if (cmpSelect) {
	    fprintf(stderr,
		    "-c option not compatible with -p or -m option\n");
	    usage(argv[0]);
	    exit(1);
	}
	if (covOptions & (OPTION_CUMULATIVE | OPTION_COST)) {
	    if (doHighest) {
		fprintf(stderr,
			"-H option not compatible with -q or -K option\n");
		usage(argv[0]);
		exit(1);
	    }
	}
	displayMode |= DISPLAY_ALL;
    }

    if (counterAtac) {
	if (threshold != 1) {
	    fprintf(stderr, "-t option not valid with -C\n");
	    usage(argv[0]);
	    exit(1);
	}
	if (sortGreedy) {
	    fprintf(stderr, "-S option not valid with -C\n");
	    usage(argv[0]);
	    exit(1);
	}
    }

    /*
     * Compute options for trace_data call.
     */
    options = OPTION_BLOCK;	/* CUSE & PUSE need block 0 for formal count */
    if (covOptions & OPTION_DECIS) {
	options |= OPTION_PUSE;
    }
    if (covOptions & OPTION_CUSE) {
	options |= OPTION_CUSE;
    }
    if (covOptions & OPTION_PUSE) {
	options |= OPTION_PUSE;
    }
    if (covOptions & OPTION_ALLUSE) {
	options |= OPTION_PUSE;
	options |= OPTION_CUSE;
    }
    if (byTest) {
	options |= OPTION_BY_TEST;
    }
    if (ignoreFlag) {
	options |= OPTION_IGNORE_SRC_TIMESTAMP;
    }
    if (deselectFlag) {
	options |= OPTION_DESELECT;
    }
    if (freqFlag) {
	options |= OPTION_FREQ;
    }

    /*
     *  Find trace and .atac files.
     */
    for (; i < argc; ++i) {
	j = (int) strlen(argv[i]) + 1;
	if (strcmp(argv[i] + j - sizeof ".atac", ".atac") == 0
	    || strcmp(argv[i] + j - sizeof ".ata", ".ata") == 0
	    || strcmp(argv[i] + j - sizeof ".at", ".at") == 0
	    || strcmp(argv[i] + j - sizeof ".a", ".a") == 0) {
	    if (n_static >= argc) {
		fprintf(stderr, "no. of .atac exceeds argc\n");
		exit(1);
	    }
	    static_v[n_static++] = argv[i];
	} else {
	    if (n_traces >= argc) {
		fprintf(stderr, "no. of traces exceed argc\n");
		exit(1);
	    }
	    traces[n_traces++] = argv[i];
	}
    }
    if (n_traces > 1) {
	fprintf(stderr, "Too many trace files.  Only one allowed\n");
	exit(1);
    }

    /*
     * Read static data for .atac files.
     */
    mod = static_data(n_static, static_v, funcSelect, options, &covCount);
    if (staticDump)
	for (i = 0; i < n_static; ++i) {
	    print_mod(mod + i, NULL);
	}
    if (covCount == 0) {
	fprintf(stderr, "no functions selected\n");
	exit(1);
    }

    nSelect = 0;		/* No tests seen so far. */
    if (n_traces == 0) {
	traces[n_traces++] = "a.out.trace";
    }

    if (exprSelect) {
	if (evalExpr(exprSelect, traces, n_traces, mod, n_static, covCount,
		     options, &selectList, &nSelect, threshold,
		     !(displayMode & DISPLAY_ALL)) != 0) {
	    fprintf(stderr, "invalid selection expression: %s\n", exprSelect);
	    exit(1);
	}
	if (!doSummary) {
	    if (displayMode & DISPLAY_COV)
		displayMode &= ~DISPLAY_COV;
	    else {
		displayMode |= DISPLAY_COV;
		covReverse(selectList[0].cov, covCount);
	    }
	}
    } else {
	/*
	 * Read trace data and create selectList.
	 */
	cf = (struct cfile *) cf_openIn(traces[0]);
	if (cf == NULL) {
	    fprintf(stderr, "can't open %s\n", traces[0]);
	    exit(1);
	}
	selectList = NULL;
	trace_data(cf, traces[0], mod, n_static, covCount, options,
		   testSelect, &selectList, &nSelect);
	cf_close(cf);
	if (nSelect == 0) {
	    fprintf(stderr, "no tests selected\n");
	    exit(1);
	}

	/*
	 * Read trace data again to get comparison tests if any; create cmpList.
	 */
	if (cmpSelect) {
	    nCmp = 0;
	    cf = (struct cfile *) cf_openIn(traces[0]);
	    if (cf == NULL) {
		fprintf(stderr, "can't open %s\n", traces[0]);
		exit(1);
	    }
	    cmpList = NULL;
	    trace_data(cf, traces[0], mod, n_static, covCount, options,
		       cmpSelect, &cmpList, &nCmp);
	    cf_close(cf);
	    if (nCmp == 0) {
		fprintf(stderr, "no tests selected by -c\n");
		exit(1);
	    }
	}

	if (counterAtac) {
	    covOptions |= OPTION_COUNTER_ATAC;
	    /*
	     * Disregard threshold.
	     */
	    if (cmpSelect) {
		for (j = 0; j < covCount; ++j) {
		    selectList[0].cov[j] -= cmpList[0].cov[j];
		}
	    }
	} else if (cmpSelect) {
	    /*
	     * Do comparison.
	     */
	    covThreshold(selectList[0].cov, covCount, threshold);
	    covThreshold(cmpList[0].cov, covCount, threshold);
	    if (!(displayMode & DISPLAY_ALL)) {
		covWeaker(mod, n_static, selectList[0].cov, options);
		covWeaker(mod, n_static, cmpList[0].cov, options);
	    }
	    for (j = 0; j < covCount; ++j) {
		if (cmpList[0].cov[j] == 1 && selectList[0].cov[j] == 1) {
		    selectList[0].cov[j] = 0;
		}
	    }
	    if (displayMode & DISPLAY_COV) {
		covReverse(selectList[0].cov, covCount);
	    }
	} else {
	    /*
	     * Adjust for threshold, DISPLAY_ALL option, and DISPLAY_COV option.
	     */
	    for (i = 0; i < nSelect; ++i) {
		covThreshold(selectList[i].cov, covCount, threshold);
		if (!(displayMode & DISPLAY_ALL)) {
		    covWeaker(mod, n_static, selectList[i].cov, options);
		}
		if (displayMode & DISPLAY_COV) {
		    covReverse(selectList[i].cov, covCount);
		}
	    }
	}
    }

    /*
     * Produce output.
     */
    if (dumpFlag)
	for (i = 0; i < n_static; ++i) {
	    print_mod(mod + i, selectList[0].cov);
    } else if (minimizeFlag) {
	for (i = 0; i < nSelect; ++i) {
	    printf("%s:%ld:", selectList[i].name, selectList[i].cost);
	    vectorPut(mod, n_static, selectList[i].cov, covOptions);
	}
    } else if (riskFlag) {
	risk(mod, n_static, selectList[0].cov, covOptions);
    } else if (doSummary) {
	if (doHighest) {
	    highest(mod, n_static, nSelect, selectList, covCount, byFunc,
		    byFile, covOptions);
	} else {
	    if (sortGreedy) {
		greedyOrder(nSelect, selectList, covCount);
	    }
	    summary(mod, n_static, nSelect, selectList, covCount, byFunc,
		    byFile, covOptions);
	}
    } else if (tabularDisplay) {
	tab_disp(covOptions, 0, mod, n_static, selectList[0].cov);
    } else {
	if (covOptions & OPTION_F_ENTRY) {
	    fdisp(mod, n_static, selectList[0].cov, displayMode);
	}
	if (covOptions & OPTION_BLOCK) {
	    bdisp(mod, n_static, selectList[0].cov, displayMode);
	}
	if (covOptions & OPTION_DECIS) {
	    ddisp(mod, n_static, selectList[0].cov, displayMode);
	}
	if (covOptions & (OPTION_CUSE | OPTION_ALLUSE)) {
	    cdisp(mod, n_static, selectList[0].cov, displayMode);
	}
	if (covOptions & (OPTION_PUSE | OPTION_ALLUSE)) {
	    pdisp(mod, n_static, selectList[0].cov, displayMode);
	}
    }

    /*
     * Clean up memory allocations.
     */
    for (i = 0; i < nSelect; ++i) {
	if (selectList[i].cov) {
	    free(selectList[i].cov);
	}
    }
    if (selectList) {
	free(selectList);
    }
    if (funcSelect) {
	free(funcSelect);
    }
    free(traces);
    free(static_v);
    freeStatic(mod, n_static);

    return 0;
}
