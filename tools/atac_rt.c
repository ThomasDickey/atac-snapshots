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
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef MVS
#include <mvapts.h>
MODULEID(%M%,%J%/%D%/%T%)
#endif /* MVS */

static const char atac_rt_c[] = "$Id: atac_rt.c,v 3.19 2013/12/08 21:26:51 tom Exp $";

/*
* @Log: atac_rt.c,v @
* Revision 3.18  2008/12/17 01:24:59  tom
* convert to ANSI, indent'd
*
* Revision 3.17  2005/08/14 13:57:41  tom
* gcc warning
*
* Revision 3.16  1998/08/23 22:01:18  tom
* moved timestamp code into 'write_timestamp()', documented Y2K impact (none).
*
* Revision 3.15  1997/12/10 11:19:23  tom
* simplified/corrected ifdef's for atexit() vs on_exit()
*
* Revision 3.14  1997/11/01 16:05:34  tom
* Linux's atexit() expects void function.
*
* Revision 3.13  1997/07/17 18:32:53  tom
* fix missing return values
*
* Revision 3.12  1996/12/02 00:57:17  tom
* gcc warnings (missing prototypes)
*
* Revision 3.11  1995/12/27 23:36:49  tom
* fix gcc warnings (missing prototypes)
*
* Revision 3.10  94/07/05  16:03:23  saul
* Fix atac_child to use when fork() is detected after the fact.
*
* Revision 3.9  94/07/05  15:51:00  saul
* Added aTaC_fork to deal with fork().
*
* Revision 3.8  94/07/01  13:13:38  saul
* Add test restart feature using atac_restart or ATAC_SIGNAL.
*
* Revision 3.7  94/04/04  10:52:42  jrh
* Add Release Copyright
*
* Revision 3.6  93/08/04  15:50:10  ewk
* Added MVS and solaris support.  Squelched some ANSI warnings.
*
* Revision 3.5  93/07/12  15:43:59  ewk
* Applied modifications for MARCH (includes CSAM).
*
* Revision 3.4  93/07/09  14:13:15  saul
* changed for VMS
*
* Revision 3.3  93/06/30  15:38:31  saul
* Experimental ATAC_COMPRESS feature.
*
* Revision 3.2  93/01/18  08:48:06  saul
* Add SCCS-what/RCS-ident string
*
* Revision 3.1  92/11/11  07:07:43  saul
* #ifdefs expecting atexit when not available
*
* Revision 3.0  92/11/06  07:46:28  saul
* propagate to version 3.0
*
* Revision 2.10  92/11/02  11:39:44  saul
* let portable.h determine "at end" procedures
*
* Revision 2.9  92/10/30  09:42:27  saul
* include portable.h
*
* Revision 2.8  92/10/28  09:14:52  saul
* removed defined() macro for portability
*
* Revision 2.7  92/10/05  10:41:51  saul
* Redefine exit for systems without atexit and on_exit
*
* Revision 2.6  92/09/30  11:35:34  saul
* #elif not portable.  No compress option.  Rename some variables.
*
* Revision 2.5  92/09/22  15:26:56  saul
* Fix source file list errors.  Store fileId per file not per function.
*
* Revision 2.4  92/09/08  10:31:24  saul
* On exit processing and new data structures for freq counts.
*
* Revision 2.3  92/04/07  09:19:08  saul
* quick fix for asynch interupts
*
* Revision 2.2  92/03/17  15:34:48  saul
* copyright
*
* Revision 2.1  91/06/19  13:56:26  saul
* Propagte to version 2.0
*
* Revision 1.1  91/06/12  16:30:23  saul
* Aug 1990 baseline
*
*-----------------------------------------------end of log
*/

/*
* SCCS-what/RCS-ident strings.  '\044' hides '$' from RCS source control.
*/
static const char ident[] = "\044Log: @(#)ATAC runtime\044";

