#ifndef config_H
#define config_H
static char config_h[] = 
	"$Header: /users/source/archives/atac.vcs/atac_cpp/RCS/config.h,v 3.0 1992/11/06 07:46:52 saul Released $";
/*
*-----------------------------------------------$Log: config.h,v $
*-----------------------------------------------Revision 3.0  1992/11/06 07:46:52  saul
*-----------------------------------------------FROM_KEYS
*-----------------------------------------------
* Revision 3.0  92/11/06  07:46:52  saul
* propagate to version 3.0
* 
* Revision 2.5  92/11/02  11:59:48  saul
* let portable.h make portability decisions
* 
* Revision 2.4  92/10/30  10:53:55  saul
* #ifdef error
* 
* Revision 2.3  92/10/30  10:23:01  saul
* fix alloca ifdefs.  Remove TRUE/FALSE defs.
* 
* Revision 2.2  92/04/06  12:48:39  saul
* GNU version 1.40 + ATAC_EXPAND + ATAC_LINENO
* 
* Revision 2.1  91/06/19  13:45:41  saul
* Propagte to version 2.0
* 
* Revision 1.1  91/06/12  20:38:06  saul
* Aug 1990 baseline
* 
*-----------------------------------------------end of log
*/
/* Configuration for GNU C-compiler
   Copyright (C) 1987 Free Software Foundation, Inc.

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

/* This describes the machine the compiler is hosted on.  */
#define HOST_BITS_PER_CHAR 8
#define HOST_BITS_PER_SHORT 16
#define HOST_BITS_PER_INT 32
#define HOST_BITS_PER_LONG 32

/* target machine dependencies.
   tm.h is a symbolic link to the actual target specific file.   */
#include "tm.h"

/* Arguments to use with `exit'.  */
#define SUCCESS_EXIT_CODE 0
#define FATAL_EXIT_CODE 33

#endif /* config_H */
