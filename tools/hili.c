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

static const char hili_c[] = "$Id: hili.c,v 3.11 2013/12/08 23:25:57 tom Exp $";
static const char bellcoreCopyRight[] =
"Copyright (c) 1993 Bell Communications Research, Inc. (Bellcore)";

/*
* @Log: hili.c,v @
* Revision 3.9  1998/08/23 19:50:57  tom
* fix gcc warnings for both termcap and terminfo configurations.
*
* Revision 3.8  1997/12/10 01:51:44  tom
* prototyped outchar()
*
* Revision 3.7  1996/12/02 01:55:52  tom
* gcc warnings (missing prototypes)
*
* Revision 3.6  1995/12/27 23:34:47  tom
* fix gcc warnings (missing prototypes)
*
* Revision 3.5  94/07/11  14:26:05  saul
* Enlarge header buffer.  (Overflow causes display of raw hili input.)
*
* Revision 3.4  94/04/04  15:06:27  saul
* *** empty log message ***
*
* Revision 3.3  94/04/04  10:52:46  jrh
* Add Release Copyright
*
* Revision 3.2  93/08/04  15:50:21  ewk
* Added MVS and solaris support.  Squelched some ANSI warnings.
*
* Revision 3.1  93/03/31  11:44:04  saul
* Change isgraph to isprint && != ' ' for portablility.
*
* Revision 3.0  92/11/06  07:46:30  saul
* propagate to version 3.0
*
* Revision 2.8  92/10/30  09:42:29  saul
* include portable.h
*
* Revision 2.7  92/06/11  14:19:42  saul
* output not turned back on when file opened
*
* Revision 2.6  92/05/11  09:25:16  saul
* Outputs extra blank line in some circumstances.
*
* Revision 2.5  92/05/01  13:27:26  saul
* Avoid hilighting leading blanks by changing isprint to isgraph.
*
* Revision 2.4  92/04/29  10:48:56  saul
* fixed -u option to work for headings
*
* Revision 2.3  92/04/29  08:29:00  saul
* Expect | in script to allow filename begining with digit.
*
* Revision 2.2  92/03/17  15:34:50  saul
* copyright
*
* Revision 2.1  91/06/19  13:56:27  saul
* Propagte to version 2.0
*
* Revision 1.1  91/06/12  16:39:08  saul
* Aug 1990 baseline
*
*-----------------------------------------------end of log
*/
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

#ifdef M_TERMINFO
#include <curses.h>
#include <term.h>
#endif

#ifdef M_TERMCAP
#if HAVE_TERMCAP_H
#include <termcap.h>
#else
extern int tgetent();
extern char *tgetstr();
#endif
#endif

#include "portable.h"

/* forward declarations */
extern void initcap(void);
static int getscript(FILE *f, unsigned *line, unsigned *col, int *cmd, char *sarg);
static void mode_normal(void);
static void mode_reverse(void);

#define USAGE(S)	"usage: %s [-u] [-r] {script-file | -} [text-file | -]\n", S

#define SARG_MAX	1024

char *pgmname = "";

#ifdef M_TERMCAP
static char *reverse_code;
static char *normal_code;
#endif

static void
uputs(char *s)
{
    register char *p;

    for (p = s; *p; ++p) {
	putchar('_');
	putchar('\b');
	putchar(*p);
    }
}

static void
mputs(char *s)
{
    register char *p;

    for (p = s; *p; ++p)
	putchar(*p);
}

