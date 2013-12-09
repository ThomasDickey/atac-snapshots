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

#include "config.h"
#include "portable.h"
#include "atacysis.h"

static char const error_c[] = "$Id: error.c,v 3.6 2013/12/08 23:55:14 tom Exp $";
/*
* @Log: error.c,v @
* Revision 3.4  2005/08/14 14:02:03  tom
* gcc warnings
*
* Revision 3.3  1995/12/27 20:10:09  tom
* adjust headers, prototyped for autoconfig
*
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

void
internal_error(const char *s,
	       const char *arg1,
	       const char *arg2)
{
    fprintf(stderr, s, arg1, arg2);
    exit(1);
}

void
trace_error(const char *filename,
	    int recno)
{
    fprintf(stderr, "%s: corrupted trace line: %d\n", filename, recno);
    exit(1);
}