/*
* Includes.
*/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#if HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifndef MVS
#ifndef vms
#include <fcntl.h>
#endif /* vms */
#endif /* MVS */
#include <time.h>
#include <ctype.h>
#include <errno.h>		/* unix end processing */
#include <signal.h>		/* for setupSignal() */

#if HAVE_SYS_WAIT_H
#include <sys/types.h>
#include <sys/wait.h>
#endif

#include "portable.h"
#include "version.h"

/*
* Constants.
*/
#define FILENAME_SIZE	1024
#define PUSE_STACKMAX	100

#define VAR_VOID	0	/* expression value is not used */
#define VAR_DEF		1	/* expression is an lvalue */
#define VAR_CUSE	2	/* expression is part of a computation */
#define VAR_PUSE	4	/* expression participates in branch decision */
#define VAR_DREF	8	/* expression is dereferenced as a pointer */

#define NULL_BLK	-1	/* Blk numbers must be signed to recog. this. */

#define IGNORE_LEVEL	-1	/* ignore functions at this level */

#define INTERMEDIATE_FLUSH	0	/* arg to atac_flush */
#define FINAL_FLUSH	1	/* arg to atac_flush */

/*
* Structure definitions.
*/

typedef struct {
    struct du *du;
    short blk;
} PDEF;

typedef struct path {
    short def_blk;
    short use_blk;		/* needed for p-uses */
    unsigned int count;
    struct path *next;
} PATH;

typedef struct du {
    unsigned short var;		/* index into var[] */
    unsigned short type;	/* VAR_DEF, VAR_PUSE, VAR_CUSE */
    PATH *path;			/* list of defs reaching this use */
} DU;

typedef struct {
    char *name;
    int stamp;
} FILESTAMP;

typedef struct du_table {
    FILESTAMP *files;		/* Array of source file names/stamps */
    char version[2 * sizeof(char *)];	/* version string */
    unsigned short fileId;
    unsigned short funcno;	/* Function number in source file */
    unsigned short nblk;	/* number of entries in blk[] */
    unsigned short nvar;	/* number of entries in var[] */
    DU **blk;
    struct du_table *next;	/* Link pointer. */
    unsigned int *blkCounts;	/* block visitation counts */
} DU_TABLE;

typedef struct Context {
    int funcno;
    struct Context *prev_context;
    DU **blk;
    unsigned int *blkCounts;
    short *deflist;
    unsigned short fileId;
    PDEF *p_use;
    int p_use_count;
    int prev_blk;
} DU_CONTEXT;

/*
* Static data.
*/

static DU_CONTEXT *context_stack = NULL;
static DU_CONTEXT dummy_context =
{-1, 0, 0, 0, 0, 0, 0, 0, 0};
static DU_CONTEXT *context = &dummy_context;

static DU_TABLE dummyTable;
static DU_TABLE *list_of_tables = &dummyTable;

static char traceName[FILENAME_SIZE];
static FILE *f = NULL;

static int fileId = 0;

static int restartFlag = 0;
static char *restartTestName = NULL;

/* forward declarations */
extern int aTaC(int level, int blk);
extern int aTaC_fork(void);
extern int atac_child(void);
extern void aTaC_dump(void);
extern void atac_restart(char *testName);
static FILE *opentrace(void);
static RETSIGTYPE sigHandler(int n);
static char *getTestName(void);
static int add_path(DU * use, int def_blk, int use_blk);
static int atac_flush(FILE *fp, int final);
static int atac_zero(void);
static int redoFileNames(FILE *fp);
static void prepareEnd(void);
static void setupSignal(void);
static void write_timestamp(void);

