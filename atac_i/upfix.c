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
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef MVS
#include <mvapts.h>
MODULEID(%M%,%J%/%D%/%T%)
#endif /* MVS */

static const char upfix_c[] = "$Id: upfix.c,v 3.9 2013/12/08 18:06:31 tom Exp $";
/*
* @Log: upfix.c,v @
* Revision 3.8  1997/12/09 00:00:52  tom
* int/size_t fixes
*
* Revision 3.7  1997/05/11 23:14:24  tom
* use size_t
*
* Revision 3.6  1997/05/10 22:03:12  tom
* split-out upfix.h
*
* Revision 3.5  1996/11/13 00:40:26  tom
* change ident to 'const' to quiet gcc
* add forward-ref prototypes
*
* Revision 3.4  94/04/04  10:15:29  jrh
* Add Release Copyright
*
* Revision 3.3  93/08/09  16:09:50  saul
* error in mvs code
*
* Revision 3.2  93/08/04  15:49:20  ewk
* Added MVS and solaris support.  Squelched some ANSI warnings.
*
* Revision 3.1  93/07/12  11:50:56  saul
* MVS MODULEID
*
* Revision 3.0  92/11/06  07:46:08  saul
* propagate to version 3.0
*
* Revision 2.3  92/10/30  09:49:27  saul
* include portable.h
*
* Revision 2.2  92/04/07  07:53:08  saul
* prefix should not begin with underscore
*
* Revision 2.1  92/04/07  07:33:52  saul
* Nov 23, 1989 version
*
*-----------------------------------------------end of log
*/
#if HAVE_STDLIB_H
#include <stdlib.h>
#endif
#include <string.h>
#include <stdio.h>
#include "portable.h"
#include "upfix.h"

#ifndef MVS
#define UPCASE_COUNT	('Z' - 'A' + 1)
#define LOWCASE_COUNT	('z' - 'a' + 1)
#else /* MVS */
#define UPCASE_COUNT	26
#define LOWCASE_COUNT	26
#endif /* MVS */
#define DIGIT_COUNT	('9' - '0' + 1)
#define USCORE_COUNT	1

PREFIX *
upfix_init(size_t maxlen,
	   char *s)
{
    PREFIX *p;

    p = (PREFIX *) malloc(sizeof *p);
    if (p == NULL)
	return NULL;

    p->charsleft = UPCASE_COUNT + LOWCASE_COUNT;
    p->upcase = (1 << UPCASE_COUNT) - 1;
    p->lowcase = (1 << LOWCASE_COUNT) - 1;
    p->uscore = 0;
    p->digit = 0;
    p->maxlen = maxlen;
    p->prefix = (char *) malloc(maxlen + 1);
    if (p->prefix == NULL) {
	free(p);
	return NULL;
    }
    if (s) {
	p->prefixlen = strlen(s);
	if (p->prefixlen > maxlen) {
	    free(p->prefix);
	    free(p);
	    return NULL;
	} else
	    strcpy(p->prefix, s);
	if (p->prefixlen > 0)
	    p->unique = 1;	/* unique so far */
	else
	    p->unique = 0;
    } else {
	p->prefixlen = 0;
	p->unique = 0;
    }

    return p;
}

void
upfix_free(PREFIX * p)
{
    free(p->prefix);
    free(p);
}

