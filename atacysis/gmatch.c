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

#include "portable.h"
#include "atacysis.h"

static char const gmatch_c[] = 
	"$Header: /users/source/archives/atac.vcs/atacysis/RCS/gmatch.c,v 3.4 1995/12/27 20:54:20 tom Exp $";
/*
* $Log: gmatch.c,v $
* Revision 3.4  1995/12/27 20:54:20  tom
* adjust headers, prototyped for autoconfig
*
*Revision 3.3  94/05/02  10:11:36  saul
*fix comments
*
*Revision 3.2  94/04/04  10:25:20  jrh
*Add Release Copyright
*
*Revision 3.1  93/08/04  15:54:24  ewk
*Added MVS and solaris support.  Squelched some ANSI warnings.
*
* Revision 3.0  92/11/06  07:47:08  saul
*propagate to version 3.0
*
*Revision 2.2  92/10/30  09:54:18  saul
*include portable.h
*
*Revision 2.1  92/09/08  08:28:32  saul
*File dated Aug 1, 1985.
*
*-----------------------------------------------end of log
*/

/*
 * gmatch compares the string s with the shell pattern p and returns 1 if for match,
 * 0 otherwise.
 * The eighth bit is used to 'quote' a character
 */

#define STRIP	0177

int
gmatch(s, p)
register char *p;
char *s;
{
 register int 	scc,c;
 while ((scc = *s++) != '\0')
	{
	 if(scc)
		{
		 if((scc &= STRIP)==0)
			 scc=0200;
		}
	 switch(c = *p++)
		{
		 case '[':
			{
			 char ok = 0;
			 int lc = -1;
			 int notflag=0;
			 if(*p == '!' )
				{
				 notflag=1;
				 p++;
				}
			 while ((c = *p++) != '\0')
				{
				 if(c==']' && lc>=0)
					 return(ok?gmatch(s,p):0);
				 else if(c=='-' && lc>=0 && *p!=']')
					 /*character range */
					{
					 c = *p++;
					 if(notflag)
						{
						 if(lc>scc || scc>c)
							 ok++;
						 else
							 return(0);
						}
					 else
						 if(lc<scc && scc<=c)
							 ok++;
					}
				 else
					{
					 c &=STRIP;
				 	 if(notflag)
						{
						 if(scc!=c)
							 ok++;
						 else
							 return(0);
						}
					 else
						{
						 if(scc==c)
							 ok++;
						}
					 lc = c;
					}
				}
			 return(0);
			}
		 default:
			 if((c&STRIP) != scc)
				 return(0);
		 case '?':
			 break;
		 case '*':
		/* several asteriks are the same as one */
			 while(*p=='*' )
				 p++;
			 if(*p==0)
				 return(1);
			 --s;
			 c = (*p)&STRIP;
			 while(*s)
				{
				 if(c != ((*s)&STRIP) && *p!='?' && *p!='[')
					 s++;
				 else if(gmatch(s++,p))
					 return(1);
				}
			 return(0);

		 case 0:
			 return(scc==0);
		}
	}
 while(*p == '*')
	 p++;
 return(*p==0);
}
