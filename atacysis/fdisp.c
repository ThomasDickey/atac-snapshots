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

static char const fdisp_c[] = 
	"$Header: /users/source/archives/atac.vcs/atacysis/RCS/fdisp.c,v 3.5 1995/12/27 20:24:53 tom Exp $";
/*
* $Log: fdisp.c,v $
* Revision 3.5  1995/12/27 20:24:53  tom
* adjust headers, prototyped for autoconfig
*
*Revision 3.4  94/04/04  10:25:17  jrh
*Add Release Copyright
*
*Revision 3.3  94/01/31  14:22:01  saul
*Add disp_title interface to standardize display headings.
*
*Revision 3.2  93/08/04  15:54:03  ewk
*Added MVS and solaris support.  Squelched some ANSI warnings.
*
* Revision 3.1  92/12/02  11:36:36  saul
*Change heading from "covered" to "entered" for -mf.
*
*Revision 3.0  92/11/06  07:47:14  saul
*propagate to version 3.0
*
*Revision 2.3  92/10/30  09:54:13  saul
*include portable.h
*
*Revision 2.2  92/10/29  11:15:27  saul
*Use window size.
*
*Revision 2.1  92/09/08  09:03:48  saul
*New function entry display feature.
*
*-----------------------------------------------end of log
*/

/* forward declarations */
static void print_header P_((char *srcfile, int cov, int tot, int displayMode));

#define MAX_HEADER		(MAX_SRCFILE_NAME + 50)

static void
print_header(srcfile, cov, tot, displayMode)
char	*srcfile;
int	cov;
int	tot;
int	displayMode;
{
	char	buf[MAX_HEADER];
	char    *not;

	if (displayMode & DISPLAY_COV) {
	    not = "";
	} else {
	    not = "not ";
	}
	sprintf(buf, "%s %d of %d functions %scovered",	srcfile, cov, tot, not);
	disp_title(buf, 0, 0);
}

void
fdisp(modules, n_mod, covVector, displayMode)
T_MODULE	*modules;
int		n_mod;
int		*covVector;
int		displayMode;
{
    T_FUNC	*func;
    T_MODULE	*mod;
    int         tot;
    int         nDisp;
    int		totNDisp;

    totNDisp = 0;
    for (mod = modules; mod < modules + n_mod; ++mod) {
	if (mod->ignore) continue;
	tot = 0;
	nDisp = 0;
	for (func = mod->func; func < mod->func + mod->n_func; ++func) {
	    if (func->ignore) continue;
	    ++tot;
	    if (covVector[func->blkCovStart] == 0) {
		++nDisp;
	    }
	}
	if (nDisp || displayMode & DISPLAY_ALL) {
	    print_header(srcfile_name(mod->file[0].filename,
				      &mod->file[0].chgtime, mod->atacfile),
			 nDisp, tot, displayMode);
	    totNDisp += nDisp;
	}
	for (func = mod->func; func < mod->func + mod->n_func; ++func) {
	    if (func->ignore) continue;
	    if (covVector[func->blkCovStart] == 0) {
		disp_str(func->fname, DISP_NEWLINE);
	    }
	}
    }

    if (totNDisp == 0 && !(displayMode & DISPLAY_ALL)) {
	if (displayMode & DISPLAY_COV) {
	    disp_str("No functions entered.", DISP_NEWLINE);
	} else {
	    disp_str("All functions entered.", DISP_NEWLINE);
	}
    }
}
