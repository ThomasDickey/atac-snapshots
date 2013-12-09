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

#ifdef MVS
#include <mvapts.h>
MODULEID(%M%,%J%/%D%/%T%)
#endif /* MVS */

static const char filestamp_c[] = "$Id: filestamp.c,v 3.6 2013/12/08 18:25:41 tom Exp $";
/*
* @Log: filestamp.c,v @
* Revision 3.5  1997/12/09 00:46:12  tom
* move 'filestamp()' prototype to srcpos.h
*
* Revision 3.4  1996/11/13 00:42:11  tom
* change ident to 'const' to quiet gcc
* add forward-ref prototype
*
* Revision 3.3  94/04/04  10:13:06  jrh
* Add Release Copyright
* 
* Revision 3.2  93/08/04  15:45:51  ewk
* Added MVS and solaris support.  Squelched some ANSI warnings.
* 
* Revision 3.1  93/07/12  10:51:10  saul
* MVS MODULEID
* MVS dummy timestamp
* 
* Revision 3.0  92/11/06  07:46:06  saul
* propagate to version 3.0
* 
* Revision 2.4  92/10/30  09:48:19  saul
* include portable.h
* 
* Revision 2.3  92/03/17  14:22:29  saul
* copyright
* 
* Revision 2.2  91/06/13  12:46:00  saul
* add ifdef for _AIX
* 
* Revision 2.1  91/06/13  12:39:07  saul
* Propagate to version 2.0
* 
* Revision 1.1  91/06/12  20:25:42  saul
* Aug 1990 baseline
* 
*-----------------------------------------------end of log
*/
#include "portable.h"
#ifdef unix
#include <sys/types.h>
#include <sys/stat.h>
#else /* not unix */
#ifndef MVS
#include <stat.h>
#endif /* MVS */
#endif

#include "srcpos.h"

int
filestamp(char *path)
{
#ifdef MVS
    return 0;
#else /* not MVS */
    struct stat buf;

    if (stat(path, &buf) != 0)
	return 0;

    return (int) buf.st_mtime;
#endif /* not MVS */
}
