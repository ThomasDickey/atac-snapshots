/****************************************************************
Copyright (c) 1993 Bell Communications Research, Inc. (Bellcore)

Permission to use, copy, modify, and distribute this material
for any purpose and without fee is hereby granted, provided
that the above copyright notice and this permission notice
appear in all copies, and that the name of Bellcore not be
used in advertising or publicity pertaining to this
material without the specific, prior written permission
of an authorized representative of Bellcore.  BELLCORE
MAKES NO REPRESENTATIONS ABOUT THE ACCURACY OR SUITABILITY
OF THIS MATERIAL FOR ANY PURPOSE.  IT IS PROVIDED "AS IS",
WITHOUT ANY EXPRESS OR IMPLIED WARRANTIES.
****************************************************************/
#ifndef portable_H
#define portable_H

#if __STDC__
#define P_(p) p
extern	int main(int, char **);
#else	/* assume K&R */
#define P_(p) ()
#define const
#endif

static const char portable_h[] = 
	"$Header: /users/source/archives/atac.vcs/RCS/portable.h,v 3.14 1997/05/12 00:19:04 tom Exp $";
/*
* Copyright @ 1992 Bell Communications Research, Inc. All Rights Reserved.
*$Log: portable.h,v $
*Revision 3.14  1997/05/12 00:19:04  tom
*correct sign in LURSHIFT
*
*Revision 3.13  1995/12/27 19:48:27  tom
*define P_ macro
*
*Revision 3.12  94/08/08  13:12:09  saul
*Add linux section.
*
*Revision 3.11  94/04/04  09:51:17  jrh
*Add Release Copyright
*
*Revision 3.10  94/03/21  07:54:53  saul
*fix MAXPATHLEN for portability
*get rid of silly and useless stuff
*
*Revision 3.9  93/09/24  13:23:56  saul
*Add hp unix entry
*
*Revision 3.8  93/08/11  13:00:46  saul
*fix SYSV stuff
*
*Revision 3.7  93/08/11  10:04:30  saul
*fix ifdefs (again)
*
*Revision 3.6  93/08/04  15:32:01  ewk
*Changes for MVS and solaris support.
*
*Revision 3.5  93/07/09  14:28:59  saul
*new entries for MVS and VMS.
*
*Revision 3.4  93/04/02  10:31:49  saul
*Fix handling of missing memcpy, memset for pyramid
*
*Revision 3.3  93/03/31  11:37:28  saul
*Add PYRAMID section.
*Define MEMSET and MEMCPY.  Macros on pyramid.  memcpy and memset elsewhere.
*
*Revision 3.2  92/12/30  07:51:13  saul
*Don't use __builtin_alloca for sun3
*
*Revision 3.1  92/11/11  07:17:57  saul
*remove ATEXIT_SUPPORT from non BSD and UTS
*
*Revision 3.0  92/11/06  07:35:04  saul
*propagate to version 3.0
*
*Revision 2.3  92/11/03  10:10:15  saul
*change bool to boolean to avoid conflict with curses.h
*
*Revision 2.2  92/11/03  08:01:37  saul
*service specific ifdefs and local preference mods
*
*Revision 2.1  92/11/02  13:36:42  saul
*Rabbit Software 1987 (from printed copy)     
*
*-----------------------------------------------end of log
*/
/*
* portable.h -- type and macro definitions for portable C
*
* Include after system include files but before user include files.
*
* Copied from "Portable C and Unix System Programming" by J.E.Lapin,
* Rabbit Software, Prentice-Hall, 1987, Appendix A.
*
*/

#ifndef EOF
#include <stdio.h>			/* Used to check for system V */
#endif

/* Standard types */
typedef int		boolean;	/* >= 16 bits used as boolean	*/
typedef char		flag;		/* >= 8  bits used as boolean	*/

/* Define byte and BYTE to portably support an unsigned
   8 bit data type (see Section 2.9.1 in text).				*/

/*	If char is signed by default and unsigned keyword is
	not allowed with char, byte and BYTE are:			*/
#define MAXCHAR		0x7f		/* largest character value	*/
typedef char		byte;		/* 8 bit unsigned type		*/
#define BYTE(x)		((x) & 0xff)	/* BYTE truncated data		*/

