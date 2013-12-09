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
#ifndef srcpos_H
#define srcpos_H
static const char srcpos_h[] = "$Id: srcpos.h,v 3.4 2013/12/08 22:04:21 tom Exp $";
/*
* @Log: srcpos.h,v @
* Revision 3.3  1997/12/09 00:46:10  tom
* add 'filestamp()' prototype
*
* Revision 3.2  1996/11/13 00:25:43  tom
* change ident to 'const' to quiet gcc
*
* Revision 3.1  94/04/04  10:14:21  jrh
* Add Release Copyright
* 
* Revision 3.0  92/11/06  07:45:38  saul
* propagate to version 3.0
* 
* Revision 2.2  92/03/17  14:22:54  saul
* copyright
* 
* Revision 2.1  91/06/13  12:39:20  saul
* Propagate to version 2.0
* 
* Revision 1.1  91/06/12  20:25:51  saul
* Aug 1990 baseline
* 
*-----------------------------------------------end of log
*/
typedef struct srcpos {
    short file;
    short col;
    int line;
} SRCPOS;

/* filestamp.c */
extern int filestamp(char *path);

#endif /* srcpos_H */
