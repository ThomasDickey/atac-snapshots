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

static char version_c[] = 
	"$Header: /users/source/archives/atac.vcs/atac_cpp/RCS/version.c,v 3.2 1994/04/04 10:22:41 jrh Exp $";
/*
*$Log: version.c,v $
*Revision 3.2  1994/04/04 10:22:41  jrh
*FROM_KEYS
*
* Revision 3.2  94/04/04  10:22:41  jrh
* Add Release Copyright
* 
* Revision 3.1  93/08/04  15:41:20  ewk
* Added MVS and solaris support.  Squelched some ANSI warnings.
* 
* Revision 3.0  92/11/06  07:46:50  saul
* propagate to version 3.0
* 
* Revision 2.2  92/04/06  12:48:36  saul
* GNU version 1.40 + ATAC_EXPAND + ATAC_LINENO
* 
* Revision 2.1  91/06/19  13:45:45  saul
* Propagte to version 2.0
* 
* Revision 1.1  91/06/12  20:38:08  saul
* Aug 1990 baseline
* 
*-----------------------------------------------end of log
*/
char *version_string = "1.40";