/* Standard macros */
#define HIBYTE(x)	(((x) >> 8) & 0xff)	/* hi byte of short	*/
#define LOBYTE(x)	((x) & 0xff)		/* lo byte of short	*/
#define HIWORD(x)	(((x) >> 16) & 0xffffL)	/* upper half of long	*/
#define LOWORD(x)	((x) & 0xffffL)		/* lower half of long	*/
#define CHAR(x)		((x) & 0x7f)		/* Truncate to 7 bits	*/
#define WORD(x)		((x) & 0xffffL)		/* Truncate to 16 bits	*/
#define DECODE(x)	((int) ((x)-'0'))	/* int value of a digit	*/

/* long unsigned right shift */
#define LURSHIFT(n, b)	(((unsigned long)(n) >> (b)) & (0x7fffffffL >> ((b)-1)))

/* number of elements in array a */
#define DIM(a)		(sizeof(a) /sizeof(*(a)))

/* Scope control psuedo-keywords */
#define global

/* Standard constants */
#ifndef TRUE
#define TRUE	1		/* for use with booleans	*/
#endif

#ifndef FALSE
#define FALSE	0
#endif

#define SUCCEED		0		/* for use in exit()		*/
#define FAIL		(-1)		/* for exit() & error returns	*/

/*
* AIX
*/
#if AIX || _AIX || __AIX__ || _AIX32 || IBMR2
#define portable_h_AIX 1		/* for debugging */
#define SET_unix 1
#define FORK_SUPPORT 1
#define ATEXIT_SUPPORT 1
#define _EXIT_SUPPORT 1
#define RENAME_SUPPORT 1
#define SYS_RESOURCE_H_SUPPORT 1
#define GETRLIMIT_SUPPORT 1
#define TMP_MAXPATHLEN	1024
#define TMP_MAXNAMLEN	255
#else /* end AIX */

/*
* sun (includes sparc)
*/
#if sun || sun3 || sun386 || sparc
#define portable_h_SUN 1		/* for debugging */
#define SET_unix 1
#define FORK_SUPPORT 1
#define ON_EXIT_SUPPORT 1
#define _EXIT_SUPPORT 1
#define RENAME_SUPPORT 1
#define SYS_RESOURCE_H_SUPPORT 1
#define GETRLIMIT_SUPPORT 1
#define TMP_MAXPATHLEN	1024
#define TMP_MAXNAMLEN	255
#ifdef _SIZE_T
#define solaris 1
#endif /* _SIZE_T */
#ifdef solaris
#define portable_h_SOLARIS 1		/* for debugging */
#undef ON_EXIT_SUPPORT
#define ATEXIT_SUPPORT 1
#define index strchr
#define rindex strrchr
#endif /* solaris */
#define portable_h_SUN 1		/* for debugging */
#else /* end SUN */

/*
* linux
*/
#if linux
#define portable_h_LINUX 1		/* for debugging */
#define SET_unix 1
#define FORK_SUPPORT 1
#define ON_EXIT_SUPPORT 1
#define _EXIT_SUPPORT 1
#define RENAME_SUPPORT 1
#define SYS_RESOURCE_H_SUPPORT 1
#define GETRLIMIT_SUPPORT 1
#define TMP_MAXPATHLEN	1024
#define TMP_MAXNAMLEN	255
#define portable_h_SUN 1		/* for debugging */
#else /* end LINUX */
/*
* HP unix
*/
#ifdef hpux
#define portable_h_hpux 1		/* for debugging */
#define SET_unix 1
#define FORK_SUPPORT 1
#define ATEXIT_SUPPORT 1
#define _EXIT_SUPPORT 1
#define RENAME_SUPPORT 1
#define SYS_RESOURCE_H_SUPPORT 1
#define GETRLIMIT_SUPPORT 1
#define TMP_MAXPATHLEN	1024
#define TMP_MAXNAMLEN	255
#else /* end hpux */

/*
* ultrix
*/
#ifdef ultrix
#define portable_h_ULTRIX 1		/* for debugging */
#define SET_unix 1
#define FORK_SUPPORT 1
#define ATEXIT_SUPPORT 1
#define _EXIT_SUPPORT 1
#define RENAME_SUPPORT 1
#define SYS_RESOURCE_H_SUPPORT 1
#define GETRLIMIT_SUPPORT 1
#define TMP_MAXPATHLEN	1024
#define TMP_MAXNAMLEN	255
#else /* end ULTRIX */

