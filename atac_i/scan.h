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
#ifndef scan_H
#define scan_H
static const char scan_h[] = "$Id: scan.h,v 3.4 2013/12/08 22:04:16 tom Exp $";
/*
* @Log: scan.h,v @
* Revision 3.3  1997/05/10 20:40:39  tom
* add prototypes for scan.c
*
* Revision 3.2  1996/11/13 00:26:12  tom
* change ident to 'const' to quiet gcc
*
* Revision 3.1  94/04/04  10:14:07  jrh
* Add Release Copyright
* 
* Revision 3.0  92/11/06  07:45:37  saul
* propagate to version 3.0
* 
* Revision 2.3  92/09/16  07:36:14  saul
* Get rid of unused tokval field.
* 
* Revision 2.2  92/03/17  14:22:50  saul
* copyright
* 
* Revision 2.1  91/06/13  12:39:18  saul
* Propagate to version 2.0
* 
* Revision 1.1  91/06/12  20:25:49  saul
* Aug 1990 baseline
* 
*-----------------------------------------------end of log
*/
typedef struct tokenvalue {
    char *text;
    SRCPOS srcpos[2];
} TOKENVALUE;

/* scan.c */
void scan_setType(char *name);
void scan_pushScope(void);
int scan_popScope(void);
void scan_init(FILE *srcfile);
void scan_end(char **uprefix);

#endif /* scan_H */