int
main(int argc,
     char *argv[])
{
    int c;
    int argi;
    char *p;
    FILE *script;
    FILE *text;
    unsigned cur_line;
    unsigned cur_col;
    unsigned line;
    unsigned col;
    int cmd;
    int on_count;
    int on;
    int onvalue;
    int h_state;
    int morescript;
    int ulflag;
    int output;
    char sarg[SARG_MAX];
    int margin;
    int tabcol;

    pgmname = argv[0];
    ulflag = 0;
    onvalue = 1;
    margin = 0;

    for (argi = 1; argi < argc; ++argi) {
	p = argv[argi];
	if (*p++ != '-' || *p == '\0')
	    break;
	do
	    switch (*p) {
	    case 'u':
		ulflag = 1;
		break;
	    case 'r':
		onvalue = 0;
		break;
	    default:
		fprintf(stderr, "%s: unknown option: -%c\n",
			pgmname, *p);
		fprintf(stderr, USAGE(pgmname));
		exit(1);
		break;
	    }
	while (*++p);
    }

    text = stdin;
    switch (argc - argi) {
    case 1:
	break;
    case 2:
	if (strcmp(argv[argi + 1], "-") == 0)
	    break;
	text = fopen(argv[argi + 1], "r");
	if (text == NULL) {
	    fprintf(stderr, "can't open %s\n", argv[argi + 1]);
	    exit(1);
	}
	break;
    default:
	fprintf(stderr, USAGE(pgmname));
	exit(1);
    }
    if (strcmp(argv[argi], "-") == 0)
	script = stdin;
    else {
	script = fopen(argv[argi], "r");
	if (script == NULL) {
	    fprintf(stderr, "can't open %s\n", argv[argi]);
	    exit(1);
	}
    }

    if (!ulflag)
	initcap();

    output = 1;

    h_state = 0;
    on_count = 0;
    on = 0;
    morescript = 1;
    if (!getscript(script, &line, &col, &cmd, sarg))
	morescript = 0;

    tabcol = 1;
    cur_line = 1;
    cur_col = 1;
    while (1) {
	while (morescript &&
	       (cur_line > line ||
		(cur_line == line && cur_col >= col))) {
	    switch (cmd) {
	    case '-':
		if (on_count)
		    if (--on_count == 0)
			on = 0;
		break;
	    case '+':
		++on_count;
		on = 1;
		break;
	    case 'o':
		output = 1;
		break;
	    case 'O':
		output = 0;
		break;
	    case 'f':
		if (text && text != script)
		    fclose(text);
		text = fopen(sarg, "r");
		if (text == NULL) {
		    fprintf(stderr, "can't open %s\n", sarg);
		    exit(1);
		}
		output = 1;
		cur_line = 1;
		cur_col = 1;
		break;
	    case 't':
		if (h_state == 1) {
		    mode_normal();
		    h_state = 0;
		}
		mputs(sarg);
		putchar('\n');
		tabcol = 1;
		break;
	    case 'T':
		if (h_state == 1) {
		    mode_normal();
		    h_state = 0;
		}
		mputs(sarg);
		tabcol += (int) strlen(sarg);
		break;
	    case 'h':
		if (ulflag)
		    uputs(sarg);
		else {
		    if (h_state == 0) {
			mode_reverse();
			h_state = 1;
		    }
		    mputs(sarg);
		    mode_normal();
		}
		putchar('\n');
		tabcol = 1;
		break;
	    case 'H':
		if (ulflag)
		    uputs(sarg);
		else {
		    if (h_state == 0) {
			mode_reverse();
			h_state = 1;
		    }
		    mputs(sarg);
		}
		tabcol += (int) strlen(sarg);
		break;
	    case 'm':
		margin = 8;
		break;
	    }
	    if (!getscript(script, &line, &col, &cmd, sarg)) {
		morescript = 0;
		if (output == 0) {
		    if (h_state)
			mode_normal();
		    exit(0);
		}
	    }
	}
	if ((c = getc(text)) == EOF)
	    break;
	if (c == '\n') {
	    ++cur_line;
	    cur_col = 1;
	} else
	    ++cur_col;
	if (output) {
	    if (on == onvalue) {
		if (isprint(c) && c != ' ') {
		    if (ulflag) {
			putchar('_');
			putchar('\b');
		    } else if (h_state == 0) {
			mode_reverse();
			h_state = 1;
		    }
		} else if (c == '\n') {
		    mode_normal();
		    h_state = 0;
		}
	    } else if (h_state) {
		mode_normal();
		h_state = 0;
	    }
	    if (tabcol == 1 && margin)
		mputs("        ");
	    if (c == '\t') {
		do
		    putchar(' ');
		while (++tabcol % 8 != 1);
	    } else {
		putchar(c);
		if (c == '\n')
		    tabcol = 1;
		else
		    ++tabcol;
	    }
	}
    }

    if (h_state)
	mode_normal();

    exit(0);
}

#ifdef M_TERMCAP
void
initcap(void)
{
    char *termtype;
    static char tcapbuf[512];
    char termcap[1024];
    char *bp = tcapbuf;

    termtype = getenv("TERM");
    if (termtype == NULL)
	termtype = "lpr";
    if (tgetent(termcap, termtype) != 1) {
	fprintf(stderr, "%s: trouble reading termcap\n", pgmname);
	exit(1);
    }
#ifdef OLD
    /* This nonsense attempts to work with both old and new termcap */
    reverse_code = tgetstr("mr", &bp);
    normal_code = tgetstr("me", &bp);
#else
    reverse_code = tgetstr("so", &bp);
    normal_code = tgetstr("se", &bp);
#endif

    if (reverse_code == 0 || normal_code == 0) {
	fprintf(stderr, "%s: no termcap highlight mode for <%s>\n",
		pgmname, termtype);
	exit(1);
    }
}

#ifndef OUTC_ARGS
#define OUTC_ARGS int c
#endif

#define OUTC_FUNC(func) func(OUTC_ARGS)

#ifdef OUTC_RETURN
static int
OUTC_FUNC(outchar)
{
    return putchar(c & 0177);
}
#else
static void
OUTC_FUNC(outchar)
{
    (void) putchar(c & 0177);
}
#endif

static void
mode_normal(void)
{
    tputs(normal_code, 1, outchar);
}

static void
mode_reverse(void)
{
    tputs(reverse_code, 1, outchar);
}
#endif

#ifdef M_TERMINFO
void
initcap(void)
{
    char *termtype;
    int err;

    termtype = getenv("TERM");
    if (termtype == NULL)
	termtype = "lpr";
    setupterm(termtype, 1, &err);
}

static void
mode_normal(void)
{
    vidattr(0);
}

static void
mode_reverse(void)
{
    vidattr(A_STANDOUT);
}
#endif

static int
getscript(FILE *f,
	  unsigned *line,
	  unsigned *col,
	  int *cmd,
	  char *sarg)
{
    char s[10];
    char inbuf[1024];

    if (fgets(inbuf, sizeof inbuf, f) == NULL)
	return 0;
    *sarg = '\0';
    if (sscanf(inbuf, "%s %d %d|%[^\n]", s, line, col, sarg) < 3)
	return 0;
    if (s[0] == '-')
	++ * col;
    *cmd = s[0];
    return 1;
}
