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

static const char deparse_c[] = "$Id: deparse.c,v 3.13 2013/12/09 01:41:05 tom Exp $";
/*
* @Log: deparse.c,v @
* Revision 3.11  2005/08/14 13:44:42  tom
* gcc warnings
*
* Revision 3.10  1998/09/19 15:01:02  tom
* add a cast for 2nd param of aTaC function
*
* Revision 3.9  1997/12/08 23:23:43  tom
* int/size_t fix.
* correct char* cast of n->sym.hook.type
*
* Revision 3.8  1997/05/12 00:22:26  tom
* corrent sprintf-format
*
* Revision 3.7  1997/05/10 23:21:05  tom
* absorb srcpos.h into error.h
*
* Revision 3.6  1996/11/13 00:42:34  tom
* change ident to 'const' to quiet gcc
* add forward-ref prototypes
*
* Revision 3.5  94/04/04  10:12:12  jrh
* Add Release Copyright
* 
* Revision 3.4  93/08/04  15:44:27  ewk
* Added MVS and solaris support.  Squelched some ANSI warnings.
* 
* Revision 3.3  93/07/12  10:07:32  saul
* MVS MODULEID
* 
* Revision 3.2  93/04/29  13:35:45  saul
* Malloc error in prevous fix.
* 
* Revision 3.1  93/04/29  07:38:52  saul
* Handle very long function names.
* 
* Revision 3.0  92/11/06  07:45:27  saul
* propagate to version 3.0
* 
* Revision 2.6  92/10/30  09:47:40  saul
* include portable.h
* 
* Revision 2.5  92/04/27  09:14:10  saul
* defensive code added for bad genus/species
* 
* Revision 2.4  92/03/17  14:22:19  saul
* copyright
* 
* Revision 2.3  91/06/13  13:23:56  saul
* ignore last entry of deparse.h (Header)
* 
* Revision 2.2  91/06/13  12:47:55  saul
* Change \% to \\%
* 
* Revision 2.1  91/06/13  12:38:58  saul
* Propagate to version 2.0
* 
* Revision 1.1  91/06/12  20:25:36  saul
* Aug 1990 baseline
* 
*-----------------------------------------------end of log
*/

#include "portable.h"

#include "error.h"
#include "tree.h"
#include "tnode.h"
#include "hook.h"

/* forward declarations */
static void dparse(TNODE * n, FILE *f, int tablevel, char *hookname, char *prefix);

#define CHECK_MALLOC(p) ((p)?1:internal_error(NULL, "Out of memory\n"))

#define TABSIZE 8
#define LINELEN 72
#define STRINGBUF_SIZE 30

typedef struct script {
    int genus;
    int species;
    const char *script;
} SCRIPT;

static SCRIPT all_scripts[] =
{
#include "deparse.h"
};
			/* -1 is for RCS ID at end */
#define N_ALL_SCRIPTS 	((sizeof all_scripts / sizeof *all_scripts) - 1)
/* *INDENT-OFF* */
static SCRIPT hook_scripts[] = {
	{GEN_HOOK,	HOOK_STMT_R,	"@N\n@H(@P,@B);"},
	{GEN_HOOK,	HOOK_STMT_L,	"@H(@P,@B);\n@N"},
	{GEN_HOOK,	HOOK_STMT_R_B,	"{@+\n@N\n@H(@P,@B);@-\n}"},
	{GEN_HOOK,	HOOK_STMT_L_B,	"{@+\n@H(@P,@B);\n@N@-\n}"},
	{GEN_HOOK,	HOOK_START,	"{@+\nint @P=@H(0,(long)&@PT@C);\n@N@-\n}"},
	{GEN_HOOK,	HOOK_EXPR_R,	"(@P@I= @N,@H(@P,@B),@P@I)"},
	{GEN_HOOK,	HOOK_EXPR_L,	"(@H(@P,@B),@N)"},
	{GEN_HOOK,	HOOK_EXPR_CAST,	"(@F)(@H(@P,@B),@N)"},
	{GEN_HOOK,	HOOK_TEMP,	"@T;\n@N"},
};
/* *INDENT-ON* */

#define N_HOOK_SCRIPTS 	(sizeof hook_scripts / sizeof *hook_scripts)

static int scriptIndex[GENUS_COUNT];

static void
init_scriptIndex(void)
{
    int g;
    int s = 0;
    int i;

    g = -1;
    for (i = 0; i < (int) N_ALL_SCRIPTS; ++i) {
	if (all_scripts[i].genus != g) {
	    s = 0;
	    ++g;
	    if (g >= GENUS_COUNT) {
		fprintf(stderr, "Too many genuses; entry: %d", i);
		internal_error(NULL, "deparse init failed");
	    }
	    scriptIndex[g] = i;
	}
	if (all_scripts[i].genus != g || all_scripts[i].species != s) {
	    fprintf(stderr, "index init error at entry: %d, ", i);
	    fprintf(stderr, "expected {%d, %d, ...} ", g, s);
	    fprintf(stderr, "got {%d, %d, \"%s\"}\n",
		    all_scripts[i].genus, all_scripts[i].species,
		    all_scripts[i].script);
	    internal_error(NULL, "deparse init failed");
	    return;
	}
	++s;
    }

    while (++g < GENUS_COUNT)
	scriptIndex[g] = 0;
}