static int
atac_flush(FILE *fp,
	   int final)
{
    DU_TABLE *table;
    int blk;
    int count;
    DU *du;
    DU *du_limit;
    PATH *p;
    int type;
    int var;
    int file_id;
    int funcNo;
    int nblk;

    for (table = list_of_tables; table != &dummyTable; table = table->next) {
	file_id = table->fileId;
	funcNo = table->funcno;
	nblk = table->nblk;
	for (blk = 0; blk < nblk; ++blk) {
	    count = table->blkCounts[blk] - 1;
	    table->blkCounts[blk] = 0;
	    if (count > 0) {
		fprintf(fp, "b %d %d %d %d\n",
			file_id, funcNo, blk, count);
	    }
	    du = table->blk[blk];
	    du_limit = table->blk[blk + 1];
	    for (; du < du_limit; ++du) {
		type = du->type;
		var = du->var;
		for (p = du->path; p != NULL; p = p->next) {
		    count = p->count - 1;
		    p->count = 0;
		    if (count > 0) {
			if (type & VAR_CUSE) {
			    fprintf(fp, "c %d %d %d %d %d %d\n",
				    file_id, funcNo, var,
				    p->def_blk, blk, count);
			} else if (type & VAR_PUSE) {
			    fprintf(fp, "p %d %d %d %d %d %d %d\n",
				    file_id, funcNo, var,
				    p->def_blk, p->use_blk, blk, count);
			}
		    }
		}
	    }
	}
    }

    if (final != INTERMEDIATE_FLUSH)
	fprintf(fp, "f 0\n");

    return 0;
}

static int
atac_zero(void)
{
    DU_TABLE *table;
    int blk;
    DU *du;
    DU *du_limit;
    PATH *p;
    int nblk;

    for (table = list_of_tables; table != &dummyTable; table = table->next) {
	nblk = table->nblk;
	for (blk = 0; blk < nblk; ++blk) {
	    table->blkCounts[blk] = 0;
	    du = table->blk[blk];
	    du_limit = table->blk[blk + 1];
	    for (; du < du_limit; ++du) {
		for (p = du->path; p != NULL; p = p->next) {
		    p->count = 0;
		}
	    }
	}
    }

    return 0;
}

#undef END_PROCESSING

/* 'atexit()' is ANSI - 'on_exit()' is not, but we only want one */
#ifdef HAVE_ATEXIT
#define END_PROCESSING
#undef HAVE_ON_EXIT
#undef _EXIT_SUPPORT
#endif /*  HAVE_ATEXIT */

#ifdef HAVE_ON_EXIT
#define END_PROCESSING
#undef HAVE_ATEXIT
#undef _EXIT_SUPPORT
#endif /*  HAVE_ON_EXIT */

#ifdef _EXIT_SUPPORT
#define END_PROCESSING
#endif /*  _EXIT_SUPPORT */

#ifndef FORK_SUPPORT
#undef END_PROCESSING
#undef HAVE_ON_EXIT
#undef HAVE_ATEXIT
#undef _EXIT_SUPPORT
#endif

#ifdef END_PROCESSING
#if HAVE_ATEXIT
static void
aTaC_cleanup(void)
#else /* using 'on_exit()' */
static void
aTaC_cleanup(int status, char *arg)
#endif
{
    int pid;
    int wpid;
    char *compress;
    int nthCompress;

    if (f) {
	atac_flush(f, FINAL_FLUSH);
	fclose(f);
    }
#ifdef FORK_SUPPORT
    if (getenv("ATAC_NOCOMPRESS") != NULL)
	return;

    compress = getenv("ATAC_COMPRESS");
    if (compress != NULL) {
	nthCompress = atoi(compress);
	if (nthCompress <= 0)
	    return;
	if ((time(0) % nthCompress) != 0)
	    return;
    }

    /*
       * Run atactm to compress trace.  Return when done.
     */
    pid = fork();
    if (pid == 0) {
	close(0);
	open("/dev/null", O_RDONLY, 0);		/* 0 (stdin) */
	close(1);
	open("/dev/null", O_WRONLY, 0);		/* 1 (stdout) */
	close(2);
	open("/dev/null", O_WRONLY, 0);		/* 2 (stderr) */
	execlp("atactm", "atactm", traceName, NULL);
	_exit(1);
    } else {
	while ((wpid = wait(0)) != pid) {
	    if (wpid == -1 && errno != EINTR)
		break;
	}
    }
#endif /* FORK_SUPPORT */
}
#endif /* END_PROCESSING */

