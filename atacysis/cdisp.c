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

static char const cdisp_c[] = "$Id: cdisp.c,v 3.9 2013/12/09 01:03:26 tom Exp $";
/*
* @Log: cdisp.c,v @
* Revision 3.7  1995/12/27 20:26:06  tom
* adjust headers, prototyped for autoconfig
*
*Revision 3.6  94/04/04  10:24:54  jrh
*Add Release Copyright
*
*Revision 3.5  94/01/31  14:21:00  saul
*Add disp_title interface to standardize display headings.
*
*Revision 3.4  93/11/02  11:45:44  saul
*Same as revision 3.2
*
*Revision 3.2  93/08/04  15:52:49  ewk
*Added MVS and solaris support.  Squelched some ANSI warnings.
*
* Revision 3.1  93/01/15  12:05:25  saul
*Don't do "1 lines skipped".
*
*Revision 3.0  92/11/06  07:48:14  saul
*propagate to version 3.0
*
*Revision 2.4  92/10/30  09:53:56  saul
*include portable.h
*
*Revision 2.3  92/10/29  11:15:43  saul
*Use window size.
*New elipsis format.
*
*Revision 2.2  92/09/08  09:07:20  saul
*New coverage vector data struture.  General display cleanup.
*
*Revision 2.1  92/07/10  13:44:50  saul
*new
*
*-----------------------------------------------end of log
*/

#define SRC_CONTEXT_LINES	4

#define MAX_HEADER		(MAX_SRCFILE_NAME + 50)

typedef int RLIST;

typedef struct {
    RLIST **rlist;
    int notcov;
    int tot;
} COV_LIST;

/* forward declarations */
static void same
  (const char *srcfile, int start_line, int end_line, SE_POSITION * blk,
   const char *str);
static void fr_to
  (const char *srcfile, int start_line, int end_line, SE_POSITION * from,
   SE_POSITION * to, const char *str);
static void from_to
  (const char *srcfile, int start_line, int end_line, SE_POSITION * from,
   SE_POSITION * to, const char *f_str, const char *t_str);
static void display
  (T_FILE * file, const char *atacfile, const char *funcname, SE_POSITION * from,
   SE_POSITION * to, SE_POSITION * func, char *var);

void
cdisp(T_MODULE * modules,
      int n_mod,
      int *covVector,
      int displayMode)
{
    int i;
    T_FUNC *func;
    T_MODULE *mod;
    T_CUSE *cuse;
    int none = 1;		/* 1 if no c-use to be printed; 0 otherwise */

    for (mod = modules; mod < modules + n_mod; ++mod) {
	for (func = mod->func; func < mod->func + mod->n_func; ++func) {
	    if (func->ignore)
		continue;
	    /*
	     * Find and display not covered c-uses.
	     */
	    cuse = func->cuse;
	    for (i = 0; i < (int) func->n_cuse; ++i) {
		cuse = func->cuse + i;
		if (covVector[func->cUseCovStart + i] != 0) {
		    continue;
		}
		if (cuse->def.start.file != cuse->def.end.file)
		    continue;
		if (cuse->def.start.file != cuse->use.start.file)
		    continue;
		if (cuse->def.start.file != cuse->use.end.file)
		    continue;
		none = 0;
		display(mod->file + cuse->def.start.file,
			mod->atacfile, func->fname, &cuse->def,
			&cuse->use, &func->pos,
			func->var[cuse->varno].vname);
	    }
	}
    }
    if (none) {
	switch (displayMode & (DISPLAY_ALL | DISPLAY_COV)) {
	case DISPLAY_COV | DISPLAY_ALL:
	    disp_str("No covered c-uses.", DISP_NEWLINE);
	    break;
	case DISPLAY_ALL:
	    disp_str("No uncovered c-uses.", DISP_NEWLINE);
	    break;
	case DISPLAY_COV:
	    disp_str("No covered c-uses in covered blocks.", DISP_NEWLINE);
	    break;
	case 0:
	    disp_str("No uncovered c-uses in covered blocks.", DISP_NEWLINE);
	    break;
	}
	disp_end();
    }
}

static void
print_header(char *srcfile,
	     const char *func,
	     int line,
	     int dline,
	     char *var)
{
    char buf[MAX_HEADER];

    sprintf(buf, "%s:%s C-USE of %s at line %d [def at line %d]",
	    srcfile, func, var, line, dline);
    disp_title(buf, 0, 0);
}

