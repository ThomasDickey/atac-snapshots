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
#ifndef disp_H
#define disp_H
static char disp_h[] = "$Header: /users/source/archives/atac.vcs/atacysis/RCS/disp.h,v 3.1 1994/04/04 10:25:06 jrh Exp $";
/*
*-----------------------------------------------$Log: disp.h,v $
*-----------------------------------------------Revision 3.1  1994/04/04 10:25:06  jrh
*-----------------------------------------------FROM_KEYS
*-----------------------------------------------
* Revision 3.1  94/04/04  10:25:06  jrh
* Add Release Copyright
* 
* Revision 3.0  92/11/06  07:48:05  saul
* propagate to version 3.0
* 
* Revision 2.4  92/09/08  08:45:52  saul
* Add srcfile_name() dcl.
* 
* Revision 2.3  92/03/17  15:27:07  saul
* copyright
* 
* Revision 2.2  91/12/13  09:25:11  saul
* add MAX_SRCFILE_NAME constant for same length file name bug
* 
* Revision 2.1  91/06/19  13:09:59  saul
* Propagate to version 2.0
* 
* Revision 1.1  91/06/12  16:58:22  saul
* Aug 1990 baseline
* 
*-----------------------------------------------end of log
*/
#define MAX_SRCFILE_NAME	256

#define DISP_HILI	1
#define DISP_CLEAR	2
#define DISP_NEWLINE	4
#define DISP_INDENT	8
#define DISP_UNINDENT	16

char *srcfile_name();

#endif /* disp_H */