void
upfix_exclude(PREFIX * p,
	      char *name)
{
    size_t namelen;
    int c;
    int bit;

    namelen = strlen(name);
    if (namelen < p->prefixlen)
	return;

    if (p->prefixlen) {
	if (strncmp(name, p->prefix, p->prefixlen))
	    return;
	p->unique = 0;
	if (p->prefixlen == p->maxlen)
	    return;		/* upfix() will fail. */
    }

    if (namelen == p->prefixlen)
	return;

    c = name[p->prefixlen];
#ifndef MVS
    if (c >= 'A' && c <= 'Z') {
	bit = 1 << (c - 'A');
	if (p->upcase & bit) {
	    p->upcase &= ~bit;
	    p->charsleft--;
	}
    } else if (c >= 'a' && c <= 'z') {
	bit = 1 << (c - 'a');
	if (p->lowcase & bit) {
	    p->lowcase &= ~bit;
	    p->charsleft--;
	}
    }
#else /* MVS */
    if (c >= 'A' && c <= 'I') {
	bit = 1 << (c - 'A');
	if (p->upcase & bit) {
	    p->upcase &= ~bit;
	    p->charsleft--;
	}
    } else if (c >= 'J' && c <= 'R') {
	bit = 1 << ((c - 'J') + 9);
	if (p->upcase & bit) {
	    p->upcase &= ~bit;
	    p->charsleft--;
	}
    } else if (c >= 'S' && c <= 'Z') {
	bit = 1 << ((c - 'S') + 18);
	if (p->upcase & bit) {
	    p->upcase &= ~bit;
	    p->charsleft--;
	}
    } else if (c >= 'a' && c <= 'i') {
	bit = 1 << (c - 'a');
	if (p->lowcase & bit) {
	    p->lowcase &= ~bit;
	    p->charsleft--;
	}
    } else if (c >= 'j' && c <= 'r') {
	bit = 1 << ((c - 'j') + 9);
	if (p->lowcase & bit) {
	    p->lowcase &= ~bit;
	    p->charsleft--;
	}
    } else if (c >= 's' && c <= 'z') {
	bit = 1 << ((c - 's') + 18);
	if (p->lowcase & bit) {
	    p->lowcase &= ~bit;
	    p->charsleft--;
	}
    }
#endif /* MVS */
    else if (c >= '0' && c <= '9') {
	bit = 1 << (c - '0');
	if (p->digit & bit) {
	    p->digit &= ~bit;
	    p->charsleft--;
	}
    } else if (c == '_' && p->uscore) {
	p->uscore = 0;
	p->charsleft--;
    }

    if (p->charsleft > 1)
	return;

#ifndef MVS
    if (p->upcase) {
	c = 'A';
	while (p->upcase >>= 1)
	    ++c;
    } else if (p->lowcase) {
	c = 'a';
	while (p->lowcase >>= 1)
	    ++c;
    }
#else /* MVS */
    if (p->upcase) {
	c = 'A';
	while (p->upcase >>= 1) {
	    ++c;
	    if (c > 'I' && c < 'J')
		c = 'J';
	    else if (c > 'R' && c < 'S')
		c = 'S';
	}
    } else if (p->lowcase) {
	c = 'a';
	while (p->lowcase >>= 1) {
	    ++c;
	    if (c > 'i' && c < 'j')
		c = 'j';
	    else if (c > 'r' && c < 's')
		c = 's';
	}
    }
#endif /* MVS */
    else if (p->digit) {
	c = '0';
	while (p->digit >>= 1)
	    ++c;
    } else
	c = '_';

    p->prefix[p->prefixlen++] = c;
    p->unique = 1;
    p->charsleft = DIGIT_COUNT + UPCASE_COUNT + LOWCASE_COUNT + USCORE_COUNT;
    p->digit = (1 << DIGIT_COUNT) - 1;
    p->upcase = (1 << UPCASE_COUNT) - 1;
    p->lowcase = (1 << LOWCASE_COUNT) - 1;
    p->uscore = 1;
}

char *
upfix(PREFIX * p)
{
    int c;
    unsigned long tmp;

    if (!p->unique) {
	if (p->prefixlen == p->maxlen)
	    return NULL;

	if (p->upcase) {
	    tmp = p->upcase;
	    c = 'A';
#ifndef MVS
	    while (tmp >>= 1)
		++c;
#else /* MVS */
	    while (tmp >>= 1) {
		++c;
		if (c > 'I' && c < 'J')
		    c = 'J';
		else if (c > 'R' && c < 'S')
		    c = 'S';
	    }
#endif
	} else if (p->lowcase) {
	    tmp = p->lowcase;
	    c = 'a';
#ifndef MVS
	    while (tmp >>= 1)
		++c;
#else /* MVS */
	    while (tmp >>= 1) {
		++c;
		if (c > 'i' && c < 'j')
		    c = 'j';
		else if (c > 'r' && c < 's')
		    c = 's';
	    }
#endif /* MVS */
	} else if (p->digit) {
	    tmp = p->digit;
	    c = '0';
	    while (tmp >>= 1)
		++c;
	} else
	    c = '_';

	p->prefix[p->prefixlen] = c;
    }

    p->prefix[p->prefixlen + 1] = '\0';

    return p->prefix;
}
