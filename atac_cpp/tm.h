#ifndef tm_H
#define tm_H
static char tm_h[] = 
	"$Header: /users/source/archives/atac.vcs/atac_cpp/RCS/tm.h,v 3.0 1992/11/06 07:47:00 saul Released $";
/*
*-----------------------------------------------$Log: tm.h,v $
*-----------------------------------------------Revision 3.0  1992/11/06 07:47:00  saul
*-----------------------------------------------FROM_KEYS
*-----------------------------------------------
*Revision 3.0  92/11/06  07:47:00  saul
*propagate to version 3.0
*
*Revision 2.6  92/10/28  09:12:24  saul
*remove TARGET_MACHINE symbols and CPP_PREDEFINES
*
*Revision 2.5  92/06/17  11:17:33  saul
*Added m88k_unix and m88k_sysV
*
*Revision 2.4  92/04/29  08:45:23  saul
*Add _BSD_INCLUDES for AIX (so sys/time.h includes time.h).
*
*Revision 2.3  92/04/08  15:17:58  saul
*change 3b to _3b (some cpp's don't like it)
*
*Revision 2.2  92/04/08  11:51:19  saul
*add __builtin_alloca for sparc
*
*Revision 2.1  92/04/06  13:40:46  saul
*GNU version 1.40 + ATAC_EXPAND + ATAC_LINENO
*
*-----------------------------------------------end of log
*/
/* Definitions of target machine for GNU compiler.
   Copyright (C) 1990 Free Software Foundation, Inc.

This file is part of GNU CC.

GNU CC is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 1, or (at your option)
any later version.

GNU CC is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with GNU CC; see the file COPYING.  If not, write to
the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.  */

/* Define results of standard character escape sequences.  */
#define TARGET_BELL 007
#define TARGET_BS 010
#define TARGET_TAB 011
#define TARGET_NEWLINE 012
#define TARGET_VT 013
#define TARGET_FF 014
#define TARGET_CR 015

#define BITS_PER_UNIT 8

#endif /* tm_H */
