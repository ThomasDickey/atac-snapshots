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

static char bdisp_c[] =
	"$Header: /users/source/archives/atac.vcs/atacysis/RCS/bdisp.c,v 3.5 1994/04/04 10:24:52 jrh Exp $";
/*
*-----------------------------------------------$Log: bdisp.c,v $
*-----------------------------------------------Revision 3.5  1994/04/04 10:24:52  jrh
*-----------------------------------------------FROM_KEYS
*-----------------------------------------------
* Revision 3.5  94/04/04  10:24:52  jrh
* Add Release Copyright
* 
* Revision 3.4  94/01/31  14:20:24  saul
* Add disp_title interface to standardize display headings.
* 
* Revision 3.3  93/11/02  11:44:17  saul
* Same as revision 3.1.
* 
* Revision 3.1  93/08/04  15:52:40  ewk
* Added MVS and solaris support.  Squelched some ANSI warnings.
* 
* Revision 3.0  92/11/06  07:47:56  saul
* propagate to version 3.0
* 
* Revision 2.10  92/11/02  11:44:11  saul
* remove unused variables
* 
* Revision 2.9  92/10/30  09:53:53  saul
* include portable.h
* 
* Revision 2.8  92/10/29  11:14:51  saul
* Use window size.
* 
* Revision 2.7  92/10/22  10:51:42  saul
* uncovered block display total block count may be too high.
* 
* Revision 2.6  92/09/08  09:09:51  saul
* New coverage vector data struture.  General display cleanup.
* 
* Revision 2.5  92/07/10  11:05:43  saul
* new POSITION struct
* 
* Revision 2.4  92/06/05  16:40:52  saul
* Fix nspec display problem
* 
* Revision 2.3  92/03/17  15:26:59  saul
* copyright
* 
* Revision 2.2  91/12/13  09:28:15  saul
* same length file name bug
* 
* Revision 2.1  91/06/19  13:09:55  saul
* Propagate to version 2.0
* 
* Revision 1.1  91/06/12  16:58:16  saul
* Aug 1990 baseline
* 
*-----------------------------------------------end of log
*/
#include <stdio.h>
#include "portable.h"
#include "atacysis.h"
#include "disp.h"

#define SRC_CONTEXT_LINES	7

#define MAX_HEADER		(MAX_SRCFILE_NAME + 50)

#define CHECK_MALLOC(p) if((p)==NULL)fprintf(stderr,"Out of memory\n"),exit(1)

typedef int	RLIST;

typedef struct {
	RLIST	**rlist;
	int	nDisp;
	int	tot;
} COV_LIST;

/* forward declarations */
static void display();
static void print_header();
void bdisp();