/*
* UTS
*/
#ifdef UTS
#define portable_h_UTS 1		/* for debugging */
#define SET_unix 1
#define FORK_SUPPORT 1
#define _EXIT_SUPPORT 1
#define SYS_RESOURCE_H_SUPPORT 1
#define index strchr
#define rindex strrchr
#ifndef __STDC__
#define void int
#endif /* __STDC__ */
#define TMP_MAXPATHLEN	64	/* ? */
#define TMP_MAXNAMLEN	14	/* ? */
#else /* end UTS */

/*
* PYRAMID
*/
#ifdef pyr
#define portable_h_PYRAMID 1		/* for debugging */
#define SET_unix 1
#define FORK_SUPPORT 1
#define _EXIT_SUPPORT 1
#ifndef __STDC__
#define void int
#endif /* __STDC__ */
#define TMP_MAXPATHLEN	1024	/* ? */
#define TMP_MAXNAMLEN	255	/* ? */
#else /* end PYRAMID */

/*
* VMS
*/
#ifdef vms
#define portable_h_VMS 1		/* for debugging */
#define _EXIT_SUPPORT 1
#define index strchr
#define rindex strrchr
#define TMP_MAXPATHLEN	1024	/* ? */
#define TMP_MAXNAMLEN	255	/* ? */
#else /* end VMS */

/*
* MVS
*/
#ifdef MVS
#define portable_h_MVS 1		/* for debugging */
#undef MAXCHAR
#define MAXCHAR		0xff		/* largest character value	*/
#undef CHAR
#define CHAR(x)		((x) & 0xff)		/* Truncate to 8 bits on MVS */
#define ATEXIT_SUPPORT 1
#define index strchr
#define rindex strrchr
#define TMP_MAXPATHLEN	1024	/* ? */
#define TMP_MAXNAMLEN	255	/* ? */
#else /* end MVS */

/*
* other BSD
*/
#ifndef L_ctermid
#ifndef BSD
#define BSD 1
#endif /* BSD */
#endif /* L_ctermid */
#if BSD || SYSTYPE_BSD || __SYSTYPE_BSD || __bsd4_2 || bsd43 || bsd4_2
#define portable_h_BSD 1		/* for debugging */
#define SET_unix 1
#define FORK_SUPPORT 1
#define _EXIT_SUPPORT 1
#define RENAME_SUPPORT 1
#define SYS_RESOURCE_H_SUPPORT 1
#define GETRLIMIT_SUPPORT 1
#define TMP_MAXPATHLEN	1024
#define TMP_MAXNAMLEN	255
#else /* end other BSD */

/*
* Other
*/
#define portable_h_Other 1		/* for debugging */
#define FORK_SUPPORT 1
#define _EXIT_SUPPORT 1
#define index strchr
#define rindex strrchr
#define TMP_MAXPATHLEN	1024	/* ? */
#define TMP_MAXNAMLEN	255	/* ? */
#ifndef __STDC__
#define void int
#endif /* __STDC__ */

#endif /* not other BSD */
#endif /* not MVS */
#endif /* not VMS */
#endif /* not PYRAMID */
#endif /* not UTS */
#endif /* not ULTRIX */
#endif /* not HPUX */
#endif /* not LINUX */
#endif /* not SUN */
#endif /* not AIX */

/*
* set unix
*/
#if SET_unix
#ifndef unix
#define unix 1
#endif /* unix */
#endif /* SET_unix */
#undef SET_unix

/*
* ANSI C
*/
#ifdef __STDC__
#ifndef ATEXIT_SUPPORT
#define ATEXIT_SUPPORT 1
#undef ON_EXIT_SUPPORT
#endif /* __STDC__ */
#endif

typedef char		path_t[TMP_MAXPATHLEN];
#undef TMP_MAXPATHLEN

typedef char		filename_t[TMP_MAXNAMLEN + 1];
#undef TMP_MAXNAMLEN

/* portable.h ends here */
#endif /* portable_H */