static void
prepareEnd(void)
{
#ifdef HAVE_ATEXIT
    atexit(aTaC_cleanup);
#else /* no HAVE_ATEXIT */
#ifdef HAVE_ON_EXIT
    on_exit(aTaC_cleanup, 0);
#endif /* HAVE_ON_EXIT */
#endif /* no HAVE_ATEXIT */
    return;
}

#ifndef MARCH
#ifdef _EXIT_SUPPORT
#ifndef HAVE_ATEXIT
#ifndef HAVE_ON_EXIT
exit(int status)
{
    aTaC_cleanup();
    _cleanup();
    _exit(status);
}
#endif /* no HAVE_ON_EXIT */
#endif /* no HAVE_ATEXIT */
#endif /* _EXIT_SUPPORT */
#endif /* MARCH */

static int
redoFileNames(FILE *fp)
{
    DU_TABLE *table;
    FILESTAMP *filestamp;
    int nFiles;
    int i;

    for (table = list_of_tables; table != &dummyTable; table = table->next) {
	filestamp = table->files;
	if (filestamp->name != NULL)
	    break;
	filestamp->name = "";	/* marker */
	nFiles = filestamp->stamp >> 16;
	fprintf(fp, "s %d %s %d %s\n", table->fileId, filestamp[nFiles].name,
		filestamp[nFiles].stamp, table->version);
	for (i = 1; i < nFiles; ++i)
	    fprintf(fp, "h %s %d\n", filestamp[i].name, filestamp[i].stamp);
    }

    for (table = list_of_tables; table != &dummyTable; table = table->next) {
	filestamp = table->files;
	filestamp->name = NULL;
    }

    return 0;
}

static char *
getTestName(void)
{
    char *testName;
    char *p;

    testName = restartTestName;
    if (testName == NULL) {
	testName = getenv("ATAC_TEST");
	if (testName == NULL)
	    return "t";
    }

    for (p = testName; *p; ++p) {
	if (!isalnum(*p) && *p != '_') {
	    return "E";
	}
    }

    return testName;
}

/*
 * Write timestamp and version, used later for accounting purposes.  ATAC does
 * not compare dates except to see if they are equal, so there is no need to
 * make a new file-format version with a 4-digit year.
 */
static void
write_timestamp(void)
{
    time_t now;
    struct tm *tm;

    now = time((time_t *) 0);
    tm = (struct tm *) localtime(&now);
    fprintf(f, "t %.2d/%.2d/%.2d-%.2d:%.2d:%.2d",
	    tm->tm_mon + 1,
	    tm->tm_mday,
	    tm->tm_year,
	    tm->tm_hour,
	    tm->tm_min,
	    tm->tm_sec);
    fprintf(f, " %s %s\n", VERSION, getTestName());
}

