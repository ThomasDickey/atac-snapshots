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
#if HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef MVS
#include <mvapts.h>
MODULEID(%M%,%J%/%D%/%T%)
#endif /* MVS */

#include <stdio.h>

#include "portable.h"
#include "atacysis.h"
#include "ramfile.h"
#include "man.h"

static char const init_c[] = 
	"$Header: /users/source/archives/atac.vcs/atacysis/RCS/init.c,v 3.4 1996/11/13 01:31:25 tom Exp $";
/*
* $Log: init.c,v $
* Revision 3.4  1996/11/13 01:31:25  tom
* include <config.h> to declare 'const'
*
* Revision 3.3  1995/12/27 20:48:04  tom
* adjust headers, prototyped for autoconfig
*
* Revision 3.2  94/04/04  10:25:29  jrh
* Add Release Copyright
*
* Revision 3.1  93/08/04  15:54:59  ewk
* Added MVS and solaris support.  Squelched some ANSI warnings.
*
* Revision 3.0  92/11/06  07:47:18  saul
* propagate to version 3.0
*
* Revision 2.3  92/10/30  09:54:24  saul
* include portable.h
*
* Revision 2.2  92/09/08  10:12:12  saul
* changed trace format and data structures
*
* Revision 2.1  92/09/08  09:59:18  saul
* Purdue trace management
*
*-----------------------------------------------end of log
*/

void
init(tables)
tablestype *tables;
{
        tables->rf.files = NULL;
	tables->rf.iFilePool = 0;
	tables->rf.iFileCount = 0;
        tables->fams.families = NULL;
	tables->fams.iFamilyPool = 0;
	tables->fams.iFamilyCount = 0;
        tables->mems.members = NULL;
	tables->mems.iMemberPool = 0;
	tables->mems.iMemberCount = 0;
}
