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
#ifdef MVS
#include <mvapts.h>
MODULEID(%M%,%J%/%D%/%T%)
#endif /* MVS */

static char pat_match_c[] = 
	"$Header: /users/source/archives/atac.vcs/atacysis/RCS/pat_match.c,v 3.2 1994/04/04 10:25:52 jrh Exp $";
/*
*-----------------------------------------------$Log: pat_match.c,v $
*-----------------------------------------------Revision 3.2  1994/04/04 10:25:52  jrh
*-----------------------------------------------FROM_KEYS
*-----------------------------------------------
*Revision 3.2  94/04/04  10:25:52  jrh
*Add Release Copyright
*
*Revision 3.1  93/08/04  15:57:19  ewk
*Added MVS and solaris support.  Squelched some ANSI warnings.
*
* Revision 3.0  92/11/06  07:47:26  saul
*propagate to version 3.0
*
*Revision 2.2  92/10/30  09:55:18  saul
*include portable.h
*
*Revision 2.1  92/09/08  08:43:46  saul
*New pattern matching features.
*
*-----------------------------------------------end of log
*/
#include <stdio.h>
#include "portable.h"

/* forward declarations */
int patMatch();

/*
* patMatch:  Return 1 if name matches any of the comma separated patterns
*	in pat.  Patterns are shell filename matching patterns (composed
*	of ?, [...], *, and literal characters).  If deselect is non-zero,
*	reverse the return value (0 becomes 1, 1 becomes 0).  A NULL pattern
*	list matches everything.
*/
int
patMatch(pat, name, deselect)
char *pat;
char *name;
int deselect;
{
    char	*pStart;
    char	*pEnd;
    int		match;
    int		flip;

    if (deselect) {
	flip = 1;	/* must be 1, not other non-zero */
    } else {
	flip = 0;
    }

    if (pat == NULL) {
	return 1 ^ flip;
    }

    pStart = pat;
    for (pEnd = pStart; *pEnd; ++pEnd) {
	if (*pEnd == ',') {
	    *pEnd = '\0';
	    match = gmatch(name, pStart);
	    *pEnd = ',';
	    if (match) {
		return 1 ^ flip;
	    }
	    pStart = pEnd + 1;
	}
    }

    match = gmatch(name, pStart);	/* returns 1 or 0 */

    return match ^ flip;
}
