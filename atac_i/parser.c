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

static const char parser_c[] = "$Id: parser.c,v 3.7 2013/12/08 18:03:48 tom Exp $";
static const char bellcoreCopyRight[] =
"Copyright (c) 1993 Bell Communications Research, Inc. (Bellcore)";

/*
* @Log: parser.c,v @
* Revision 3.6  1997/12/08 22:22:48  tom
* use TNODE* instead of void*
*
* Revision 3.5  1997/05/12 00:11:47  tom
* add includes to get prototypes
*
* Revision 3.4  1994/04/04 13:36:30  saul
* FROM_KEYS
*
* Revision 3.4  94/04/04  13:36:30  saul
* Fix binary copyright.
* 
* Revision 3.3  94/04/04  10:13:32  jrh
* Add Release Copyright
* 
* Revision 3.2  93/08/04  15:46:49  ewk
* Added MVS and solaris support.  Squelched some ANSI warnings.
* 
* Revision 3.1  93/07/12  10:56:17  saul
* MVS MODULEID
* 
* Revision 3.0  92/11/06  07:44:52  saul
* propagate to version 3.0
* 
* Revision 2.7  92/10/30  09:48:31  saul
* include portable.h
* 
* Revision 2.6  92/06/11  13:44:17  saul
* changes for unique prefix
* 
* Revision 2.5  92/04/07  07:38:22  saul
* added unique prefix stuff
* 
* Revision 2.4  92/03/17  14:31:37  saul
* copyright
* 
* Revision 2.3  92/03/17  14:22:40  saul
* copyright
* 
* Revision 2.2  91/06/13  12:56:19  saul
* Remove gramar details.
* 
* Revision 2.1  91/06/13  12:39:13  saul
* Propagate to version 2.0
* 
* Revision 1.1  91/06/12  22:14:58  saul
* Initial revision
* 
*-----------------------------------------------end of log
*/
#include <stdlib.h>
#include <stdio.h>
#include "portable.h"
#include "error.h"
#include "tnode.h"

static void
usage(char *cmd)
{
    fprintf(stderr, "Usage: %s [-S] [-s] [-t] [-d] [-w] srcin\n", cmd);
    fprintf(stderr, "\t-S supress symbol resolution\n");
    fprintf(stderr, "\t-s dump symbols\n");
    fprintf(stderr, "\t-t dump parse tree\n");
    fprintf(stderr, "\t-d deparse\n");
    fprintf(stderr, "\t-w supress warning messages\n");
}

int
main(int argc,
     char *argv[])
{
    static char *prefix;
    TNODE *tree;
    int status;
    int i;
    char *p;
    int supress_sym = 0;
    int dump_tree = 0;
    int dump_sym_flag = 0;
    int do_deparse = 0;
    FILE *srcin = NULL;

    for (i = 1; i < argc; ++i) {
	p = argv[i];
	if (*p == '-' && *(p + 1)) {
	    while (*++p) {
		switch (*p) {
		case 'S':
		    supress_sym = 1;
		    break;
		case 's':
		    dump_sym_flag = 1;
		    break;
		case 't':
		    dump_tree = 1;
		    break;
		case 'd':
		    do_deparse = 1;
		    break;
		case 'w':
		    supress_warnings();
		    break;
		case '?':
		    usage(argv[0]);
		    exit(1);
		default:
		    fprintf(stderr,
			    "unknown flag %c\n",
			    *p);
		    exit(1);
		}
	    }
	} else {
	    if (srcin == NULL) {
		if (*p == '-')
		    srcin = stdin;
		else {
		    srcin = fopen(p, "r");
		    if (srcin == NULL) {
			fprintf(stderr,
				"can't open %s\n", p);
			exit(1);
		    }
		}
	    } else {
		fprintf(stderr, "extra argument %s\n", p);
		exit(1);
	    }
	}
    }

    if (srcin == NULL)
	srcin = stdin;

    status = parse(srcin, &tree, &prefix);

    if (status == 0) {
	if (!supress_sym)
	    do_sym(tree);
	if (dump_tree)
	    print_tree(tree, 1, 0, 0);
	if (dump_sym_flag)
	    dump_sym(tree, prefix);
	if (do_deparse)
	    deparse(tree, stdout, "HOOK", prefix);
    } else {
	fprintf(stderr, "parse errors\n");
	exit(1);
    }

    return 0;
}
