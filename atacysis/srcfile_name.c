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
#ifdef vms
#include <types.h>
#else /* not vms */
#ifdef MVS
#include <time.h>		/* for time_t */
#else /* not MVS */
#include <sys/types.h>		/* for time_t */
#endif /* not MVS */
#endif /* not vms */

#include "version.h"
#include "portable.h"
#include "disp.h"

static char const srcfile_name_c[] = 
	"$Header: /users/source/archives/atac.vcs/atacysis/RCS/srcfile_name.c,v 3.5 1995/12/27 20:07:20 tom Exp $";
/*
* $Log: srcfile_name.c,v $
* Revision 3.5  1995/12/27 20:07:20  tom
* adjust headers, prototyped for autoconfig
*
*Revision 3.4  94/04/04  10:26:17  jrh
*Add Release Copyright
*
*Revision 3.3  93/08/10  14:51:00  ewk
*Fixed definition of time_t for vsm, MVS, and unix.
*
*Revision 3.2  93/08/04  15:58:28  ewk
*Added MVS and solaris support.  Squelched some ANSI warnings.
*
*Revision 3.1  93/07/09  14:02:06  saul
**** empty log message ***
*
*Revision 3.0  92/11/06  07:47:37  saul
*propagate to version 3.0
*
*Revision 2.3  92/10/30  09:55:37  saul
*include portable.h
*
*Revision 2.2  92/10/08  10:07:58  saul
* change file time stamp checking to work with compression
*
*Revision 2.1  92/09/08  09:12:34  saul
*Src file time stamp checking.
*
*-----------------------------------------------end of log
*/

char *
srcfile_name(srcfile, chgtime, atacfile)
char *srcfile;
char *atacfile;
time_t *chgtime;
{
	static char buf[MAX_SRCFILE_NAME];
	static char errMsg[80];
	char	*p;
	char	*s;
	char	*b;
	time_t	tExpected;
	time_t	tActual;

	if (*srcfile == '/') b = srcfile;
	else {

	    p = atacfile + strlen(atacfile);
	    while (--p >= atacfile) {
		if (*p == '/') break;
	    }

	    if (p == atacfile) return srcfile;
	    else {

		b = buf + sizeof buf;

		s = srcfile + strlen(srcfile);
		while (s >= srcfile) {
		    if (b < buf) break;
		    *--b = *s--;
		}

		while (p >= atacfile) {
		    if (b <= buf) break;
		    *--b = *p--;
		}
	    }
	}

	tExpected = *chgtime;
	if (tExpected == 0) {
	    return b;
	}

	tActual = (time_t)filestamp(b);
	if (tActual != tExpected) {
	    sprintf(errMsg, "! ! ! ! WARNING %s has been modified ! ! ! !", b);
	    disp_str(errMsg, DISP_CLEAR | DISP_HILI | DISP_NEWLINE);
	    *chgtime = 0;
	}
	return b;
}
