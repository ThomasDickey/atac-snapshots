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
#ifndef man_H
#define man_H
static const char man_h[] = 
	"$Header: /users/source/archives/atac.vcs/atacysis/RCS/man.h,v 3.2 1996/11/13 01:29:27 tom Exp $";
/*
* $Log: man.h,v $
* Revision 3.2  1996/11/13 01:29:27  tom
* change ident to 'const' to quiet gcc
*
* Revision 3.1  94/04/04  10:25:39  jrh
* Add Release Copyright
*
* Revision 3.0  92/11/06  07:47:22  saul
* propagate to version 3.0
*
* Revision 2.4  92/10/30  10:24:25  saul
* Don't define TRUE/FALSE (defined in portable.h)
*
* Revision 2.3  92/09/08  10:21:26  saul
* Missing #endif.
*
* Revision 2.2  92/09/08  10:12:27  saul
* changed trace format and data structures
*
* Revision 2.1  92/09/08  10:02:40  saul
* Purdue trace management
*
*-----------------------------------------------end of log
*/
#define	BUFFER_SIZE 1024

#define	FILE_POOL_SIZE		(4)
#define	FUNC_POOL_SIZE		(10)
#define	BLOCK_POOL_SIZE		(50)
#define	VAR_POOL_SIZE		(15)
#define	HEADER_POOL_SIZE	(5)
#define	SOURCE_POOL_SIZE	(5)
#define	CUSE_POOL_SIZE		(10)
#define	PUSE_POOL_SIZE		(10)

#define	TEST_POOL_SIZE		(25)

#define	FAMILY_POOL_SIZE	(5)
#define	MEMBER_POOL_SIZE	(25)

#define	MAP_POOL_SIZE 		(5)
#endif /* man_H */
