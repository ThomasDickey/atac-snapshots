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

#if HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <stdio.h>
#include <ctype.h>
#include <string.h>

#include "portable.h"
#include "atacysis.h"
#include "pack.h"
#include "ramfile.h"
#ifdef vms
#include <stat.h>
#else
#ifndef MVS
#include <sys/stat.h>	/* for portability must be below "ramfile.h" */
#endif /* MVS */
#endif /* vms */
#include "man.h"

static char const atactm_c[] = 
	"$Header: /users/source/archives/atac.vcs/atacysis/RCS/atactm.c,v 3.8 1997/07/17 18:32:53 tom Exp $";
static char const bellcoreCopyRight[] =
"Copyright (c) 1993 Bell Communications Research, Inc. (Bellcore)";

/*
* $Log: atactm.c,v $
* Revision 3.8  1997/07/17 18:32:53  tom
* ifdef'd "rename()" fallback for configure-script
*
* Revision 3.7  1995/12/29 21:25:35  tom
* adjust headers, prototyped for autoconfig
* correct gcc warnings (shadowed variables).
*
*Revision 3.6  94/04/04  13:51:21  saul
*Fix binary copyright.
*
*Revision 3.5  94/04/04  10:24:45  jrh
*Add Release Copyright
*
*Revision 3.4  93/08/11  14:39:58  saul
*VMS #include rejected by some cpp's
*
*Revision 3.3  93/08/04  15:52:29  ewk
*Added MVS and solaris support.  Squelched some ANSI warnings.
*
actm.c
*Revision 3.2  93/07/09  12:04:48  saul
*Change stat.h include for VMS
*
*Revision 3.1  92/12/30  07:52:35  saul
*RENAME_SUPPORTED ==> RENAME_SUPPORT error
*
*Revision 3.0  92/11/06  07:47:02  saul
*propagate to version 3.0
*
*Revision 2.10  92/11/02  15:47:45  saul
*remove unused errno definition
*
*Revision 2.9  92/11/02  11:42:34  saul
*use portable.h to deterimine use of rename()
*
*Revision 2.8  92/10/30  09:53:47  saul
*include portable.h
*
*Revision 2.7  92/10/29  10:59:36  saul
*Bad free()'s sometimes caused core dump on atactm -r.  Also memory leak.
*
*Revision 2.6  92/10/28  09:02:13  saul
*change #includes for portability
*
*Revision 2.5  92/10/01  13:29:02  saul
*Bug affecting -d and -e.  Index only should affect -l and -L only.
*
*Revision 2.4  92/09/30  11:45:59  saul
*Use ATAC_DIR & ATAC_TRACE.  Get rid of -R option.
*
*Revision 2.3  92/09/22  15:38:43  saul
*Trace compression.
*Overwrite temp trace file
*
*Revision 2.2  92/09/08  10:12:23  saul
*changed trace format and data structures
*
*Revision 2.1  92/09/08  09:59:58  saul
*Purdue trace management
*
*-----------------------------------------------end of log
*/

/* forward declarations */
#if !HAVE_RENAME
static int rename();
#endif /* !HAVE_RENAME */
static int exists P_((char *path));
static char *prefixFileName P_((char *filename, char *prefix));
static void usage P_((char *cmd));

#define CHECK_MALLOC(p) if((p)==NULL)memoryError("Out of memory"),exit(1)

#ifndef MVS
#define DEFAULT_TRACEFILE "a.out.trace"
#endif /* MVS */

int iTestCase;

static tablestype	tables;

static void
usage(cmd)
char	*cmd;
{
fprintf(stderr,"Usage:\n");
fprintf(stderr,"%s [-o new.trace] [file.trace]\n", cmd);
fprintf(stderr,"%s -c cost -n testname [-x] [-o new.trace] [file.trace]\n", cmd);
fprintf(stderr,"%s -d -n testnames [-x] [-o new.trace] [file.trace]\n", cmd);
fprintf(stderr,"%s -e -n testnames [-x] -o new.trace [file.trace]\n", cmd);
fprintf(stderr,"%s -l [-n testnames [-x]] [file.trace]\n", cmd);
fprintf(stderr,"%s -L [-n testnames [-x]] [file.trace]\n", cmd);
fprintf(stderr,"%s -r newname [-n testnames [-x]] [-o new.trace] [file.trace]\n", cmd);
fprintf(stderr,"\t-c assign cost to named test case\n");
fprintf(stderr,"\t-d delete named test cases\n");
fprintf(stderr,"\t-e extract named test cases into new trace file\n");
fprintf(stderr,"\t-l list test cases\n");
fprintf(stderr,"\t-L list test cases with time, cost, version, etc.\n");
fprintf(stderr,"\t-r rename test case\n");
fprintf(stderr,"\t-R rename multiple test cases\n");
fprintf(stderr,"\t-n test names (wild cards ?, * [chars])\n");
fprintf(stderr,"\t-o output trace file; defaults to input trace\n");
fprintf(stderr,"\t-x excluded tests specified with -n\n");
}