int				/* returns call level */
aTaC(int level,
     int blk)
{
    static int call_level = -1;
    static int busy = 0;
    PDEF *pdef;
    DU *du;
    DU *du_limit;
    int def_blk;
    int i;
    FILESTAMP *filestamp;
#ifndef NOFLUSH
    int flush;
#endif

    if (busy)
	return IGNORE_LEVEL;
    busy = 1;

    if (restartFlag != 0 && f != NULL) {
	atac_flush(f, FINAL_FLUSH);
	write_timestamp();
	redoFileNames(f);
	restartFlag = 0;
    }

    if (level != call_level) {
	/*
	 * First call for this invocation of this function.
	 */
	if (level == 0) {
	    DU_TABLE *table;

	    /*
	     * One time initializations.
	     */
	    if (f == NULL) {
		prepareEnd();
		setupSignal();
		f = opentrace();
		if (f == NULL)
		    exit(1);
		write_timestamp();
		call_level = 0;
		restartFlag = 0;
	    }

	    table = (DU_TABLE *) blk;
	    blk = 0;

	    /*
	     * First entry to code in this source file.
	     */
	    filestamp = table->files;
	    if (filestamp->name) {
		int n = 1;
		while (filestamp[n].name != NULL)
		    ++n;
		table->fileId = fileId++;
		fprintf(f, "s %d %s %d %s\n",
			table->fileId, filestamp->name,
			filestamp->stamp, table->version);
		/*
		 * Kludges: CLEAN THIS UP!
		 * filestamp[0].name is used as a flag that
		 * indicates that a function in this file
		 * has been entered before.
		 * filestamp[0].stamp is used to store the
		 * fileId.
		 */
		filestamp[n].name = filestamp[0].name;
		filestamp[n].stamp = filestamp[0].stamp;
		filestamp->name = NULL;
		filestamp->stamp = (n << 16) | table->fileId;
		if (strcmp(table->version, RT_VERSION) != 0) {
		    return IGNORE_LEVEL;
		}
		while (++filestamp < table->files + n)
		    fprintf(f, "h %s %d\n", filestamp->name,
			    filestamp->stamp);
	    } else
		table->fileId = filestamp->stamp & 0xFFFF;

	    /*
	       * Push prev context on stack.
	     */
	    context->prev_context = context_stack;
	    context_stack = context;
	    ++call_level;

	    /*
	       * Create new context.
	     */
	    context = (DU_CONTEXT *) malloc(
					       sizeof *context +
					       (PUSE_STACKMAX * sizeof *context->p_use)
					       + (table->nvar * sizeof *context->deflist));
	    if (context == NULL) {
		fprintf(f,
			"%d %d=>can't alloc context level %d\n",
			table->fileId, (int) table->funcno,
			call_level);
		fflush(f);
		exit(1);
	    }
	    context->funcno = table->funcno;
	    context->fileId = table->fileId;
	    context->blk = table->blk;
	    context->blkCounts = table->blkCounts;
	    context->p_use = (PDEF *) & context[1];
	    context->p_use_count = 0;
	    context->deflist = (short *)
		&context->p_use[PUSE_STACKMAX];
	    for (i = 0; i < table->nvar; ++i)
		context->deflist[i] = NULL_BLK;
	    /*
	     * Put table on linked list of functions for
	     * "atac_flush".
	     */
	    if (table->next == NULL) {
		table->next = list_of_tables;
		list_of_tables = table;
	    }
	}

	/*
	 * Level is the value returned by the first call to aTaC()
	 * (i.e. with level=0), from the current invocation of the
	 * calling function.  Call_level is the value of level on the
	 * previous call of aTaC().  The static variable "context"
	 * points to the most recent context.  If level != call_level
	 * a function must have returned (or longjmp() was called)
	 * since the previous call of aTaC().  In this case we pop
	 * preceeding contexts off the stack and free them down to the
	 * one for the current call level.
	 */
	else
	    do {
		if (level == IGNORE_LEVEL)
		    return -1;
#ifdef RETURNPATHS
		for (i = context->p_use_count; i > 0; --i) {
		    pdef = &context->p_use[i - 1];
		    if (add_path(pdef->du, pdef->blk, 0)) {
			fprintf(f, "p %d %d %d %d %d 0 1\n",
				context->fileId,
				context->funcno,
				pdef->du->var,
				pdef->blk, context->prev_blk);
#ifndef NOFLUSH
			flush = 1;
#endif
		    }
		}
#endif
		free(context);
		/*
		 * Pop stack.
		 */
		context = context_stack;
		if (context == &dummy_context) {
		    fprintf(stderr,
			    "aTaC: can't find context\n");
		    fflush(f);
		    exit(1);
		}
		context_stack = context->prev_context;
		--call_level;
	    } while (level != call_level);
    }
#ifndef NOFLUSH
    flush = 0;
#endif
    for (i = context->p_use_count; i > 0; --i) {
	pdef = &context->p_use[i - 1];
	if (add_path(pdef->du, pdef->blk, blk)) {
	    fprintf(f, "p %d %d %d %d %d %d 1\n",
		    context->fileId, context->funcno,
		    pdef->du->var,
		    pdef->blk, context->prev_blk, blk);
#ifndef NOFLUSH
	    flush = 1;
#endif
	}
    }
    context->p_use_count = 0;

    if (context->blkCounts[blk]++ == 0) {
	fprintf(f, "b %d %d %d 1\n",
		context->fileId, context->funcno, blk);
#ifndef NOFLUSH
	flush = 1;
#endif
    }
    du = context->blk[blk];
    du_limit = context->blk[blk + 1];
    for (; du < du_limit; ++du) {
	def_blk = context->deflist[du->var];
	if (du->type & VAR_CUSE) {
	    if (def_blk != NULL_BLK) {
		if (add_path(du, def_blk, blk)) {
		    fprintf(f, "c %d %d %d %d %d 1\n",
			    context->fileId,
			    context->funcno,
			    du->var,
			    def_blk, blk);
#ifndef NOFLUSH
		    flush = 1;
#endif
		}
	    }
#ifdef UNINIT_USE
	    else {
		if (add_path(du, def_blk, blk)) {
		    fprintf(f, "c %d %d %d ? %d 1\n",
			    context->fileId,
			    context->funcno,
			    du->var,
			    blk);
#ifndef NOFLUSH
		    flush = 1;
#endif
		}
	    }
#endif
	}
	if (du->type & VAR_DEF) {
	    context->deflist[du->var] = blk;
	    def_blk = blk;
	}
	if (du->type & VAR_PUSE) {
	    if (def_blk != NULL_BLK) {
		if (context->p_use_count == PUSE_STACKMAX)
		    continue;
		pdef = context->p_use + context->p_use_count++;
		pdef->du = du;
		pdef->blk = def_blk;
		context->prev_blk = blk;
	    }
#ifdef UNINIT_USE
	    else {
		if (add_path(du, def_blk, blk)) {
		    fprintf(f, "p %d %d %d ? %d %d 1\n",
			    context->fileId,
			    context->funcno,
			    du->var,
			    context->prev_blk, blk);
#ifndef NOFLUSH
		    flush = 1;
#endif
		}
	    }
#endif
	}
    }
#ifndef NOFLUSH
    if (flush)
	fflush(f);
#endif

    busy = 0;

    return call_level;
}

