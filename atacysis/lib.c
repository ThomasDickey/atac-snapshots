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

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include "portable.h"
#include "atacysis.h"

static char const lib_c[] = 
	"$Header: /users/source/archives/atac.vcs/atacysis/RCS/lib.c,v 3.4 1995/12/29 21:24:41 tom Exp $";
/*
* $Log: lib.c,v $
* Revision 3.4  1995/12/29 21:24:41  tom
* adjust headers, prototyped for autoconfig
* correct sign-extension in string-copy.
*
*Revision 3.3  94/04/04  10:25:32  jrh
*Add Release Copyright
*
*Revision 3.2  94/01/03  09:33:27  saul
*Made more complete and consistent to support replacement of getfields.c.
*Comments added.
*
*Revision 3.1  93/08/04  15:55:09  ewk
*Added MVS and solaris support.  Squelched some ANSI warnings.
*
* Revision 3.0  92/11/06  07:47:20  saul
*propagate to version 3.0
*
*Revision 2.8  92/11/02  11:43:13  saul
*remove unused variables
*
*Revision 2.7  92/10/30  09:54:26  saul
*include portable.h
*
*Revision 2.6  92/10/28  09:03:23  saul
*remove enum for portability
*
*Revision 2.5  92/10/05  10:39:24  saul
*Remove ungetchar
*
*Revision 2.4  92/10/01  15:33:04  saul
*compression error at end of lines.
*
*Revision 2.3  92/09/22  15:41:10  saul
*Trace compression.  Also, exit on output error.
*
*Revision 2.2  92/09/08  10:12:14  saul
*changed trace format and data structures
*
*Revision 2.1  92/09/08  09:59:26  saul
*Purdue trace management
*
*-----------------------------------------------end of log
*/

/*
* Routines to read and write ascii data files.  These routines are
* used to read and write line oriented ascii data files where each
* line has a single character key followed by a variable number
* of fields consisting of numbers or strings.  On output, runs of
* the same number are encoded to save space.  (These are transparently
* unencoded on input.)
*
* Unimplemented:
*	getDynamicString -- allocate a big enough buffer & return string
*
* Bugs:
*	- putString doesn't do quoting
*	- should have a .h file to define struct cfile for callers.
*
* Fix:
*	On output, if a string contains !, blank, tab, newline, or quote,
*	or is empty, write it as "string" with every occurance of ", \, and
*	newline preceeded by \.
*/

#define CHECK_MALLOC(p) if((p)==NULL)fprintf(stderr,"Out of memory\n"),exit(1)

/* forward declarations */
static int putLong P_((FILE *fp, long n));

struct cfile *
cf_openIn(path)
char *path;
{
    struct cfile	*cf;

    cf = (struct cfile *)malloc(sizeof *cf);
    CHECK_MALLOC(cf);

    cf->fp = fopen(path, "r");
    if (cf->fp == NULL) {
	free(cf);
	return NULL;
    }

    cf->fileName = (char *)malloc(strlen(path) + 1);
    CHECK_MALLOC(cf->fileName);
    strcpy(cf->fileName, path);

    cf->mode = R_MODE;	/* read */
    cf->lineNo = 0;
    cf->pendingCount = 0;
    cf->pendingValue = -1;
    cf->atFirstChar = 1;

    return cf;
}

struct cfile *
cf_openOut(path)
char *path;
{
    struct cfile	*cf;

    cf = (struct cfile *)malloc(sizeof *cf);
    CHECK_MALLOC(cf);

    cf->fp = fopen(path, "w");
    if (cf->fp == NULL) {
	free(cf);
	return NULL;
    }

    cf->fileName = (char *)malloc(strlen(path) + 1);
    CHECK_MALLOC(cf->fileName);
    strcpy(cf->fileName, path);

    cf->mode = W_MODE;	/* read */
    cf->lineNo = 0;
    cf->pendingCount = 0;
    cf->pendingValue = -1;
    cf->atFirstChar = 1;

    return cf;
}

static int			/* EOF for error */
putLong(fp, n)
FILE		*fp;
long		n;
{
    register int	r;
    char		buf[3 * sizeof(long) + 1];
    register long	value;
    register long	x;
    long		y;
    char		*p;

    r = 0;

    value = n;

    if (value < 0) {
	r |= putc('-', fp);
	value = -value;
    }

    p = buf;

    while (value >= 10) {
	x = value / 10;
	y = (x << 3) + (x << 1);
	*p++ = '0' + value - y;
	value = x;
    }

    r |= putc(value + '0', fp);

    while (p > buf) {
	r |= putc(*--p, fp);
    }

    return r;
}

void
cf_close(cf)
struct cfile *cf;
{
    int		r;
    FILE	*fp;

    r = 0;

    fp = cf->fp;

