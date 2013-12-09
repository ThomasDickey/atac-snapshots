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
#pragma runopts(execops,isasize(50k))	/* ==> for MVS delete leading blank! */
#include <mvapts.h>
MODULEID(%M%,%J%/%D%/%T%)
#endif /* MVS */

static const char atac_i_c[] = "$Id: atac_i.c,v 3.12 2013/12/08 17:36:25 tom Exp $";
static const char bellcoreCopyRight[] =
"Copyright (c) 1993 Bell Communications Research, Inc. (Bellcore)";

/*
* @Log: atac_i.c,v @
* Revision 3.11  2005/08/13 15:13:22  tom
* portable.h now always includes stdio.h
*
* Revision 3.10  1997/12/09 00:48:59  tom
* use TNODE* instead of void*
*
* Revision 3.9  1997/05/11 23:59:50  tom
* add includes to get prototypes
*
* Revision 3.8  1996/11/13 00:41:53  tom
* change ident to 'const' to quiet gcc
* add forward-ref prototypes
*
* Revision 3.7  94/04/04  13:42:39  saul
* Fix binary copyright.
* 
* Revision 3.6  94/04/04  10:11:53  jrh
* Add Release Copyright
* 
* Revision 3.5  93/12/15  12:55:09  saul
* McCabe cyclomatic number calculation
* 
* Revision 3.4  93/08/11  13:05:57  saul
* #pragma not permitted by some cpp's
* 
* Revision 3.3  93/08/09  12:24:05  saul
* -e flag added for testing
* 
* Revision 3.2  1993/08/04  15:44:04  ewk
* Added MVS and solaris support.  Squelched some ANSI warnings.
*
* Revision 3.1  93/07/12  10:06:54  saul
* MVS MODULEID
* 
* Revision 3.0  92/11/06  07:45:50  saul
* propagate to version 3.0
* 
* Revision 2.8  92/10/30  09:47:33  saul
* include portable.h
* 
* Revision 2.7  92/07/10  11:53:02  saul
* New -F suppress feasability analysis option
* 
* Revision 2.6  92/06/11  13:45:22  saul
* changes for unique prefix
* 
* Revision 2.5  92/04/07  07:37:05  saul
* added unique prefix stuff
* 
* Revision 2.4  92/03/17  14:31:34  saul
* copyright
* 
* Revision 2.3  92/03/17  14:22:14  saul
* copyright
* 
* Revision 2.2  91/06/19  13:21:32  saul
* Moved tree dumping stuff to tree.h
* 
* Revision 2.1  91/06/13  12:38:55  saul
* Propagate to version 2.0
* 
* Revision 1.1  91/06/12  20:25:35  saul
* Aug 1990 baseline
* 
*-----------------------------------------------end of log
*/
#include "portable.h"
#include "error.h"
#include "flowgraph.h"
#include "atac_i.h"

/* forward declarations */
static void usage(char *);
static void runErrorTests(void *tree);

int dump_tables = 0;
int all_paths = 0;
int list_all_paths = 0;
int count_alldu = 0;
int feasableFlag = 1;
int cyclomaticFlag = 0;

static void
usage(char *cmd)
{
    fprintf(stderr, "Usage: %s [-w] srcin srcout fgout\n", cmd);
    fprintf(stderr, "\t-A list all paths\n");
    fprintf(stderr, "\t-D supress deparse\n");
    fprintf(stderr, "\t-F supress feasability analysis\n");
    fprintf(stderr, "\t-I supress instrumentation\n");
    fprintf(stderr, "\t-S supress symbol resolution\n");
    fprintf(stderr, "\t-T supress source table output\n");
    fprintf(stderr, "\t-a count all paths\n");
    fprintf(stderr, "\t-d count all du-paths\n");
    fprintf(stderr, "\t-e run some error coverage tests\n");
    fprintf(stderr, "\t-f static-data-file\n");
    fprintf(stderr, "\t-g dump flow graph tables\n");
    fprintf(stderr, "\t-f static-data-file\n");
    fprintf(stderr, "\t-m compute cyclomatic number\n");
    fprintf(stderr, "\t-s dump symbols\n");
    fprintf(stderr, "\t-t dump parse tree\n");
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
    int supress_inst = 0;
    int supress_deparse = 0;
    int dump_tree = 0;
    int dump_sym_flag = 0;
    int errorTestFlag = 0;
    FILE *srcin = NULL;
    FILE *srcout = NULL;
    FILE *fgout = NULL;
    FILE *tablesout = NULL;	/* usually same as srcout */

    for (i = 1; i < argc; ++i) {
	p = argv[i];
	if (*p == '-' && *(p + 1)) {
	    while (*++p) {
		switch (*p) {
		case 'A':
		    list_all_paths = 1;
		    break;
		case 'D':
		    supress_deparse = 1;
		    break;
		case 'F':
		    feasableFlag = 0;
		    break;
		case 'I':
		    supress_inst = 1;
		    break;
		case 'S':
		    supress_sym = 1;
		    break;
		case 'T':
		    tablesout =
			fopen("/dev/null", "w");
		    break;
		case 'a':
		    all_paths = 1;
		    break;
		case 'd':
		    count_alldu = 1;
		    supress_deparse = 1;
		    srcout = fopen("/dev/null", "r");
		    if (srcout == NULL) {
			fprintf(stderr,
				"can't open %s\n",
				"/dev/null");
			exit(1);
		    }
		    fgout = srcout;
		    break;
		case 'e':
		    errorTestFlag = 1;
		    break;
		case 'f':
		    /* backward compatibility */
		    if (*(p + 1) == '\0')
			p = argv[++i];
		    else
			++p;
		    fgout = fopen(p, "w");
		    if (fgout == NULL) {
			fprintf(stderr,
				"can't open %s\n", p);
			exit(1);
		    }
		    p = " ";
		    break;
		case 'g':
		    dump_tables = 1;
		    break;
		case 'm':
		    cyclomaticFlag = 1;
		    break;
		case 's':
		    dump_sym_flag = 1;
		    break;
		case 't':
		    dump_tree = 1;
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
	    } else if (srcout == NULL) {
		if (*p == '-')
		    srcout = stdout;
		else {
		    srcout = fopen(p, "w");
		    if (srcout == NULL) {
			fprintf(stderr,
				"can't open %s\n", p);
			exit(1);
		    }
		}
	    } else if (fgout == NULL) {
		if (*p == '-')
		    fgout = stdout;
		else {
		    fgout = fopen(p, "w");
		    if (fgout == NULL) {
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
    if (srcout == NULL)
	srcout = stdout;
    if (fgout == NULL)
	fgout = stdout;
    if (tablesout == NULL)
	tablesout = srcout;

    status = parse(srcin, &tree, &prefix);

    if (status == 0) {
	if (!supress_sym)
	    do_sym(tree);
	if (!supress_inst)
	    flowgraph(tree, tablesout, fgout, prefix);
	if (!supress_deparse)
	    deparse(tree, srcout, "aTaC", prefix);

	if (dump_tree)
	    print_tree(tree, 1, 0, 0);
	if (dump_sym_flag)
	    dump_sym(tree, prefix);
	if (errorTestFlag)
	    runErrorTests(tree);
    } else {
	fprintf(stderr, "parse errors\n");
	exit(1);
    }

    return 0;
}

static void
runErrorTests(void *tree)
{
    testConst();		/* run some tests in const.c */
}