#define SIZE_PATH_POOL	256

static int			/* return 0 if found */
add_path(
	    DU * use,
	    int def_blk,
	    int use_blk)
{
    PATH *p;
    static PATH *path_pool = NULL;
    static int size_path_pool = 0;

    for (p = use->path; p != NULL; p = p->next) {
	if ((p->def_blk == def_blk) && (p->use_blk == use_blk)) {
	    if (p->count++ == 0)
		return 1;
	    else
		return 0;
	}
    }

    /*
       * Limit calls to malloc by keeping a pool.
     */
    if (size_path_pool--)
	p = path_pool++;
    else {
	p = (PATH *) malloc(SIZE_PATH_POOL * sizeof *p);
	if (p == NULL)
	    return 1;
	path_pool = p + 1;
	size_path_pool = SIZE_PATH_POOL - 1;
    }

    p->def_blk = def_blk;
    p->use_blk = use_blk;
    p->count = 1;
    p->next = use->path;
    use->path = p;
    return 1;
}

void
aTaC_dump(void)
{
    int j;
    DU_CONTEXT *cp;

    j = 0;
    fprintf(stderr, "%d: %d %d\n", j++, context->fileId, context->funcno);
    for (cp = context_stack; cp != NULL; cp = cp->prev_context) {
	fprintf(stderr, "%d: %d %d\n", j++, cp->fileId, cp->funcno);
    }
}

