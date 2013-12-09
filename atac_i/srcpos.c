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

static const char srcpos_c[] = "$Id: srcpos.c,v 3.10 2013/12/08 18:40:17 tom Exp $";
/*
* @Log: srcpos.c,v @
* Revision 3.9  2005/08/14 13:43:57  tom
* gcc warnings
*
* Revision 3.8  1998/09/19 15:31:56  tom
* spell out line and col words for clarity in the source-position
*
* Revision 3.7  1997/05/12 00:34:05  tom
* remove redundant prototypes
*
* Revision 3.6  1997/05/10 23:19:59  tom
* absorb srcpos.h into error.h
*
* Revision 3.5  1996/11/12 23:02:07  tom
* change ident to 'const' to quiet gcc
* add forward-ref prototypes
*
* Revision 3.4  1995/12/27 23:32:19  tom
* don't use NULL for int value!
*
* Revision 3.3  94/04/04  10:14:15  jrh
* Add Release Copyright
* 
* Revision 3.2  93/08/04  15:47:49  ewk
* Added MVS and solaris support.  Squelched some ANSI warnings.
* 
* Revision 3.1  93/07/12  11:33:03  saul
* MVS MODULEID
* MVS node_srcpos_i ==> node_isrcpos for uniqueness
* 
* Revision 3.0  92/11/06  07:44:56  saul
* propagate to version 3.0
* 
* Revision 2.4  92/10/30  09:48:46  saul
* include portable.h
* 
* Revision 2.3  92/04/07  09:00:24  saul
* remove ATACYSIS #ifdefs
* 
* Revision 2.2  92/03/17  14:22:53  saul
* copyright
* 
* Revision 2.1  91/06/13  12:39:19  saul
* Propagate to version 2.0
* 
 * Revision 1.1  91/06/12  20:25:50  saul
 * Aug 1990 baseline
 * 
*-----------------------------------------------end of log
*/
#include "portable.h"
#include "error.h"
#include "tnode.h"

#define CHECK_MALLOC(p) ((p)?1:internal_error(NULL, "Out of memory\n"))

#define FNAME_BUF_SIZE	30

static char **filenames = NULL;
static int *stamps = NULL;
static int n_filenames = 0;
static int fname_buf_size = 0;

void
print_srcpos(SRCPOS * srcpos,
	     FILE *f)
{
    char *fname;

    if (srcpos == NULL) {
	fputs("?", f);
	return;
    }

    if (srcpos->file < 0) {
	fname = "\"\"";
    } else if (srcpos->file >= n_filenames) {
	fname = "\"?\"";
    } else {
	fname = filenames[srcpos->file];
    }

    fprintf(f, "%s, line %d, col %d", fname, srcpos->line, srcpos->col);
}

void
node_srcpos(node, left, f)
     TNODE *node;
     int left;
     FILE *f;
{
    if (node == NULL) {
	fprintf(f, "?");
	return;
    }

    if (left)
	print_srcpos(&node->srcpos[LEFT_SRCPOS], f);
    else
	print_srcpos(&node->srcpos[RIGHT_SRCPOS], f);
}

/*
* store_filename:  Keep a table of unique filenames.  Return index into table.
*	NOTE: filename is not copied.
*/
int
store_filename(char *s)
{
    int i;
    char *p;

    /*
     * If already in table return location.
     */
    for (i = 0; i < n_filenames; ++i)
	if (strcmp(s, filenames[i]) == 0)
	    return i;

    /*
     * Inlarge table if necessary.
     */
    if (fname_buf_size <= n_filenames) {
	fname_buf_size += FNAME_BUF_SIZE;
	if (filenames) {
	    filenames = (char **) realloc(filenames,
					  sizeof *filenames * fname_buf_size);
	    stamps = (int *) realloc(stamps,
				     sizeof *stamps * fname_buf_size);
	} else {
	    filenames = (char **)
		malloc(sizeof *filenames * fname_buf_size);
	    stamps = (int *)
		malloc(sizeof *stamps * fname_buf_size);
	}
	CHECK_MALLOC(filenames);
	CHECK_MALLOC(stamps);
    }

    filenames[n_filenames] = s;
    if (*s == '"' && *(p = s + strlen(s) - 1) == '"') {
	/*
	 * Kludge to temporarily get rid of enclosing quotes.
	 */
	*p = '\0';
	stamps[n_filenames] = filestamp(s + 1);
	*p = '"';
    } else
	stamps[n_filenames] = filestamp(s);

    return n_filenames++;
}

char *
srcfname(int findex)
{
    if (findex < 0 || findex >= n_filenames)
	return NULL;
    return filenames[findex];
}

int
srcfstamp(int findex)
{
    if (findex < 0 || findex >= n_filenames)
	return 0;
    return stamps[findex];
}

void
node_isrcpos(TNODE * node,
	     int left,
	     FILE *f)
{
    SRCPOS *srcpos;

    if (node == NULL) {
	fprintf(f, " 0 0 0");
	return;
    }

    if (left)
	srcpos = &node->srcpos[LEFT_SRCPOS];
    else
	srcpos = &node->srcpos[RIGHT_SRCPOS];

    fprintf(f, " %d %d %d", srcpos->file, srcpos->line, srcpos->col);
}
