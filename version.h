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
#ifndef version_H
#define version_H
static char version_h[] = 
	"$Header: /users/source/archives/atac.vcs/RCS/version.h,v 3.2 1994/04/04 09:52:11 jrh Exp $";
/*
*-----------------------------------------------$Log: version.h,v $
*-----------------------------------------------Revision 3.2  1994/04/04 09:52:11  jrh
*-----------------------------------------------FROM_KEYS
*-----------------------------------------------
*Revision 3.2  94/04/04  09:52:11  jrh
*Add Release Copyright
*
*Revision 3.1  93/03/30  08:43:17  saul
*Change version number to 3.2.
*
*Revision 3.0  92/11/06  07:42:46  saul
*propagate to version 3.0
*
*Revision 2.3  92/09/30  11:26:29  saul
*Update VERSION number to 3.0
*
*Revision 2.2  92/09/16  08:30:33  saul
*atac 2.9
*
*Revision 2.1  92/09/08  08:08:34  saul
*New version information.
*
*-----------------------------------------------end of log
*/
/*
* Version number found in .atac on "v" record and in .trace files
* on "V" and "t" records.  Also compiled into executables as part of atac_rt.o.
*/
#define VERSION "3.2"

/*
* Runtime version found in .trace file on "s" line.  Also compiled into .o
* files.  Only changed when atac_rt modifications make it incompatible
*  with .o's from old instrumentations.
*/
#define RT_VERSION "3.0"
#endif /* version_H */
