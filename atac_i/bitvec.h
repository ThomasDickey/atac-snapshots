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
#ifndef bitvector_H
#define bitvector_H
static const char bitvector_h[] = "$Id: bitvec.h,v 3.4 2013/12/08 22:04:06 tom Exp $";
/*
* @Log: bitvec.h,v @
* Revision 3.3  1996/11/13 00:24:05  tom
* change ident to 'const' to quiet gcc
*
* Revision 3.2  94/04/04  10:12:00  jrh
* Add Release Copyright
*
* Revision 3.1  93/07/12  09:47:24  saul
* Name changed from bitvector.h for MVS.
*
* Revision 3.0  92/11/06  07:46:04  saul
* propagate to version 3.0
* 
* Revision 2.2  92/03/17  14:22:16  saul
* copyright
* 
* Revision 2.1  91/06/13  12:38:56  saul
* Propagate to version 2.0
* 
* Revision 1.1  91/06/12  20:25:35  saul
* Aug 1990 baseline
* 
*-----------------------------------------------end of log
*/
#define BITSPW		32	/* Bits Per Word */
#define LBITSPW		5	/* Log Bits Per Word */
#define LBYTESPW	2	/* Log Bytes Per Word */
#define WORDS_FOR_BITS(n)	(((n) + BITSPW - 1) >> LBITSPW)
#define BYTES_FOR_BITS(n)	(WORDS_FOR_BITS(n) << LBYTESPW)
typedef unsigned long BVPTR;
#define BVALLOC(n)	(BVPTR *)BVCLRALL(malloc(BYTES_FOR_BITS(n)), (n))
#define BVDCL(s,n)	BVPTR (s)[WORDS_FOR_BITS(n)]
#define BVTEST(s,n)	((s)[(n)>>LBITSPW] & (1 << ((n) & (BITSPW - 1))))
#define BVSET(s,n)	((s)[(n)>>LBITSPW] |= (1 << ((n) & (BITSPW - 1))))
#define BVCLR(s,n)	((s)[(n)>>LBITSPW] &= ~(1 << ((n) & (BITSPW - 1))))
#define BVCLRALL(s,n)	memset((s),0,BYTES_FOR_BITS(n))
#define BVSETALL(s,n)	memset((s),-1,BYTES_FOR_BITS(n))
#define BVCPY(t,s,n)	memcpy((t),(s),BYTES_FOR_BITS(n))
#define BVCMP(t,s,n)	memcmp((t),(s),BYTES_FOR_BITS(n))
#endif /* bitvector_H */
