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
#ifndef MVS
#include <sys/ioctl.h>		/* for TIOCGWINSZ */
#endif /* MVS */

#include "portable.h"
#include "disp.h"

static char const disp_c[] = "$Id: disp.c,v 3.7 2013/12/09 01:07:24 tom Exp $";
/*
* @Log: disp.c,v @
* Revision 3.5  1995/12/29 21:27:22  tom
* adjust headers, prototyped for autoconfig
* correct sign-extension bug (computing centered title that may be wider than
* the maximum width).
*
* Revision 3.4  94/04/04  13:50:55  saul
* Fix binary copyright.
* 
* Revision 3.3  94/04/04  10:25:04  jrh
* Add Release Copyright
* 
* Revision 3.2  94/01/31  14:18:11  saul
* Add disp_title interface to standardize display headings.
* 
* Revision 3.1  93/08/04  15:53:27  ewk
* Added MVS and solaris support.  Squelched some ANSI warnings.
* 
* Revision 3.0  92/11/06  07:48:02  saul
* propagate to version 3.0
* 
* Revision 2.8  92/10/30  09:54:06  saul
* include portable.h
* 
* Revision 2.7  92/10/29  11:16:20  saul
* Use window size.
* New elipsis format.
* 
* Revision 2.6  92/09/08  08:40:36  saul
* Remove unused variable.
* 
* Revision 2.5  92/07/10  11:13:11  saul
* end of file display error
* 
* Revision 2.4  92/04/29  08:27:23  saul
* add | to hili script to allow filename begining with digit.
* 
* Revision 2.3  92/03/17  15:27:04  saul
* copyright
* 
* Revision 2.2  91/12/13  09:26:09  saul
* same length file name bug
* Two files in a row with same length file name would not display correctly.
* 
* Revision 2.1  91/06/19  13:09:58  saul
* Propagate to version 2.0
* 
* Revision 1.1  91/06/12  16:58:21  saul
* Aug 1990 baseline
* 
*-----------------------------------------------end of log
*/

static char cur_file[MAX_SRCFILE_NAME] = "";
static int cur_line;
static int cur_col;
static int end_line = 0;
static int end_col = 0;

void
disp_str(const char *str,
	 int attributes)
{
    if (attributes & DISP_HILI) {
	if (attributes & DISP_NEWLINE)
	    printf("h %d %d|", end_line, end_col);
	else
	    printf("H %d %d|", end_line, end_col);
    } else {
	if (attributes & DISP_NEWLINE)
	    printf("t %d %d|", end_line, end_col);
	else
	    printf("T %d %d|", end_line, end_col);
    }
    if (attributes & DISP_CLEAR)
	putchar('\f');
    if (attributes & DISP_UNINDENT)
	putchar('\r');
    puts(str);
}

void
disp_file(const char *filename,
	  int f_line,
	  int f_col,
	  int t_line,
	  int t_col,
	  int attributes)
{
    if (*cur_file == '\0' || strcmp(filename, cur_file) ||
	f_line < cur_line ||
	(f_line == cur_line && (f_col < cur_col || cur_col == -1))) {
	if (end_line) {
	    printf("O %d %d|\n", end_line, end_col);
	    end_line = 0;
	    end_col = 0;
	}
	strncpy(cur_file, filename, MAX_SRCFILE_NAME);
	cur_line = 1;
	cur_col = 1;
	printf("f 0 0|%s\n", filename);
	if (attributes & DISP_INDENT)
	    printf("m 0 0|\n");
    }

    if (cur_line < f_line || (cur_line == f_line && cur_col < f_col - 1)) {
	printf("O 0 0|\n");
	printf("o %d %d|\n", f_line, f_col);
    }
    if (attributes & DISP_HILI) {
	printf("+ %d %d|\n", f_line, f_col);
	printf("- %d %d|\n", t_line, t_col);
    } else {
	end_line = t_line;
	end_col = t_col;
    }
    cur_line = t_line;
    cur_col = t_col;
}

void
disp_end(void)
{
    if (end_line) {
	printf("O %d %d|\n", end_line, end_col);
	end_line = 0;
	end_col = 0;
    }
}

void
disp_elipsis(int nSkipped)
{
    static int windowSize = 0;
    static char *buf;
    static char msg[] = " (%d lines skipped)";
    int i;

    if (windowSize == 0) {
	windowSize = disp_windowSize();
	if (windowSize < 2)
	    windowSize = 2;
	buf = (char *) malloc((size_t) windowSize + 1);
    }

    disp_str("", DISP_NEWLINE);

    for (i = 0; i < windowSize - 3; i += 2) {
	buf[i] = 'o';
	buf[i + 1] = ' ';
    }
    buf[windowSize - 2] = '\0';

    if ((windowSize - (int) sizeof(msg)) >= 10) {
	sprintf(buf + (windowSize - (int) sizeof(msg)) / 2 - 5, msg, nSkipped);
	buf[strlen(buf)] = ' ';
    }

    disp_str(buf, DISP_NEWLINE);

    disp_str("", DISP_NEWLINE);
}

#define MAX_HEADER		(MAX_SRCFILE_NAME + 50)

void
disp_title(char *title,
	   int startLine,
	   int endLine)
{
    char buf[MAX_HEADER];
    char buf2[MAX_HEADER];
    int dashes;
    int i;
    char dash;
    char *p;
    static int windowSize = 0;

    if (windowSize == 0)
	windowSize = disp_windowSize();

    dash = '-';
    sprintf(buf, title, startLine, endLine);
    dashes = (windowSize - 2 - (int) strlen(buf) - 4) / 2;
    p = buf2;
    for (i = 0; i < dashes; ++i)
	*p++ = dash;
    sprintf(p, "> %s <", buf);
    p = buf2 + strlen(buf2);
    for (i = 0; i < dashes; ++i)
	*p++ = dash;
    *p = '\0';
    disp_str(buf2, DISP_CLEAR | DISP_NEWLINE);
}

int
disp_windowSize(void)
{
#ifdef TIOCGWINSZ
    struct winsize win;

    if (isatty(1) && ioctl(1, TIOCGWINSZ, &win) != -1 && win.ws_col != 0) {
	return win.ws_col;
    }
#endif /* TIOCGWINSZ */

    return 80;
}

#ifdef TEST
static char bellcoreCopyRight[] =
"Copyright (c) 1993 Bell Communications Research, Inc. (Bellcore)";
int
main(void)
{
    disp_str("heading", DISP_CLEAR | DISP_NEWLINE);
    disp_file("disp.c", 3, 0, 8, 11, 0);
    disp_file("disp.c", 8, 12, 8, 18, DISP_HILI);
    disp_file("disp.c", 8, 19, 15, 0, 0);
    disp_str("heading", DISP_CLEAR | DISP_HILI | DISP_NEWLINE);
    disp_file("disp.c", 3, 0, 8, 0, DISP_INDENT);
    disp_str("from==> ", DISP_UNINDENT | DISP_HILI);
    disp_file("disp.c", 8, 1, 8, 11, 0);
    disp_file("disp.c", 8, 12, 8, 18, DISP_HILI);
    disp_file("disp.c", 8, 19, 15, 0, 0);
    disp_end();
}
#endif