void
bdisp(modules, n_mod, covVector, displayMode)
T_MODULE	*modules;
int		n_mod;
int		*covVector;
int		displayMode;
{
    int		i;
    T_FUNC	*func;
    T_BLK	*blk;
    T_MODULE	*mod;
    int		n_file;
    COV_LIST	*cov_list;
    COV_LIST	*clist;
    int		none;

    none = 1;

    /*
    * Make room for maximum number of files.
    */
    n_file = 0;
    for (mod = modules; mod < modules + n_mod; ++mod)
	    if (n_file < (int)mod->n_file) n_file = mod->n_file;
    cov_list = (COV_LIST *)malloc(n_file * sizeof *cov_list);
    CHECK_MALLOC(cov_list);
    for (i = 0; i < n_file; ++i) {
	    cov_list[i].rlist = (RLIST **)rlist_create();
    }

    for (mod = modules; mod < modules + n_mod; ++mod) {
	for (func = mod->func; func < mod->func + mod->n_func; ++func) {
	    if (func->ignore) continue;
	    /*
	    * Initialize display counts for this function accross all files.
	    */
	    for (i = 0; i < n_file; ++i) {
		cov_list[i].nDisp = 0;
		cov_list[i].tot = 1;
	    }
	    /*
	    * Find blocks to be displayed.
	    */
	    for(i = 1; i < (int)func->n_blk; ++i) {
		blk = func->blk + i;
		if (blk->pos.start.file != blk->pos.end.file) continue;
		clist = cov_list + blk->pos.start.file;
		if (covVector[func->blkCovStart + i] != -1) {
		    ++clist->tot;
		    if (covVector[func->blkCovStart + i] == 0)
		    {
			++clist->nDisp;
			rlist_put(clist->rlist, blk->pos.start.line,
				  blk->pos.start.col,
				  blk->pos.end.line,
				  blk->pos.end.col);
		    }
		}
	    }

	    /*
	    * Display blocks.
	    */
	    for (i = 0; i < n_file; ++i) {
		    clist = cov_list + i;
		    if (clist->tot == 1) continue;
		    if (clist->nDisp == 0) continue;
		    none = 0;
		    display(mod->file + i, mod->atacfile, func->fname,
			    clist->nDisp, clist->tot, clist->rlist,
			    &func->pos, displayMode);
		    clist->tot = 1;
		    clist->nDisp = 0;
	    }
	}
    }
    if (none) {
	switch (displayMode & (DISPLAY_ALL | DISPLAY_COV))
	{
	case DISPLAY_COV | DISPLAY_ALL:
	    disp_str("No covered blocks.", DISP_NEWLINE);
	    break;
	case DISPLAY_ALL:
	    disp_str("No uncovered blocks.", DISP_NEWLINE);
	    break;
	case DISPLAY_COV:
	    disp_str("No covered blocks in entered functions.", DISP_NEWLINE);
	    break;
	case 0:
	    disp_str("No uncovered blocks in entered functions.", DISP_NEWLINE);
	    break;
	}
    }
    disp_end();

    /*
    * Free empty lists.  (Lists are emptied in display()).
    */
    for (i = 0; i < n_file; ++i)
	    rlist_free(cov_list[i].rlist);
}

static void
print_header(srcfile, func, nDisp, tot, displayMode, startLine, endLine)
char	*srcfile;
char	*func;
int	nDisp;
int	tot;
int	displayMode;
int	startLine;
int	endLine;
{
	char	buf[MAX_HEADER];
	char	*not;

	if (displayMode & DISPLAY_COV) {
	    not = "";
	} else {
	    not = "not ";
	}
	sprintf(buf, "%s:%s [%d of %d blocks %scovered] lines %%d - %%d",
			srcfile, func, nDisp, tot, not);
	disp_title(buf, startLine, endLine);
}

static void
display(file, atacfile, funcname, nDisp, tot, rlist, func, displayMode)
T_FILE		*file;
char		*atacfile;
char		*funcname;
int		nDisp;
int		tot;
RLIST		**rlist;
SE_POSITION	*func;
int		displayMode;
{
	int		s1, s2, e1, e2, p1, p2;
	int		start;
	int		end;
	char		*srcfile;
	static char	*prev_filename = NULL;
	static		prev_end;

	srcfile = srcfile_name(file->filename, &file->chgtime, atacfile);
	if (rlist_get(rlist, &s1, &s2, &e1, &e2) == 0)	/* get last */
		return;
	end = e1 + SRC_CONTEXT_LINES;
	if (func->end.file == 0 && end > (int)func->end.line) 
		end = func->end.line;
	rlist_put(rlist, s1, s2, e1, e2);	/* put it back */
	rlist_reverse(rlist);
	if (rlist_get(rlist, &s1, &s2, &e1, &e2) == 0)
		return;
	start = s1 - SRC_CONTEXT_LINES;
	if (start <= 0) start = 1;
	if (func->start.file == 0 && start < (int)func->start.line) 
		start = func->start.line;

	print_header(srcfile, funcname, nDisp, tot, displayMode, start, end);

	if (file->filename != prev_filename || start <= prev_end) {
		prev_filename = file->filename;
	}
	p1 = start;
	p2 = 1;

	do {
		disp_file(srcfile, p1, p2, s1, s2 - 1, 0);
		disp_file(srcfile, s1, s2, e1, e2, DISP_HILI);
		p1 = e1;
		p2 = e2 + 1;
	} while (rlist_get(rlist, &s1, &s2, &e1, &e2));

	disp_file(srcfile, p1, p2, end, -1, 0);

	prev_end = end;
}
