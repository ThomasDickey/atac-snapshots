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
static char const disp_h[] =
"$Header: /users/source/archives/atac.vcs/atacysis/RCS/disp.h,v 3.2 1995/12/27 19:32:48 tom Exp $";
/*
* $Log: disp.h,v $
* Revision 3.2  1995/12/27 19:32:48  tom
* adjust headers, prototyped for autoconfig
*
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

/* interface of 'disp.c' */
extern void disp_str P_((char *str, int attributes));
extern void disp_file
	P_((char *filename, int f_line, int f_col, int t_line, int t_col, int
	attributes));
extern void disp_end P_((void));
extern void disp_elipsis P_((int nSkipped));
extern void disp_title P_((char *title, int startLine, int endLine));
extern int disp_windowSize P_((void));

/* interface of 'srcfile_name.c' */
extern char *srcfile_name P_((char *srcfile, time_t *chgtime, char *atacfile));


#endif /* disp_H */
