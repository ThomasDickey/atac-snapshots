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

static char tnode_c[] = 
	"$Header: /users/source/archives/atac.vcs/atac_i/RCS/tnode.c,v 3.3 1994/04/04 10:14:56 jrh Exp $";
/*
*-----------------------------------------------$Log: tnode.c,v $
*-----------------------------------------------Revision 3.3  1994/04/04 10:14:56  jrh
*-----------------------------------------------FROM_KEYS
*-----------------------------------------------
*Revision 3.3  94/04/04  10:14:56  jrh
*Add Release Copyright
*
*Revision 3.2  93/08/04  15:48:36  ewk
*Added MVS and solaris support.  Squelched some ANSI warnings.
*
e.c
*Revision 3.1  93/07/12  11:47:19  saul
*MVS MODULEID
*
*Revision 3.0  92/11/06  07:45:44  saul
*propagate to version 3.0
*
*Revision 2.2  92/10/30  09:49:18  saul
*include portable.h
*
*Revision 2.1  92/09/30  10:38:35  saul
**** empty log message ***
*
*-----------------------------------------------end of log
*/
#include <stdio.h>
#include "portable.h"
#include "srcpos.h"
#include "tnode.h"

/* forward declarations */
void check_malloc();
TNODE *tnext();
TNODE *child4();
TNODE *child3();
TNODE *child2();
TNODE *child1();
TNODE *child0();

TNODE *
child0(n)
TNODE *n;
{
    TNODE *macro_n;

    return (macro_n = (n)->down,
	    macro_n ? macro_n->over : (TNODE *)NULL);
}

TNODE *
child1(n)
TNODE *n;
{
    TNODE *macro_n;

    return (macro_n = (n)->down,
	    macro_n ? macro_n->over->over : (TNODE *)NULL);
}

TNODE *
child2(n)
TNODE *n;
{
    TNODE *macro_n;

    return (macro_n = (n)->down,
	macro_n ? macro_n->over->over->over : (TNODE *)NULL);
}

TNODE *
child3(n)
TNODE *n;
{
    TNODE *macro_n;

    return (macro_n = (n)->down,
	macro_n ? macro_n->over->over->over->over : (TNODE *)NULL);
}

TNODE *
child4(n)
TNODE *n;
{
    TNODE *macro_n;

    return (macro_n = (n)->down,
	macro_n ? macro_n->over->over->over->over->over : (TNODE *)NULL);
}

TNODE *
tnext(n)
TNODE *n;
{
    TNODE *macro_n;

    return (macro_n = (n),
	macro_n->up->down == macro_n ? (TNODE *)NULL : macro_n->over);
}

void
check_malloc(p)
char *p;
{
    if (p == NULL) internal_error(NULL, "Out of memory");
}
