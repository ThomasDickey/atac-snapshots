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

static char error_c[] = 
	"$Header: /users/source/archives/atac.vcs/atacysis/RCS/error.c,v 3.2 1994/04/04 10:25:12 jrh Exp $";
/*
*-----------------------------------------------$Log: error.c,v $
*-----------------------------------------------Revision 3.2  1994/04/04 10:25:12  jrh
*-----------------------------------------------FROM_KEYS
*-----------------------------------------------
*Revision 3.2  94/04/04  10:25:12  jrh
*Add Release Copyright
*
*Revision 3.1  93/08/04  15:53:46  ewk
*Added MVS and solaris support.  Squelched some ANSI warnings.
*
* Revision 3.0  92/11/06  07:47:13  saul
*propagate to version 3.0
*
*Revision 2.2  92/10/30  09:54:11  saul
*include portable.h
*
*Revision 2.1  92/09/08  08:37:21  saul
*New error routines.
*
*-----------------------------------------------end of log
*/
#include <stdio.h>
#include "portable.h"

/* forward declarations */
void trace_error();
void internal_error();

void
internal_error(s, arg1, arg2)
char	*s;
char	*arg1;
char	*arg2;
{
	fprintf(stderr, s, arg1, arg2);
	exit(1);
}

void
trace_error(filename, recno)
char	*filename;
int	 recno;
{
	fprintf(stderr, "%s: corrupted trace line: %d\n", filename, recno);
	exit(1);
}