/*
* opentrace:  Open an output stream for trace.
*	If an environment variable named ATAC_DIR has a value it is used
*	as the trace directory, otherwise the current directory is used.
*	If an environment variable named ATAC_TRACE has a value it is used
*	as the trace file name, otherwise the name linked into aTaC_trace
*	is used.
*/
static FILE *
opentrace(void)
{
    FILE *trace;
    char *envval;
#ifdef MVS
    static char aTaC_trace[] = "DD:ATACTRCE";
#else /* not MVS */
    extern char *aTaC_trace;
#endif

    envval = getenv("ATAC_DIR");
    if (envval && *envval) {
	strcpy(traceName, envval);
#ifdef unix
	if (traceName[strlen(traceName) - 1] != '/')
	    strcat(traceName, "/");
#endif /* unix */
    } else
	traceName[0] = '\0';

    envval = getenv("ATAC_TRACE");
    if (envval && *envval) {
#ifdef unix
	if (*envval == '/')
	    strcpy(traceName, envval);
	else
#endif /* unix */
#ifdef vms
	if (index(*envval, '[') != NULL || index(*envval, ':') != NULL)
	    strcpy(traceName, envval);
	else
#endif /* vms */
	    strcat(traceName, envval);
#ifdef vms
    } else
	strcat(traceName, "atac.trace");
#else /* not vms */
    } else
	strcat(traceName, aTaC_trace);
#endif /* not vms */

    trace = fopen(traceName, "a");
    if (trace == NULL) {
	fprintf(stderr, "atac runtime can't write trace file: %s\n",
		traceName);
	return NULL;
    }

    return trace;
}

void
atac_restart(char *testName)
{
    if (testName != NULL)
	restartTestName = testName;

    restartFlag = 1;
}

static RETSIGTYPE
sigHandler(int n)
{
    restartFlag = 1;
#if 0				/* FIXME */
    return 0;
#endif
}