    if (cf->mode == W_MODE && cf->atFirstChar == 0)
	    cf_putNewline(cf);

    r |= fclose(fp);
    if (r == EOF) {
	perror(cf->fileName);
	exit(1);
    }
    free(cf->fileName);
    free(cf);
}

int
cf_lineNo(cf)
struct cfile *cf;
{
    return cf->lineNo;
}

char *
cf_fileName(cf)
struct cfile *cf;
{
    return cf->fileName;
}

/*
* cf_getFirstChar: Return the next input character that occurs at the begining
*	of a line.  (Empty lines are skipped.)  May return EOF.
*/
int
cf_getFirstChar(cf)
struct cfile *cf;
{
	int	c;
	FILE	*fp;

	if (cf->mode != R_MODE) {
	    fprintf(stderr, "%s: not open for reading\n", cf->fileName);
	    exit(1);
	}
	fp = cf->fp;

	cf->pendingCount = 0;	/* swallow pending values */

	if (cf->atFirstChar) {
	    ++cf->lineNo;
	    cf->atFirstChar = 0;
	} else {
	    do {
		c = getc(fp);
		if (c == EOF) {
		    return c;
		}
	    } while (c != '\n');
	    ++cf->lineNo;
	}

	while ((c = getc(fp)) == '\n') {
	    ++cf->lineNo;
	}

	return c;
}

/*
* cf_getString:  Read the next string of up to len-1 non-blank/non-tab
*	characters from the current line into pString.  Discard characters
*	remaining up to blank/tab (or other space character) or end of line.
*	If at end of line, return null string.
*/
void
cf_getString(cf, pString, len)
struct cfile	*cf;
char		*pString;
int		len;
{
	register int	c;
	register char	*p;
	register char	*pEnd;
	FILE		*fp;
	char		buf[30];

	if (cf->mode != R_MODE) {
	    fprintf(stderr, "%s: not open for reading\n", cf->fileName);
	    exit(1);
	}
	fp = cf->fp;

	if (cf->pendingCount) {
	    --cf->pendingCount;
	    if (len > 0) {
		sprintf(buf, "%ld", cf->pendingValue);
		strncpy(pString, buf, (size_t)len);
		pString[len - 1] = '\0';
	    }
	    return;
	}
	if (cf->atFirstChar) {
	    if (len > 0) pString[0] = '\0';
	    return;
	}

	do {
	    c = getc(fp);
	} while (c == ' ' || c == '\t');

	p = pString;
	if (len == 0) pEnd = p;
	else pEnd = pString + len - 1;

	if (c == '"') {
	    c = getc(fp);
	    while (c != EOF && c != '"' && c != '\n') {
		if (c == '\\') {
		    c = getc(fp);
		    if (c == EOF)
			break;
		}
		if (p < pEnd) *p++ = c;
		c = getc(fp);
	    }
	} else {
	    while (c != EOF && (!isspace(c))) {
		if (p < pEnd) *p++ = c;
		c = getc(fp);
	    }
	}

	if (len) *p = '\0';

	if (c == '\n') {
	    cf->atFirstChar = 1;
	}
}

int
cf_atFirstChar(cf)
struct cfile *cf;
{
	register int	c;
	FILE		*fp;

	if (cf->mode != R_MODE) {
	    return cf->atFirstChar;
	}

	fp = cf->fp;

	if (cf->pendingCount)
	    return 0;
	if (cf->atFirstChar)
	    return 1;

	do {
	    c = getc(fp);
	} while (c == ' ' || c == '\t');

	if (c == '\n') {
	    cf->atFirstChar = 1;
	    return 1;
	}

	ungetc(c, fp);
	return 0;
}

/*
* cf_getLong: Return the next number on the input.  If next input is not
*	a number, skip the whole string and return 0.
*/
long
cf_getLong(cf)
struct cfile	*cf;
{
    register int	value;
    register int	c;
    int			sign;
    FILE		*fp;

    if (cf->pendingCount) {
	--cf->pendingCount;
	return cf->pendingValue;
    }

    if (cf->atFirstChar) return 0; /* short cut. */

    fp = cf->fp;

    do {
	c = getc(fp);
    } while (c == ' ' || c == '\t');

    if (c == '-') {
	sign = -1;
	c = getc(fp);
    } else {
	sign = 1;
    }

    while (c == '0') c = getc(fp);

    value = 0;

    if (isdigit(c)) {
	value = c - '0';
	c = getc(fp);
	while (isdigit(c)) {
	    value = (value << 3) + (value << 1);
	    value += c - '0';
	    c = getc(fp);
	}
    }

    if (sign == -1) value = -value;
    cf->pendingValue = value;