/*
* prefixFileName: Allocate a variant filename string by prefixing the
* non-directory part of filename with the string prefix.
*/
static char *
prefixFileName(filename, prefix)
char	*filename;
char	*prefix;
{
    char	*f;
    char	*p;
    char	*new;
    size_t	fLen;
    size_t	pLen;

    /*
    * Allocate new string.
    */
    fLen = strlen(filename);
    pLen = strlen(prefix);
    new = (char *)malloc(fLen + pLen + 1);
    CHECK_MALLOC(new);

    /*
     * Copy filename to new from right to left (starting with '\0') until '/'
     * or no more.
     */
    new += fLen + pLen;
    for (f = filename + fLen; f >= filename; --f) {
	if (*f == '/') break;
	*new-- = *f;
    }

    /*
    * Copy prefix to new from right to left.
    */
    for (p = prefix + pLen - 1; p >= prefix; --p) {
	*new-- = *p;
    }

    /*
    * Copy rest of filename to new.
    */
    for (;f >= filename; --f) {
	*new-- = *f;
    }

    return new + 1;
}

int
main(argc, argv)
int	argc;
char	*argv[];
{
#ifndef MVS
    char	*traceFile = NULL;
    char	*traceDir = NULL;
#endif /* MVS */
    char	*tracePath;
    struct cfile	*cf;
    char	*outfile;
    int		i;
    char	*p;
    int		function;
    int		cost = 0;
    char	*newName = NULL;
    char	*names;
    int		nMatch;
    int		deselect;
    int		indexOnly;

    function = ' ';
    names = NULL;
    outfile = NULL;
    deselect = 0;
    indexOnly = 0;

    /*
    * Collect options.
    */
    for (i = 1; i < argc; ++i) {
	p = argv[i];
	if (*p != '-') break;
	while (*++p) switch(*p)
	{
	case 'c':
	    if (function != ' ') {
		fprintf(stderr, "conflicting options: -%c -%c\n", function, *p);
		usage(argv[0]);
		exit(1);
	    }
	    function = *p;
	    if (*++p == '\0') {
		if (++i == argc) {
		    fprintf(stderr, "argument missing for -%c\n", *--p);
		    usage(argv[0]);
		    exit(1);
		}
		p = argv[i];
	    }
	    cost = atoi(p);
	    for (;*p; ++p) {
		if (*p < '0' || *p > '9') {
		    fprintf(stderr, "%s: invalid cost\n", argv[i]);
		    usage(argv[0]);
		    exit(1);
		}
	    }
	    --p;		/* setup for next arg */
	    break;
	case 'd':
	case 'e':
	case 'l':
	case 'L':
	    if (function != ' ') {
		fprintf(stderr, "conflicting options: -%c -%c\n", function, *p);
		usage(argv[0]);
		exit(1);
	    }
	    function = *p;
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
	    if (names == NULL) {
		names = (char *)malloc(strlen(p) + 1);
		CHECK_MALLOC(names);
		strcpy(names, p);
	    } else {
		names = (char *)realloc(names, strlen(names) + strlen(p) + 2);
		strcat(names, ",");
		strcat(names, p);
	    }
	    p += strlen(p) - 1;		/* setup for next arg */
	    break;
	case 'o':
	    if (outfile != NULL) {
		fprintf(stderr, "conflicting options: -%c -%c\n", *p, *p);
		usage(argv[0]);
		exit(1);
	    }
	    if (*++p == '\0') {
		if (++i == argc) {
		    fprintf(stderr, "argument missing for -%c\n", *--p);
		    usage(argv[0]);
		    exit(1);
		}
		p = argv[i];
	    }
	    outfile = p;
	    p += strlen(p) - 1;		/* setup for next arg */
	    break;
	case 'r':
	    if (function != ' ') {
		fprintf(stderr, "conflicting options: -%c -%c\n", function, *p);
		usage(argv[0]);
		exit(1);
	    }
	    function = *p;
	    if (*++p == '\0') {
		if (++i == argc) {
		    fprintf(stderr, "argument missing for -%c\n", *--p);
		    usage(argv[0]);
		    exit(1);
		}
		p = argv[i];
	    }
	    newName = p;
	    for (; *p; ++p) {
		if (!isalnum(*p) && *p != '_') {
		    fprintf(stderr, "%s: invalid\n", newName);
		    exit(1);
		}
	    }
	    --p;		/* setup for next arg */
	    break;
	case 'x':
	    deselect = 1;
	    break;
	default:
	    fprintf(stderr, "unknown option: -%c\n", *p);
	    usage(argv[0]);
	    exit(1);
	    break;
	}
    }

    /*
    * Collect trace files. (Only one allowed currently.)
    */
#ifndef MVS 
    if (argc == i) {
	traceFile = getenv("ATAC_TRACE");
	if (traceFile == NULL)
	    traceFile = DEFAULT_TRACEFILE;
	traceDir = getenv("ATAC_DIR");
    }
    else if (argc == i + 1) {
	traceFile = argv[i];
	if (exists(traceFile)) {
	    traceDir = NULL;
	} else {
	    traceDir = getenv("ATAC_DIR");
	}
    } else {
	fprintf(stderr, "%s: unexpected arguments\n", argv[i+1]);
	usage(argv[0]);
	exit(1);
    }
    if (traceDir && *traceFile != '/') {
	tracePath = (char *)malloc(strlen(traceFile) + strlen(traceDir) + 2);
	CHECK_MALLOC(tracePath);
	sprintf(tracePath, "%s/%s", traceDir, traceFile);
    } else {
	tracePath = (char *)malloc(strlen(traceFile) + 1);
	CHECK_MALLOC(tracePath);
	strcpy(tracePath, traceFile);
    }	
#else /* MVS */
    if (argc == i) {
	tracePath = "DD:ATACTRCE";
    } else if (argc == i + 1) {
	tracePath = argv[i];
    } else {
	fprintf(stderr, "%s: unexpected arguments\n", argv[i+1]);
	usage(argv[0]);
	exit(1);
    }
#endif /* MVS */
    
    /*
    * Validate options.
    */
    switch(function)
    {
    case 'c':
    case 'd':
    case 'e':
	if (names == NULL) {
	    fprintf(stderr, "%c: -n option required\n", function);
	    usage(argv[0]);
	    exit(1);
	}
	break;
    }

    switch(function)
    {
    case 'e':
	if (outfile == NULL) {
	    fprintf(stderr, "%c: -o option required\n", function);
	    usage(argv[0]);
	    exit(1);
	}
	break;
    case 'l':
    case 'L':
	indexOnly = 1;
	if (outfile) {
	    fprintf(stderr, "conflicting options: -%c -%c\n", function, 'o');
	    usage(argv[0]);
	    exit(1);
	}
	break;
    }

    /*
    * Read input trace file.
    */
    if (NULL == (cf = (struct cfile *)cf_openIn(tracePath))) {
	perror(tracePath);
	exit(1);
    }
    init(&tables);
    load_prev(tracePath, cf, &tables, indexOnly);
    cf_close(cf);

    /*
    * Do function.
    */
    switch(function)
    {
    case 'c':
	nMatch = 0;
	for (i = 0; i < tables.mems.iMemberCount; ++i) {
	    if (patMatch(names, tables.mems.members[i].pName, deselect)) {
		++nMatch;
		tables.mems.members[i].iCost = cost;
	    }
	}
	if (nMatch == 0) {
	    fprintf(stderr, "%s: no matches\n", names);
	    exit(1);
	}
	break;
    case 'd':
	nMatch = 0;
	for (i = 0; i < tables.mems.iMemberCount; ++i) {
	    if (patMatch(names, tables.mems.members[i].pName, deselect)) {
		++nMatch;
		tables.mems.members[i].iDelete = 1;
	    }
	}
	if (nMatch == 0) {
	    fprintf(stderr, "%s: no matches\n", names);
	    exit(1);
	}
	break;
    case 'e':
	nMatch = 0;
	for (i = 0; i < tables.mems.iMemberCount; ++i) {
	    if (patMatch(names, tables.mems.members[i].pName, deselect)) {
		++nMatch;
	    } else {
		tables.mems.members[i].iDelete = 1;
	    }
	}
	if (nMatch == 0) {
	    fprintf(stderr, "%s: no matches\n", names);
	    exit(1);
	}
	break;
    case 'l':
	nMatch = 0;
	for (i = 0; i < tables.mems.iMemberCount; ++i) {
	    if (patMatch(names, tables.mems.members[i].pName, deselect)) {
		columns(tables.mems.members[i].pName);
		++nMatch;
	    }
	}
	if (nMatch) {
	    columnsEnd();
	} else {
	    if (names != NULL) {
		fprintf(stderr, "%s: no matches\n", names);
	    } else {
		fprintf(stderr, "no tests\n");
	    }
	    exit(1);
	}
#ifndef MVS
	free(tracePath);
#endif /* MVS */
	return 0;
    case 'L':
	nMatch = 0;
	for (i = 0; i < tables.mems.iMemberCount; ++i) {
	    if (patMatch(names, tables.mems.members[i].pName, deselect)) {
		printf("%17.17s %5.5s %4d %c%c--- %s\n", 
		       tables.mems.members[i].pDate,
		       tables.mems.members[i].pVersion,
		       tables.mems.members[i].iCost,
		       tables.mems.members[i].iFreqFlag ? '-' : 'f',
		       tables.mems.members[i].iCorrupted ? 'c' : '-',
		       tables.mems.members[i].pName);
		++nMatch;
	    }
	}
	if (nMatch == 0 && names != NULL) {
	    fprintf(stderr, "%s: no matches\n", names);
	}
#ifndef MVS
	free(tracePath);
#endif /* MVS */
	return 0;
    case 'r':
	{
	    int		nMatch2;
	    int		nextN;
	    char	**oldname;
	    int		i2;

	    nMatch2 = 0;
	    nextN = testNo(&tables.mems, newName);
	    for (i2 = 0; i2 < tables.mems.iMemberCount; ++i2) {
		oldname = &tables.mems.members[i2].pName;
		if (patMatch(names, *oldname, deselect)) {
		    ++nMatch2;
		    free(*oldname);
		    *oldname = (char *)malloc(strlen(newName) + 11);
		    CHECK_MALLOC(oldname);
		    sprintf(*oldname, "%s.%d", newName, nextN++);
		}
	    }
	    if (nMatch2 == 0) {
		fprintf(stderr, "%s: no matches\n", names);
		exit(1);
	    }
	}
	break;
    }

    if (names) free(names);
	
    /*
    * Create temporary output trace file name.
    */
    if (outfile == NULL) {
#ifdef MVS
	outfile = "DD:ATACTOUT";
#else /* MVS */
	outfile = prefixFileName(tracePath, "!");
    } else {
	free(tracePath);
	tracePath = NULL;
#endif
    }

    /*
    * Open output trace file.
    */
    if (NULL == (cf = (struct cfile *)cf_openOut(outfile))) {
	fprintf(stderr, "%s: can't open\n", outfile);
	exit(1);
    }

    /*
    * Write output trace.
    */
    dump(cf, &tables);
    cf_close(cf);

#ifndef MVS
    /*
    * Replace input trace file with temporary output trace file.
    */
    if (tracePath) {
	if (rename(outfile, tracePath) != 0) {
	    perror(tracePath);
	}
	unlink(outfile);
	free(outfile);
	free(tracePath);
    }
#endif /* MVS */

    return 0;
}

#ifndef MVS
static int
exists(path)
char *path;
{
    struct stat buf;

    if (stat(path, &buf) == 0) return 1;
    else return 0;
}
#endif /* MVS */

#if !HAVE_RENAME
static int
rename(from, to)
char	*from;
char	*to;
{
    char	*backup;

    backup = prefixFileName(to, "%");
    if (link(to, backup) != 0) return -1;
    if (unlink(to) != 0) return -1;
    if (link(from, to) != 0) {
	fprintf(stderr, "backup saved in %s\n", backup);
	return -1;
    }
    unlink(backup);
    free(backup);
    return unlink(from);
}
#endif