static void
display(T_FILE * file,
	const char *atacfile,
	const char *funcname,
	SE_POSITION * from,
	SE_POSITION * to,
	SE_POSITION * func,
	char *var)
{
    int start;
    int end;
    char *srcfile;
    SE_POSITION overlap;

    srcfile = srcfile_name(file->filename, &file->chgtime, atacfile);
    print_header(srcfile, funcname, to->start.line, from->start.line, var);
    if (from->start.line < to->start.line) {
	start = from->start.line - SRC_CONTEXT_LINES;
	if (start <= 0)
	    start = 1;
	if (start < (int) func->start.line
	    && from->start.line >= func->start.line
	    && func->start.file == to->end.file) {
	    start = func->start.line;
	}
	end = to->end.line + SRC_CONTEXT_LINES;
	if (func->end.file == to->end.file && end > (int) func->end.line)
	    end = func->end.line;
	from_to(srcfile, start, end, from, to, "def====>", "use====>");
    } else if (from->start.line > to->start.line) {
	start = to->start.line - SRC_CONTEXT_LINES;
	if (start <= 0)
	    start = 1;
	if (start < func->start.line
	    && to->start.line >= (int) func->start.line
	    && func->start.file == to->end.file) {
	    start = func->start.line;
	}
	end = from->end.line + SRC_CONTEXT_LINES;
	if (func->end.file == to->end.file && end > (int) func->end.line)
	    end = func->end.line;
	from_to(srcfile, start, end, to, from, "use====>", "def====>");
    } else {			/* same line */
	start = from->start.line - SRC_CONTEXT_LINES;
	if (start <= 0)
	    start = 1;
	if (start < (int) func->start.line
	    && from->start.line >= func->start.line
	    && func->start.file == to->end.file) {
	    start = func->start.line;
	}
	if (from->start.col < to->start.col)
	    end = to->end.line + SRC_CONTEXT_LINES;
	else
	    end = from->end.line + SRC_CONTEXT_LINES;
	if (func->end.file == to->end.file && end > (int) func->end.line)
	    end = func->end.line;
	if (from->end.col < to->start.col)
	    fr_to(srcfile, start, end, from, to, "d/use==>");
	else if (from->start.col > to->end.col)
	    fr_to(srcfile, start, end, to, from, "use/d==>");
	else {
	    if (from->start.col <= to->start.col)
		overlap.start = from->start;
	    else
		overlap.start = to->start;
	    if (from->end.col >= to->end.col)
		overlap.end = from->end;
	    else
		overlap.end = to->end;
	    same(srcfile, start, end, &overlap, "d/use==>");
	}
    }
}

static void
from_to(const char *srcfile,
	int start_line,
	int end_line,
	SE_POSITION * from,
	SE_POSITION * to,
	const char *f_str,
	const char *t_str)
{
    disp_file(srcfile,
	      start_line, 1,
	      from->start.line, 0, DISP_INDENT);
    disp_str(f_str, DISP_HILI);
    if (from->start.col != 1)
	disp_file(srcfile,
		  from->start.line, 1,
		  from->start.line, from->start.col - 1, DISP_INDENT);
    if (from->end.line == to->start.line) {
	disp_file(srcfile,
		  from->start.line, from->start.col,
		  from->end.line, 0, DISP_HILI);
	disp_str(t_str, DISP_HILI);
	disp_file(srcfile,
		  from->end.line, 1,
		  from->end.line, from->end.col, DISP_HILI);
	if (from->end.col != to->start.col)
	    disp_file(srcfile,
		      from->end.line, from->end.col + 1,
		      to->start.line, to->start.col - 1, 0);
    } else {
	disp_file(srcfile,
		  from->start.line, from->start.col,
		  from->end.line, from->end.col, DISP_HILI);
	if ((int) (to->start.line - from->end.line) > 11) {
	    disp_file(srcfile, from->end.line, from->end.col + 1,
		      from->end.line + 4, 0, 0);
	    disp_elipsis(to->start.line - from->end.line - 8);
	    disp_file(srcfile, to->start.line - 4, 1,
		      to->start.line, 0, 0);
	} else {
	    disp_file(srcfile,
		      from->end.line, from->end.col + 1,
		      to->start.line, 0, 0);
	}
	disp_str(t_str, DISP_HILI);
	if (to->start.col != 1)
	    disp_file(srcfile,
		      to->start.line, 1,
		      to->start.line, to->start.col - 1, 0);
    }
    disp_file(srcfile,
	      to->start.line, to->start.col,
	      to->end.line, to->end.col, DISP_HILI);
    disp_file(srcfile,
	      to->end.line, to->end.col + 1,
	      end_line, -1, 0);
    disp_end();
}

static void
fr_to(const char *srcfile,
      int start_line,
      int end_line,
      SE_POSITION * from,
      SE_POSITION * to,
      const char *str)
{
    disp_file(srcfile,
	      start_line, 1,
	      from->start.line, 0, DISP_INDENT);
    disp_str(str, DISP_HILI);
    if (from->start.col != 1)
	disp_file(srcfile,
		  from->start.line, 1,
		  from->start.line, from->start.col - 1, DISP_INDENT);
    disp_file(srcfile,
	      from->start.line, from->start.col,
	      from->end.line, from->end.col, DISP_HILI);
    if (from->end.col < to->start.col - 1)
	disp_file(srcfile,
		  from->end.line, from->end.col + 1,
		  to->start.line, to->start.col - 1, 0);
    disp_file(srcfile,
	      to->start.line, to->start.col,
	      to->end.line, to->end.col, DISP_HILI);
    disp_file(srcfile,
	      to->end.line, to->end.col + 1,
	      end_line, -1, 0);
    disp_end();
}

static void
same(const char *srcfile,
     int start_line,
     int end_line,
     SE_POSITION * blk,
     const char *str)
{
    disp_file(srcfile,
	      start_line, 1,
	      blk->start.line, 0, DISP_INDENT);
    disp_str(str, DISP_HILI);
    if (blk->start.col != 1)
	disp_file(srcfile,
		  blk->start.line, 1,
		  blk->start.line, blk->start.col - 1, DISP_INDENT);
    disp_file(srcfile,
	      blk->start.line, blk->start.col,
	      blk->end.line, blk->end.col, DISP_HILI);
    disp_file(srcfile,
	      blk->end.line, blk->end.col + 1,
	      end_line, -1, 0);
    disp_end();
}
