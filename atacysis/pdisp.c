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

static char pdisp_c[] = 
	"$Header: /users/source/archives/atac.vcs/atacysis/RCS/pdisp.c,v 3.6 1994/04/04 10:25:55 jrh Exp $";
/*
*-----------------------------------------------$Log: pdisp.c,v $
*-----------------------------------------------Revision 3.6  1994/04/04 10:25:55  jrh
*-----------------------------------------------FROM_KEYS
*-----------------------------------------------
*Revision 3.6  94/04/04  10:25:55  jrh
*Add Release Copyright
*
*Revision 3.5  94/01/31  14:23:48  saul
*Add disp_title interface to standardize display headings.
*
*Revision 3.4  93/11/02  11:53:19  saul
*Same as revision 3.2
*
*Revision 3.2  93/08/04  15:57:29  ewk
*Added MVS and solaris support.  Squelched some ANSI warnings.
*
* Revision 3.1  93/01/15  12:06:23  saul
*Don't do "1 lines skipped".
*
*Revision 3.0  92/11/06  07:48:16  saul
*propagate to version 3.0
*
*Revision 2.5  92/11/02  11:44:18  saul
*remove unused variables
*
*Revision 2.4  92/10/30  09:55:21  saul
*include portable.h
*
*Revision 2.3  92/10/29  11:16:01  saul
*Use window size.
*New elipsis format.
*
*Revision 2.2  92/09/08  09:08:55  saul
*New coverage vector data struture.  General display cleanup.
*
*Revision 2.1  92/07/10  13:45:55  saul
*new
*
*-----------------------------------------------end of log
*/
#include <stdio.h>
#include "portable.h"
#include "atacysis.h"
#include "disp.h"

/* forward declarations */
static void same();
static void fr_to();
static void from_to();
static void display();
static void print_header();
void pdisp();

#define SRC_CONTEXT_LINES	4

#define MAX_HEADER		(MAX_SRCFILE_NAME + 50)

#define MAX(a,b) ((a)>(b)?(a):(b))

typedef int	RLIST;

typedef struct {
	RLIST	**rlist;
	int	notcov;
	int	tot;
} COV_LIST;

void
pdisp(modules, n_mod, covVector, displayMode)
T_MODULE	*modules;
int		n_mod;
int		*covVector;
int		displayMode;
{
    int		i;
    T_FUNC	*func;
    T_MODULE	*mod;
    T_PUSE	*puse;
    int		decis_var;
    int		none = 1; /* 1 if no p-use to be printed;0 otherwise */

    for (mod = modules; mod < modules + n_mod; ++mod) {
	for (func = mod->func; func < mod->func + mod->n_func; ++func) {
	    if (func->ignore) continue;
	    /*
	    * Find and display not covered c-uses.
	    */
	    decis_var = func->decis_var;
	    puse =  func->puse;
	    for (i = 0; i < (int)func->n_puse; ++i) {
		puse = func->puse + i;
		if (puse->varno == decis_var) continue;
		if (covVector[func->pUseCovStart + i] != 0)
		{
		    continue;
		}
		if (puse->def.start.file != puse->def.end.file)
		    continue;
		if (puse->def.start.file != puse->use.start.file)
		    continue;
		if (puse->def.start.file != puse->use.end.file)
		    continue;
		none = 0;
		display(mod->file + puse->def.start.file,
			mod->atacfile, func->fname, &puse->def,
			&puse->use, &func->pos, puse->value,
			func->var[puse->varno].vname);
	    }
	}
    }
    if (none) {
	switch (displayMode & (DISPLAY_ALL | DISPLAY_COV))
	{
	case DISPLAY_COV | DISPLAY_ALL:
	    disp_str("No covered p-uses.", DISP_NEWLINE);
	    break;
	case DISPLAY_ALL:
	    disp_str("No uncovered p-uses.", DISP_NEWLINE);
	    break;
	case DISPLAY_COV:
	    disp_str("No covered p-uses in covered blocks/decisions.",
		     DISP_NEWLINE);
	    break;
	case 0:
	    disp_str("No uncovered p-uses in covered blocks/decisions.",
		     DISP_NEWLINE);
	    break;
	}
	disp_end();
    }
}