    /*
    * Repeat count present.  Format m!n means n repeats of m.
    */
    if (c == '!') {
	value = 0;
	c = getc(fp);
	if (isdigit(c)) {
	    value = c - '0';
	    c = getc(fp);
	    while (isdigit(c)) {
		value = (value << 3) + (value << 1);
		value += c - '0';
		c = getc(fp);
	    }
	    cf->pendingCount = value - 1;
	}
    }

    while (c != EOF && (!isspace(c)))	/* swallow trailing stuff */
	c = getc(fp);

    if (c == '\n')			/* end of line ? */
	cf->atFirstChar = 1;

    return cf->pendingValue;
}

void
cf_putFirstChar(cf, c)
struct cfile	*cf;
int		c;
{
    register int	r;

    if (cf->mode != W_MODE) {
	fprintf(stderr, "%s: not open for writing\n", cf->fileName);
	exit(1);
    }

    r = 0;
    if (!cf->atFirstChar)
	cf_putNewline(cf);

    r |= putc(c, cf->fp);

    if (r == EOF) {
	perror(cf->fileName);
	exit(1);
    }

    cf->atFirstChar = 0;
}

void
cf_putNewline(cf)
struct cfile	*cf;
{
    register int	r;
    FILE		*fp;

    r = 0;

    if (cf->mode != W_MODE) {
	fprintf(stderr, "%s: not open for writing\n", cf->fileName);
	exit(1);
    }
    fp = cf->fp;

    if (cf->pendingCount) {
	if (cf->pendingValue) {
	    r |= putLong(fp, cf->pendingValue);
	    if (cf->pendingCount > 1) {
		if (cf->pendingCount == 2 &&
		    cf->pendingValue < 10 && cf->pendingValue >= 0)
		{
		    r |= putc(' ', fp);
		    r |= putc(cf->pendingValue + '0', fp);
		} else {
		    r |= putc('!', fp);
		    r |= putLong(fp, cf->pendingCount);
		}
	    }
	}
	cf->pendingCount = 0;
    }

    r |= putc('\n', fp);
    if (r == EOF) {
	perror(cf->fileName);
	exit(1);
    }

    ++cf->lineNo;
    cf->atFirstChar = 1;
}

void
cf_putString(cf, s)
struct cfile	*cf;
char		*s;
{
    register int	r;
    FILE		*fp;
    register char	*p;
    int			c;

    r = 0;

    if (cf->mode != W_MODE) {
	fprintf(stderr, "%s: not open for writing\n", cf->fileName);
	exit(1);
    }
    fp = cf->fp;

    if (cf->atFirstChar) {	/* If no key, make it blank */
	cf->atFirstChar = 0;
	r |= putc(' ', fp);
    }

    if (cf->pendingCount) {
	r |= putLong(fp, cf->pendingValue);
	if (cf->pendingCount > 1) {
	    if (cf->pendingCount == 2 &&
		cf->pendingValue < 10 && cf->pendingValue >= 0)
	    {
		r |= putc(' ', fp);
		r |= putc(cf->pendingValue + '0', fp);
	    } else {
		r |= putc('!', fp);
		r |= putLong(fp, cf->pendingCount);
	    }
	}
	cf->pendingCount = 0;
	r |= putc(' ', fp);
    }

    p = s;
    while ((c = *p++) != '\0') {
	r |= putc(c, fp);
    }
    r |= putc(' ', fp);
    if (r == EOF) {
	perror(cf->fileName);
	exit(1);
    }
}

void
cf_putLong(cf, n)
struct cfile	*cf;
long		n;
{
    register int	r;
    FILE		*fp;


    if (cf->mode != W_MODE) {
	fprintf(stderr, "%s: not open for writing\n", cf->fileName);
	exit(1);
    }
    fp = cf->fp;

    if (cf->atFirstChar) {	/* If no key, make it blank */
	cf->atFirstChar = 0;
	r |= putc(' ', fp);
    }

    if (cf->pendingCount) {
	if (cf->pendingValue == n) {
	    ++cf->pendingCount;
	    cf->atFirstChar = 0;
	    return;
	}
	r = 0;
	if (cf->pendingCount <= 1) {
	    r |= putLong(fp, cf->pendingValue);
	} else {
	    if (cf->pendingValue != 0) {
		r |= putLong(fp, cf->pendingValue);
	    }
	    if (cf->pendingCount == 2 &&
		cf->pendingValue < 10 && cf->pendingValue > 0)
	    {
		r |= putc(' ', fp);
		r |= putc(cf->pendingValue + '0', fp);
	    } else {
		r |= putc('!', fp);
		r |= putLong(fp, cf->pendingCount);
	    }
	}
	r |= putc(' ', fp);
	if (r == EOF) {
	    perror(cf->fileName);
	    exit(1);
	}
    }

    cf->pendingCount = 1;
    cf->pendingValue = n;

    cf->atFirstChar = 0;
}
