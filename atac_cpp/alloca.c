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
static char alloca_c[] = 
	"$Header: /users/source/archives/atac.vcs/atac_cpp/RCS/alloca.c,v 3.2 1994/04/04 10:20:57 jrh Exp $";
/*
*-----------------------------------------------$Log: alloca.c,v $
*-----------------------------------------------Revision 3.2  1994/04/04 10:20:57  jrh
*-----------------------------------------------FROM_KEYS
*-----------------------------------------------
* Revision 3.2  94/04/04  10:20:57  jrh
* Add Release Copyright
* 
* Revision 3.1  93/08/04  15:35:47  ewk
* Added MVS and solaris support.  Squelched some ANSI warnings.
* 
* Revision 3.0  92/11/06  07:46:38  saul
* propagate to version 3.0
* 
* Revision 2.4  92/11/02  11:40:33  saul
* comment #endif tag for portability
* 
* Revision 2.3  92/10/30  10:24:03  saul
* Don't redefine NULL
* 
* Revision 2.2  92/04/06  12:47:41  saul
* GNU version 1.40 + ATAC_EXPAND + ATAC_LINENO
* 
* Revision 2.1  91/06/19  13:45:27  saul
* Propagte to version 2.0
* 
* Revision 1.1  91/06/12  20:38:01  saul
* Aug 1990 baseline
* 
*-----------------------------------------------end of log
*/
/*
	alloca -- (mostly) portable public-domain implementation -- D A Gwyn

	last edit:	86/05/30	rms
	   include config.h, since on VMS it renames some symbols.
	   Use xmalloc instead of malloc.

	This implementation of the PWB library alloca() function,
	which is used to allocate space off the run-time stack so
	that it is automatically reclaimed upon procedure exit, 
	was inspired by discussions with J. Q. Johnson of Cornell.

	It should work under any C implementation that uses an
	actual procedure stack (as opposed to a linked list of
	frames).  There are some preprocessor constants that can
	be defined when compiling for your specific system, for
	improved efficiency; however, the defaults should be okay.

	The general concept of this implementation is to keep
	track of all alloca()-allocated blocks, and reclaim any
	that are found to be deeper in the stack than the current
	invocation.  This heuristic does not reclaim storage as
	soon as it becomes invalid, but it will do so eventually.

	As a special case, alloca(0) reclaims storage without
	allocating any.  It is a good idea to use alloca(0) in
	your main control loop, etc. to force garbage collection.
*/
#ifndef lint
static char	SCCSid[] = "@(#)alloca.c	1.1";	/* for the "what" utility */
#endif

#ifdef emacs
#include "config.h"
#ifdef static
/* actually, only want this if static is defined as ""
   -- this is for usg, in which emacs must undefine static
   in order to make unexec workable
   */
#ifndef STACK_DIRECTION
you
lose
-- must know STACK_DIRECTION at compile-time
#endif /* STACK_DIRECTION undefined */
#endif /* static */
#endif /* emacs */

#ifdef X3J11
typedef void	*pointer;		/* generic pointer type */
#else
typedef char	*pointer;		/* generic pointer type */
#endif

#ifndef NULL
#define	NULL	0			/* null pointer constant */
#endif

/* forward declarations */
char *alloca();
static void find_stack_direction();

extern void	free();
extern pointer	xmalloc();

/*
	Define STACK_DIRECTION if you know the direction of stack
	growth for your system; otherwise it will be automatically
	deduced at run-time.

	STACK_DIRECTION > 0 => grows toward higher addresses
	STACK_DIRECTION < 0 => grows toward lower addresses
	STACK_DIRECTION = 0 => direction of growth unknown
*/

#ifndef STACK_DIRECTION
#define	STACK_DIRECTION	0		/* direction unknown */
#endif

#if STACK_DIRECTION != 0

#define	STACK_DIR	STACK_DIRECTION	/* known at compile-time */

#else	/* STACK_DIRECTION == 0; need run-time code */

static int	stack_dir;		/* 1 or -1 once known */
#define	STACK_DIR	stack_dir

static void
find_stack_direction (/* void */)
{
  static char	*addr = NULL;	/* address of first
				   `dummy', once known */
  auto char	dummy;		/* to get stack address */

  if (addr == NULL)
    {				/* initial entry */
      addr = &dummy;

      find_stack_direction ();	/* recurse once */
    }
  else				/* second entry */
    if (&dummy > addr)
      stack_dir = 1;		/* stack grew upward */
    else
      stack_dir = -1;		/* stack grew downward */
}

#endif	/* STACK_DIRECTION == 0 */

/*
	An "alloca header" is used to:
	(a) chain together all alloca()ed blocks;
	(b) keep track of stack depth.

	It is very important that sizeof(header) agree with malloc()
	alignment chunk size.  The following default should work okay.
*/

#ifndef	ALIGN_SIZE
#define	ALIGN_SIZE	sizeof(double)
#endif

typedef union hdr
{
  char	align[ALIGN_SIZE];	/* to force sizeof(header) */
  struct
    {
      union hdr *next;		/* for chaining headers */
      char *deep;		/* for stack depth measure */
    } h;
} header;

/*
	alloca( size ) returns a pointer to at least `size' bytes of
	storage which will be automatically reclaimed upon exit from
	the procedure that called alloca().  Originally, this space
	was supposed to be taken from the current stack frame of the
	caller, but that method cannot be made to work for some
	implementations of C, for example under Gould's UTX/32.
*/

static header *last_alloca_header = NULL; /* -> last alloca header */

pointer
alloca (size)			/* returns pointer to storage */
     unsigned	size;		/* # bytes to allocate */
{
  auto char	probe;		/* probes stack depth: */
  register char	*depth = &probe;

#if STACK_DIRECTION == 0
  if (STACK_DIR == 0)		/* unknown growth direction */
    find_stack_direction ();
#endif

				/* Reclaim garbage, defined as all alloca()ed storage that
				   was allocated from deeper in the stack than currently. */

  {
    register header	*hp;	/* traverses linked list */

    for (hp = last_alloca_header; hp != NULL;)
      if (STACK_DIR > 0 && hp->h.deep > depth
	  || STACK_DIR < 0 && hp->h.deep < depth)
	{
	  register header	*np = hp->h.next;

	  free ((pointer) hp);	/* collect garbage */

	  hp = np;		/* -> next header */
	}
      else
	break;			/* rest are not deeper */

    last_alloca_header = hp;	/* -> last valid storage */
  }

  if (size == 0)
    return NULL;		/* no allocation required */

  /* Allocate combined header + user data storage. */

  {
    register pointer	new = xmalloc (sizeof (header) + size);
    /* address of header */

    ((header *)new)->h.next = last_alloca_header;
    ((header *)new)->h.deep = depth;

    last_alloca_header = (header *)new;

    /* User storage begins just after header. */

    return (pointer)((char *)new + sizeof(header));
  }
}