static void
print_header(srcfile, func, line, dline, var)
char	*srcfile;
char	*func;
int	line;
int	dline;
char	*var;
{
	char		buf[MAX_HEADER];

	sprintf(buf, "%s:%s P-USE of %s at line %d [def at line %d]",
		srcfile, func, var, line, dline);
	disp_title(buf, 0, 0);
}

static void
display(file, atacfile, funcname, from, to, func, value, var)
T_FILE		*file;
char		*atacfile;
char		*funcname;
SE_POSITION	*from;
SE_POSITION	*to;
SE_POSITION	*func;
char		*value;
char		*var;
{
	int		start;
	int		end;
	char		*srcfile;
	char		v[9];
	int		vLen;
	SE_POSITION	overlap;

	vLen = strlen(value);

	srcfile = srcfile_name(file->filename, &file->chgtime, atacfile);
	print_header(srcfile, funcname, to->start.line, from->start.line, var);
	if (from->start.line < to->start.line) {
		start = from->start.line - SRC_CONTEXT_LINES;
		if (start <= 0) start = 1;
		if (start < (int)func->start.line
		    	&& from->start.line >= func->start.line
			&& func->start.file == from->start.file)
		{
			start = func->start.line;
		}
		end = to->end.line + SRC_CONTEXT_LINES;
		if (func->end.file == to->end.file && end > (int)func->end.line) 
			end = func->end.line;
		sprintf(v, "%.7s%.*s>", value, MAX(7-vLen,0), "=======");
		from_to(srcfile, start, end, from, to, "def====>", v);
	} 
	else if (from->start.line > to->start.line) {
		start = to->start.line - SRC_CONTEXT_LINES;
		if (start <= 0) start = 1;
		if (start < func->start.line
		    && to->start.line >= (int)func->start.line
		    && func->start.file == to->end.file)
		{
			start = func->start.line;
		}
		end = from->end.line + SRC_CONTEXT_LINES;
		if (func->end.file == to->end.file && end > (int)func->end.line) 
			end = func->end.line;
		sprintf(v, "%.7s%.*s>", value, MAX(7-vLen,0), "=======");
		from_to(srcfile, start, end, to, from, v, "def====>");
	} else {	/* same line */
		start = from->start.line - SRC_CONTEXT_LINES;
		if (start <= 0) start = 1;
		if (start < (int)func->start.line
		    && from->start.line >= func->start.line
		    && func->start.file == to->end.file)
		{
			start = func->start.line;
		}
		if (from->start.col < to->start.col)
			end = to->end.line + SRC_CONTEXT_LINES;
		else
			end = from->end.line + SRC_CONTEXT_LINES;
		if (func->end.file == to->end.file && end > (int)func->end.line) 
			end = func->end.line;
		if (from->end.col < to->start.col) {
 		    sprintf(v, "d/%.5s%.*s>", value, MAX(5-vLen,0), "=====");
		    fr_to(srcfile, start, end, from, to, v);
		} else {
		    if (from->start.col > to->end.col) {
 		        sprintf(v, "d/%.5s%.*s>", value,MAX(5-vLen,0),"=====");
			fr_to(srcfile, start, end, to, from, v);
		    } else {
			if (from->start.col <= to->start.col)
			    overlap.start = from->start;
			else
			    overlap.start = to->start;
			if (from->end.col >= to->end.col)
			    overlap.end = from->end;
			else
			    overlap.end = to->end;
 		        sprintf(v, "d/%.5s%.*s>", value,MAX(5-vLen,0),"=====");
			same(srcfile, start, end, &overlap, v);
		    }
		}
	} 
}

static void
from_to(srcfile, start_line, end_line, from, to, f_str, t_str)
char		*srcfile;
int		start_line;
int		end_line;
SE_POSITION	*from;
SE_POSITION	*to;
char		*f_str;
char		*t_str;
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
		if ((int)(to->start.line - from->end.line) > 11) {
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
fr_to(srcfile, start_line, end_line, from, to, str)
char		*srcfile;
int		start_line;
int		end_line;
SE_POSITION	*from;
SE_POSITION	*to;
char		*str;
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
same(srcfile, start_line, end_line, blk, str)
char		*srcfile;
int		start_line;
int		end_line;
SE_POSITION	*blk;
char		*str;
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
