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

#include <stdio.h>

#include "portable.h"
#include "atacysis.h"
#include "disp.h"

static char const ddisp_c[] =
	"$Header: /users/source/archives/atac.vcs/atacysis/RCS/ddisp.c,v 3.6 1995/12/27 20:25:39 tom Exp $";
/*
* $Log: ddisp.c,v $
* Revision 3.6  1995/12/27 20:25:39  tom
* adjust headers, prototyped for autoconfig
*
* Revision 3.5  94/04/04  10:25:01  jrh
* Add Release Copyright
* 
* Revision 3.4  94/01/31  14:21:35  saul
* Add disp_title interface to standardize display headings.
* 
* Revision 3.3  93/11/02  11:52:48  saul
* Same as revision 3.1
* 
* Revision 3.1  93/08/04  15:53:07  ewk
* Added MVS and solaris support.  Squelched some ANSI warnings.
* 
* Revision 3.0  92/11/06  07:48:00  saul
* propagate to version 3.0
* 
* Revision 2.8  92/10/30  09:54:02  saul
* include portable.h
* 
* Revision 2.7  92/10/29  11:15:17  saul
* Use window size.
* 
* Revision 2.6  92/09/08  09:08:04  saul
* New coverage vector data struture.  General display cleanup.
* 
* Revision 2.5  92/07/10  11:11:29  saul
* New TRUE/FALSE decision display
* 
* Revision 2.4  92/03/17  15:27:01  saul
* copyright
* 
* Revision 2.3  91/12/13  09:27:19  saul
* same length file name bug
* 
* Revision 2.2  91/06/19  13:15:16  saul
* jrh: print message when no decisions present.
* 
* Revision 2.1  91/06/19  13:09:56  saul
* Propagate to version 2.0
* 
* Revision 1.1  91/06/12  16:58:18  saul
* Aug 1990 baseline
* 
*-----------------------------------------------end of log
*/

#define SRC_CONTEXT_LINES	4

#define MAX_HEADER		(MAX_SRCFILE_NAME + 50)

#define MAX(a,b) ((a)>(b)?(a):(b))

typedef int	RLIST;

typedef struct {
	RLIST	**rlist;
	int	notcov;
	int	tot;
} COV_LIST;

/* forward declarations */
static void dispDecis
	P_((char *srcfile, int start_line, int end_line, SE_POSITION *to,
	char *t_str));
static void display
	P_((T_FILE *file, char *atacfile, char *funcname, SE_POSITION *to,
	SE_POSITION *func, char *value, int displayMode));
static void print_header
	P_((char *srcfile, char *func, int line, int displayMode));

void
ddisp(modules, n_mod, covVector, displayMode)
T_MODULE	*modules;
int		n_mod;
int		*covVector;
int		displayMode;
{
    int		i;
    T_FUNC		*func;
    T_MODULE	*mod;
    T_PUSE		*puse;
    int		decis_var;
    int		none = 1; /* 0 if no decis to be printed; 1 otherwise */

    for (mod = modules; mod < modules + n_mod; ++mod) {
	for (func = mod->func; func < mod->func + mod->n_func; ++func) {
	    if (func->ignore) continue;
	    /*
	    * Find and display not covered decisions.
	    */
	    decis_var = func->decis_var;
	    if (decis_var == -1)	/* No decisions at all */
		    continue;
	    for (i = 0; i < (int)func->n_puse; ++i) {
		puse = func->puse + i;
		if (puse->varno != decis_var) break;
		if (covVector[func->pUseCovStart + i] != 0)
		{
		    continue;
		}
		if (puse->use.start.file != puse->use.end.file)
		    continue;
		none = 0;
		display(mod->file + puse->use.start.file,
			mod->atacfile, func->fname, &puse->use, &func->pos,
			puse->value, displayMode & DISPLAY_COV);
	    }
	}
    }
    if (none) {
	switch (displayMode & (DISPLAY_ALL | DISPLAY_COV))
	{
	case DISPLAY_COV | DISPLAY_ALL:
	    disp_str("No covered decisions.", DISP_NEWLINE);
	    break;
	case DISPLAY_ALL:
	    disp_str("No uncovered decisions.", DISP_NEWLINE);
	    break;
	case DISPLAY_COV:
	    disp_str("No covered decisions in covered blocks.", DISP_NEWLINE);
	    break;
	case 0:
	    disp_str("No uncovered decisions in covered blocks.", DISP_NEWLINE);
	    break;
	}
	disp_end();
    }
}

static void
print_header(srcfile, func, line, displayMode)
char	*srcfile;
char	*func;
int	line;
int	displayMode;
{
	char	buf[MAX_HEADER];
	char 	*not;

	if (displayMode & DISPLAY_COV) {
	    not = "";
	} else {
	    not = "not ";
	}

	sprintf(buf, "%s:%s decision %scovered at line %d",
		srcfile, func, not, line);

	disp_title(buf, 0, 0);
}

static void
display(file, atacfile, funcname, to, func, value, displayMode)
T_FILE		*file;
char		*atacfile;
char		*funcname;
SE_POSITION	*to;
SE_POSITION	*func;
char		*value;
int		displayMode;
{
	int		start;
	int		end;
	char		*srcfile;
	char		v[9];
	int		vLen;

	vLen = strlen(value);

	srcfile = srcfile_name(file->filename, &file->chgtime, atacfile);
	start = to->start.line - SRC_CONTEXT_LINES;
	if (start <= 0) start = 1;
	if (func->start.file == to->end.file && start < (int)func->start.line)
	    start = func->start.line;
	end = to->end.line + SRC_CONTEXT_LINES;
	if (func->end.file == to->end.file && end > (int)func->end.line) 
		end = func->end.line;
	print_header(srcfile, funcname, start, displayMode);
	sprintf(v, "%.7s%.*s>", value, MAX(7-vLen,0), "=======");
	dispDecis(srcfile, start, end, to, v);
}

static void
dispDecis(srcfile, start_line, end_line, to, t_str)
char		*srcfile;
int		start_line;
int		end_line;
SE_POSITION	*to;
char		*t_str;
{
	disp_file(srcfile, start_line, 1, to->start.line, 0, DISP_INDENT);
	disp_str(t_str, DISP_HILI);
	if (to->start.col != 1)
		disp_file(srcfile, to->start.line, 1, to->start.line,
			  to->start.col - 1, 0);
	disp_file(srcfile, to->start.line, to->start.col, to->end.line,
		  to->end.col, DISP_HILI);
	disp_file(srcfile, to->end.line, to->end.col + 1, end_line, -1, 0);
	disp_end();
}
