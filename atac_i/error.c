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

static const char error_c[] = 
	"$Header: /users/source/archives/atac.vcs/atac_i/RCS/error.c,v 3.4 1996/11/13 00:42:43 tom Exp $";
/*
* $Log: error.c,v $
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
#include <stdio.h>
#include "portable.h"
#include "srcpos.h"

/* forward declarations */
/* FIXME: should use varargs */
#define ERR_ARGS SRCPOS *srcpos, char *msg, char *arg1, char *arg2, char *arg3
void parse_error P_(( ERR_ARGS ));
void lexical_error P_(( ERR_ARGS ));
void semantic_error P_(( ERR_ARGS ));
void internal_error P_(( ERR_ARGS ));
void supress_warnings P_(( void ));

#define PARSE_ERROR	2
#define INTERNAL_ERROR	3

static warn_flag = 1;

void
supress_warnings()
{
	warn_flag = 0;
}

void
internal_error(srcpos, msg, arg1, arg2, arg3)
char	*msg;
SRCPOS	*srcpos;
char	*arg1;
char	*arg2;
char	*arg3;
{
	fputs("internal error", stderr);
	if (msg) {
		fputs(": ", stderr);
		fprintf(stderr, msg, arg1, arg2, arg3);
	}
	if (srcpos) {
		fputs(" at ", stderr);
		print_srcpos(srcpos, stderr);
	}
	fputs("\n", stderr);

	exit(INTERNAL_ERROR);
}

void
semantic_error(srcpos, msg, arg1, arg2, arg3)
char	*msg;
SRCPOS	*srcpos;
char	*arg1;
char	*arg2;
char	*arg3;
{
	if (warn_flag == 0) return;

	fputs("semantic error", stderr);
	if (msg) {
		fputs(": ", stderr);
		fprintf(stderr, msg, arg1, arg2, arg3);
	}
	if (srcpos) {
		fputs(" at line ", stderr);
		print_srcpos(srcpos, stderr);
	}
	fputs("\n", stderr);

	return;
}

void
lexical_error(srcpos, msg, arg1, arg2, arg3)
char	*msg;
SRCPOS	*srcpos;
char	*arg1;
char	*arg2;
char	*arg3;
{
	if (warn_flag == 0) return;

	fputs("lexical error", stderr);
	if (msg) {
		fputs(": ", stderr);
		fprintf(stderr, msg, arg1, arg2, arg3);
	}
	if (srcpos) {
		fputs(" at line ", stderr);
		print_srcpos(srcpos, stderr);
	}
	fputs("\n", stderr);

	return;
}

void
parse_error(srcpos, msg, arg1, arg2, arg3)
char	*msg;
SRCPOS	*srcpos;
char	*arg1;
char	*arg2;
char	*arg3;
{
	fputs("parse error", stderr);
	if (msg) {
		fputs(": ", stderr);
		fprintf(stderr, msg, arg1, arg2, arg3);
	}
	if (srcpos) {
		fputs(" at line ", stderr);
		print_srcpos(srcpos, stderr);
	}
	fputs("\n", stderr);

	exit(PARSE_ERROR);
}
