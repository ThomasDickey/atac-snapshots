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
#ifndef hook_H
#define hook_H
static const char hook_h[] = "$Id: hook.h,v 3.3 2013/12/08 22:02:11 tom Exp $";
/*
* @Log: hook.h,v @
* Revision 3.2  1996/11/13 00:23:16  tom
* change ident to 'const' to quiet gcc.
*
* Revision 3.1  94/04/04  10:13:13  jrh
* Add Release Copyright
* 
* Revision 3.0  92/11/06  07:45:34  saul
* propagate to version 3.0
* 
* Revision 2.2  92/03/17  14:22:31  saul
* copyright
* 
* Revision 2.1  91/06/13  12:39:07  saul
* Propagate to version 2.0
* 
 * Revision 1.1  91/06/12  20:25:44  saul
 * Aug 1990 baseline
 * 
*-----------------------------------------------end of log
*/
#define GEN_HOOK	200

#define HOOK_STMT_R	0
#define HOOK_STMT_L	1
#define HOOK_STMT_R_B	2
#define HOOK_STMT_L_B	3
#define HOOK_START	4
#define HOOK_EXPR_R	5
#define HOOK_EXPR_L	6
#define HOOK_EXPR_CAST	7
#define HOOK_TEMP	8
#endif /* hook_H */
