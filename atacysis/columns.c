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
#include <string.h>
#include <time.h>

#ifndef vms
#ifndef MVS
#include <sys/param.h>
#include <sys/types.h>
#include <sys/stat.h>
#endif	/* MVS */
#endif  /* vms */

#include "portable.h"
#include "atacysis.h"

static char const columns_c[] = 
	"$Header: /users/source/archives/atac.vcs/atacysis/RCS/columns.c,v 3.7 1995/12/29 21:25:35 tom Exp $";
/*
* $Log: columns.c,v $
* Revision 3.7  1995/12/29 21:25:35  tom
* adjust headers, prototyped for autoconfig
* correct gcc warnings (shadowed variables: columns, lines)
*
*Revision 3.6  94/08/03  12:15:25  saul
*Improve portability (linux) by using spaces instead of tabs.
*
*Revision 3.5  94/05/02  10:12:24  saul
*fix comments
*
*Revision 3.4  94/04/04  10:24:57  jrh
*Add Release Copyright
*
*Revision 3.3  93/08/09  13:19:15  ewk
*Set twidth to 80 for MVS, rather than 72.
*
*Revision 3.2  93/08/04  15:52:57  ewk
*Added MVS and solaris support.  Squelched some ANSI warnings.
*
lumns.c
*Revision 3.1  93/07/09  13:20:56  saul
*made compatible with VMS
*
*Revision 3.0  92/11/06  07:47:04  saul
*propagate to version 3.0
*
*Revision 2.5  92/11/02  11:42:39  saul
*.
*
*Revision 2.4  92/10/30  09:53:59  saul
*include portable.h
*
*Revision 2.3  92/10/28  13:44:36  saul
*Bad mem allocation causing core dump
*
*Revision 2.2  92/10/28  09:02:50  saul
*#ifdef TIOCGWINSZ for portability
*
*Revision 2.1  92/09/08  10:24:10  saul
*Multi column -l output.
*
*-----------------------------------------------end of log
*/

/* forward declarations */
static void formatf P_((char **fp0, char **fplast, int twidth, int Cflg));
static void ttystuff P_((int *twidth, int *Cflg));

#define LINE_POOL_SIZE	50

static int		nLines = 0;
static char		**lines;

void
columns(p)
char *p;
{
    static int	nLinePool = 0;

    if (nLines + 1 >= nLinePool) {
	nLinePool += LINE_POOL_SIZE;
	if (nLines == 0) {
	    lines = (char **)malloc(nLinePool * sizeof *lines);
	} else {
	    lines = (char **)realloc(lines, nLinePool * sizeof *lines);
	}
	if (lines == NULL) {
	    fprintf(stderr, "columns: out of memory\n");
	    exit(1);
	}
    }
    lines[nLines++] = p;
}

void
columnsEnd()
{
    int		twidth;
    int		Cflg;

    if (nLines == 0) return;

    ttystuff(&twidth, &Cflg);

    formatf(lines, lines + nLines, twidth, Cflg);

    free(lines);
}

static void
ttystuff(twidth, Cflg)
int *twidth;
int *Cflg;
{
    *twidth = 80;
    *Cflg = 1;
#ifdef TIOCGWINSZ
{
    struct	winsize win;
    if (ioctl(1, TIOCGWINSZ, &win) != -1)
	*twidth = (win.ws_col == 0 ? 80 : win.ws_col);
}
#endif /* TIOCGWINSZ */
#ifndef MVS
    if (!isatty(1))
	*Cflg = 0;
#endif /* not MVS */
}

static void
formatf(fp0, fplast, twidth, Cflg)
	char **fp0, **fplast;
	int twidth;
	int Cflg;
{
	register char **fp;
	register int i, j, w;
	int width = 0, nentry = fplast - fp0;
	int cols, rows;
	char *cp;

	if (fp0 == fplast)
		return;
	if (Cflg == 0)
		cols = 1;
	else {
		for (fp = fp0; fp < fplast; fp++) {
			int len = strlen(*fp);
				if (len > width)
				width = len;
		}
		width += 2;
		cols = twidth / width;
		if (cols == 0)
			cols = 1;
	}
	rows = (nentry + cols - 1) / cols;
	for (i = 0; i < rows; i++) {
		for (j = 0; j < cols; j++) {
			fp = fp0 + j * rows + i;
			cp = *fp;
			fputs(cp, stdout);
			if (fp + rows >= fplast) {
				putchar('\n');
				break;
			}
			w = strlen(cp);
			while (w < width) {
			    w++;
			    putchar(' ');
			}
		}
	}
}
