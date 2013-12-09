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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

static const char error_c[] = "$Id: error.c,v 3.14 2013/12/09 01:38:46 tom Exp $";
/*
* @Log: error.c,v @
* Revision 3.10  2005/08/14 13:45:49  tom
* gcc warnings
*
* Revision 3.9  1998/09/19 15:27:18  tom
* change error-message format to put filename, line, col before the message
* to make it simpler to parse with vile's error-finder
*
* Revision 3.8  1997/12/10 01:51:44  tom
* ifdef'd to build with K&R compiler.
*
* Revision 3.7  1997/11/03 19:14:46  tom
* change type of internal_error() to int, since it is used in expression.
*
* Revision 3.6  1997/05/12 00:34:13  tom
* include tnode.h
*
* Revision 3.5  1997/05/10 22:15:42  tom
* rewrote using <stdarg.h> and vfprintf.
*
* Revision 3.4  1996/11/13 00:42:43  tom
* change ident to 'const' to quiet gcc
* add forward-ref prototypes
*
* Revision 3.3  94/04/04  10:12:37  jrh
* Add Release Copyright
* 
* Revision 3.2  93/08/04  15:44:50  ewk
* Added MVS and solaris support.  Squelched some ANSI warnings.
* 
* Revision 3.1  93/07/12  10:17:32  saul
* MVS MODULEID
* 
* Revision 3.0  92/11/06  07:45:31  saul
* propagate to version 3.0
* 
* Revision 2.3  92/10/30  09:48:09  saul
* include portable.h
* 
* Revision 2.2  92/03/17  14:22:24  saul
* copyright
* 
* Revision 2.1  91/06/13  12:39:02  saul
* Propagate to version 2.0
* 
* Revision 1.1  91/06/12  20:25:39  saul
* Aug 1990 baseline
* 
*-----------------------------------------------end of log
*/
#include "portable.h"

#include <stdarg.h>
#define VaStart(ap,arg) va_start(ap,arg)

#include "error.h"
#include "tnode.h"

#define PARSE_ERROR	2
#define INTERNAL_ERROR	3

static int warn_flag = 1;

void
supress_warnings(void)
{
    warn_flag = 0;
}

static void
any_error(SRCPOS * srcpos,
	  const char *label)
{
    if (srcpos) {
	print_srcpos(srcpos, stderr);
	fputs(", ", stderr);
    }
    fprintf(stderr, "ATAC %s", label);
}

#define MY_FUNC(func) func(SRCPOS *srcpos, const char *msg, ...)

int
MY_FUNC(internal_error)
{
    any_error(srcpos, "internal error");
    if (msg) {
	va_list ap;
	VaStart(ap, msg);
	fputs(": ", stderr);
	vfprintf(stderr, msg, ap);
	va_end(ap);
    }
    fputs("\n", stderr);

    exit(INTERNAL_ERROR);
    /*NOTREACHED */
}

void
MY_FUNC(semantic_error)
{
    if (warn_flag == 0)
	return;

    any_error(srcpos, "semantic error");
    if (msg) {
	va_list ap;
	VaStart(ap, msg);
	fputs(": ", stderr);
	vfprintf(stderr, msg, ap);
	va_end(ap);
    }
    fputs("\n", stderr);

    return;
}

void
MY_FUNC(lexical_error)
{
    if (warn_flag == 0)
	return;

    any_error(srcpos, "lexical error");
    if (msg) {
	va_list ap;
	VaStart(ap, msg);
	fputs(": ", stderr);
	vfprintf(stderr, msg, ap);
	va_end(ap);
    }
    fputs("\n", stderr);

    return;
}

void
MY_FUNC(parse_error)
{
    any_error(srcpos, "parse error");
    if (msg) {
	va_list ap;
	VaStart(ap, msg);
	fputs(": ", stderr);
	vfprintf(stderr, msg, ap);
	va_end(ap);
    }
    fputs("\n", stderr);

    exit(PARSE_ERROR);
}