static void
dparse(TNODE * n,
       FILE *f,
       int tablevel,
       char *hookname,
       char *prefix)
{
    char stringBuf[STRINGBUF_SIZE];
    char *string;
    SCRIPT *script = NULL;
    const char *p;
    char *q;
    TNODE *next;
    int genus;
    int species;
    static int temptab = 0;
    static int column = 0;

    if (n == NULL) {
	return;			/* e.g. Empty input. */
    }

    if (column >= LINELEN) {
	fputc('\n', f);
	temptab = TABSIZE;
	column = 0;
    }

    genus = n->genus;
    species = n->species;

    string = stringBuf;

    switch (genus) {
    case GEN_FCON:
    case GEN_ICON:
    case GEN_STRING:
    case GEN_NAME:
    case GEN_TNAME:
    case GEN_FNAME:
	if (column == 0) {
	    while (column < (tablevel + temptab)) {
		fputc('\t', f);
		column += TABSIZE;
	    }
	    temptab = 0;
	}
	p = n->text;
	if (p == NULL)
	    p = "<?unknown?>";
	fputs(p, f);
	column += (int) strlen(p);
	return;
    }

    if (genus >= 0 && genus < GENUS_COUNT) {
	script = all_scripts + species + scriptIndex[genus];
	if (script < all_scripts ||
	    script >= all_scripts + N_ALL_SCRIPTS) {
	    internal_error(n->srcpos, "can't deparse %d.%d",
			   genus, species);
	}
    } else {
	if (species >= 0 && species < (int) N_HOOK_SCRIPTS)
	    script = hook_scripts + species;
	else
	    internal_error(n->srcpos, "can't deparse hook %d/%d",
			   genus, species);
    }
    if (script->genus != genus || script->species != species)
	internal_error(n->srcpos, "can't deparse genus.species %d/%d",
		       genus, species);

    next = CHILD0(n);

    for (p = script->script; *p; ++p) {
	switch (*p) {
	case '@':
	    string[0] = '\0';
	    ++p;
	    switch (*p) {
	    case '@':
		p = script->script - 1;
		break;
	    case 'L':
		dparse(next, f, tablevel, hookname, prefix);
		if (next)
		    next = TNEXT(next);
		if (next == NULL)
		    return;
		break;
	    case 'N':
	    case 'S':
		dparse(next, f, tablevel, hookname, prefix);
		if (next)
		    next = TNEXT(next);
		break;
	    case '+':
		tablevel += TABSIZE;
		break;
	    case '-':
		if (tablevel)
		    tablevel -= TABSIZE;
		break;
	    case 'I':
		sprintf(string, "%d", n->sym.hook.tempno);
		break;
	    case 'A':
		sprintf(string, "%p", (void *) n->sym.sym);
		break;
	    case 'C':
		q = (char *) n->sym.hook.type;	/* kludge */
		if (strlen(q) >= sizeof stringBuf) {
		    string = (char *) malloc(strlen(q) + 1);
		    CHECK_MALLOC(string);
		}
		sprintf(string, "%s", q);
		break;
	    case 'T':
/* ? counting problem */
		sprintf(string, "%s%d",
			prefix, n->sym.hook.tempno);
		print_type(f, n->sym.hook.type, string, prefix);
		string[0] = '\0';
		break;
	    case 'F':
/* ? counting problem */
		print_type(f, n->sym.hook.type, "", prefix);
		break;
	    case 'B':
		sprintf(string, "%d", n->sym.hook.blkno);
		break;
	    case 'H':
		strcpy(string, hookname);
		break;
	    case 'P':
		strcpy(string, prefix);
		break;
	    case 'V':
	    default:
		/* Shouldn't get here. */
		sprintf(string, "@%c", *p);
		break;
	    }
	    if (string[0]) {
		if (column == 0) {
		    while (column < (tablevel + temptab)) {
			fputc('\t', f);
			column += TABSIZE;
		    }
		    temptab = 0;
		}
		fputs(string, f);
		column += (int) strlen(string);
		if (string != stringBuf) {
		    free(string);
		    string = stringBuf;
		}
	    }
	    break;
	case '\n':
	    fputc('\n', f);
	    column = 0;
	    break;
	default:
	    if (column == 0) {
		while (column < (tablevel + temptab)) {
		    fputc('\t', f);
		    column += TABSIZE;
		}
		temptab = 0;
	    }
	    if (isprint(*p) || isspace(*p)) {
		fputc(*p, f);
		++column;
	    } else {
		fprintf(f, "\\%3.3o", *p);
		column += 4;
	    }
	}
    }
}

void
deparse(TNODE * n,
	FILE *f,
	char *hookname,
	char *prefix)
{
    static int init = 0;

    if (init == 0) {
	init_scriptIndex();
	init = 1;
    }

    dparse(n, f, 0, hookname, prefix);
}