static void
setupSignal(void)
{
    char *envsig;
    size_t i;
    /* *INDENT-OFF* */
    static struct signames {
	char	*name;
	int	number;
    } signames[] = {
#ifdef SIGHUP
	{"HUP",		SIGHUP},
	{"hup",		SIGHUP},
#endif /* HUP */
#ifdef SIGINT
	{"INT",		SIGINT},
	{"int",		SIGINT},
#endif /* INT */
#ifdef SIGQUIT
	{"QUIT",	SIGQUIT},
	{"quit",	SIGQUIT},
#endif /* QUIT */
#ifdef SIGILL
	{"ILL",		SIGILL},
	{"ill",		SIGILL},
#endif /* ILL */
#ifdef SIGTRAP
	{"TRAP",	SIGTRAP},
	{"trap",	SIGTRAP},
#endif /* TRAP */
#ifdef SIGIOT
	{"IOT",		SIGIOT},
	{"iot",		SIGIOT},
#endif /* IOT */
#ifdef SIGEMT
	{"EMT",		SIGEMT},
	{"emt",		SIGEMT},
#endif /* EMT */
#ifdef SIGFPE
	{"FPE",		SIGFPE},
	{"fpe",		SIGFPE},
#endif /* FPE */
#ifdef SIGKILL
	{"KILL",	SIGKILL},
	{"kill",	SIGKILL},
#endif /* KILL */
#ifdef SIGBUS
	{"BUS",		SIGBUS},
	{"bus",		SIGBUS},
#endif /* BUS */
#ifdef SIGSEGV
	{"SEGV",	SIGSEGV},
	{"segv",	SIGSEGV},
#endif /* SEGV */
#ifdef SIGSYS
	{"SYS",		SIGSYS},
	{"sys",		SIGSYS},
#endif /* SYS */
#ifdef SIGPIPE
	{"PIPE",	SIGPIPE},
	{"pipe",	SIGPIPE},
#endif /* PIPE */
#ifdef SIGALRM
	{"ALRM",	SIGALRM},
	{"alrm",	SIGALRM},
#endif /* ALRM */
#ifdef SIGTERM
	{"TERM",	SIGTERM},
	{"term",	SIGTERM},
#endif /* TERM */
#ifdef SIGURG
	{"URG",		SIGURG},
	{"urg",		SIGURG},
#endif /* URG */
#ifdef SIGSTOP
	{"STOP",	SIGSTOP},
	{"stop",	SIGSTOP},
#endif /* STOP */
#ifdef SIGTSTP
	{"TSTP",	SIGTSTP},
	{"tstp",	SIGTSTP},
#endif /* TSTP */
#ifdef SIGCONT
	{"CONT",	SIGCONT},
	{"cont",	SIGCONT},
#endif /* CONT */
#ifdef SIGCHLD
	{"CHLD",	SIGCHLD},
	{"chld",	SIGCHLD},
#endif /* CHLD */
#ifdef SIGTTIN
	{"TTIN",	SIGTTIN},
	{"ttin",	SIGTTIN},
#endif /* TTIN */
#ifdef SIGTTOU
	{"TTOU",	SIGTTOU},
	{"ttou",	SIGTTOU},
#endif /* TTOU */
#ifdef SIGIO
	{"IO",	SIGIO},
	{"io",	SIGIO},
#endif /* IO */
#ifdef SIGXCPU
	{"XCPU",	SIGXCPU},
	{"xcpu",	SIGXCPU},
#endif /* XCPU */
#ifdef SIGXFSZ
	{"XFSZ",	SIGXFSZ},
	{"xfsz",	SIGXFSZ},
#endif /* XFSZ */
#ifdef SIGVTALRM
	{"VTALRM",	SIGVTALRM},
	{"vtalrm",	SIGVTALRM},
#endif /* VTALRM */
#ifdef SIGPROF
	{"PROF",	SIGPROF},
	{"prof",	SIGPROF},
#endif /* PROF */
#ifdef SIGWINCH
	{"WINCH",	SIGWINCH},
	{"winch",	SIGWINCH},
#endif /* WINCH */
#ifdef SIGUSR1
	{"USR1",	SIGUSR1},
	{"usr1",	SIGUSR1},
#endif /* USR1 */
#ifdef SIGUSR2
	{"USR2",	SIGUSR2},
	{"usr2",	SIGUSR2},
#endif /* USR2 */
	};
    /* *INDENT-ON* */

    envsig = getenv("ATAC_SIGNAL");
    if (envsig == NULL)
	return;

    if (atoi(envsig) != 0) {
	signal(atoi(envsig), sigHandler);
	return;
    }

    if (strncmp(envsig, "SIG", 3) == 0 || strncmp(envsig, "sig", 3) == 0)
	envsig += 3;

    for (i = 0; i < sizeof signames / sizeof *signames; ++i) {
	if (strcmp(envsig, signames[i].name) == 0) {
	    signal(signames[i].number, sigHandler);
	    return;
	}
    }
}

int
atac_child(void)
{
    atac_zero();

    fclose(f);
    sprintf(traceName + strlen(traceName), ".%d", getpid());

    f = fopen(traceName, "a");
    if (f == NULL)
	fprintf(stderr, "atac runtime can't write trace file: %s\n",
		traceName);

    write_timestamp();
    redoFileNames(f);
    restartFlag = 0;

    return 0;
}

int
aTaC_fork(void)
{
    int pid;

    atac_flush(f, INTERMEDIATE_FLUSH);	/* In case exec() comes next. */

    pid = fork();

    if (pid != 0)
	return pid;

    atac_child();

    return 0;
}
