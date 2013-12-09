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
#pragma csect (CODE, "scan$")
#include <mvapts.h>
MODULEID(%M%,%J%/%D%/%T%)
#endif /* MVS */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "portable.h"
#include "error.h"
#include "list.h"
#include "scan.h"
#include "strtab.h"
#include "tnode.h"		/* Pgram.h needs it */
#include "Pgram.h"

#define DEBUG 0

static char const scan_c[] = "$Id: scan.c,v 3.19 2013/12/09 00:17:53 tom Exp $";
/*
* @Log: scan.c,v @
* Revision 3.17  2008/12/17 01:07:56  tom
* convert to ANSI, indent'd
*
* Revision 3.16  1997/12/12 00:09:42  tom
* make yylex() return a negative value on end-of-file, rather than bogus
* ENDFILE symbol.
*
* Revision 3.14  1997/11/01 19:12:24  tom
* add __inline, since gcc uses it...
*
* Revision 3.13  1997/05/11 22:07:02  tom
* use ID_TYPE to fix mismatches
*
* Revision 3.12  1997/05/11 00:22:31  tom
* absorb srcpos.h into error.h
* correct type of strtab variable.
*
* Revision 3.11  1996/11/12 23:50:31  tom
* add forward-ref prototypes
* remove bizarre 'NULL' statements (a plain ';' works!)
*
* Revision 3.10  1995/12/29 23:29:35  tom
* adjust headers, prototyped for autoconfig
* add keywords to work with gcc 2.7.0 (including logic to skip over ASM)
*
* Revision 3.9  94/04/04  10:13:59  jrh
* Add Release Copyright
* 
* Revision 3.8  94/03/21  08:13:30  saul
* MVS support __offsetof as builtin (not handled by cpp)
* 
* Revision 3.7  1994/02/14  09:23:31  saul
* Deleted "unexpected @" message (see bug32)
* added explicit NULL body to empty while's (for readability)
*
* Revision 3.6  93/11/19  12:14:53  saul
* MVS support for _Packed
* 
* Revision 3.5  93/08/20  12:44:19  ewk
* Add recognition of EBCDIC broken bar (0x6a) for MVS.
* 
* Revision 3.4  93/08/09  16:08:31  saul
* macro expands to #ident bug
* 
* Revision 3.3  93/08/04  15:47:38  ewk
* Added MVS and solaris support.  Squelched some ANSI warnings.
* 
* Revision 3.2  93/07/12  14:07:07  saul
* MVS MODULEID
* MVS \v portability
* 
* Revision 3.1  93/03/03  13:21:35  saul
* fix empty cpp expansion bug (test case ae09)
* 
* Revision 3.0  92/11/06  07:44:58  saul
* propagate to version 3.0
* 
* Revision 2.18  92/11/06  07:11:15  saul
* remove unused variable
* 
* Revision 2.17  92/11/02  15:46:03  saul
* get rid of BITSPERBYTE usage.  Use CHAR() and MAXCHAR from portable.h
* 
* Revision 2.16  92/11/02  11:36:20  saul
* remove unused variables
* 
* Revision 2.15  92/10/30  09:48:41  saul
* include portable.h
* 
* Revision 2.14  92/10/28  08:54:19  saul
* enum's removed for portability
* 
* Revision 2.13  92/10/21  16:09:33  saul
* Line number problem with escaped newline in preexpansion macro text
* 
* Revision 2.12  92/10/09  08:23:10  saul
* Scan ELLIPSIS (...).
* 
* Revision 2.11  92/09/22  15:20:01  saul
* Optimize order of keyword search.
* Missing left source position numbers.
* 
* Revision 2.10  92/09/16  08:28:50  saul
* Error scanning a.b.
* 
* Revision 2.9  92/09/16  07:33:01  saul
* Rewritten.  Faster.  Scoped typedef facilities.
* 
* Revision 2.8  92/06/11  13:50:19  saul
* fix for wide string/character token text (L' or L" was lost)
* 
* Revision 2.7  92/04/07  07:37:03  saul
* added unique prefix stuff
* 
* Revision 2.6  92/03/17  13:18:07  saul
* add copyright
* 
* Revision 2.5  92/03/17  13:03:57  saul
* clean up white space and @< scaning
* 
* Revision 2.4  91/11/15  13:51:42  saul
* Allow vertical tab as white space for ansi
* 
* Revision 2.3  91/11/15  12:14:49  saul
* increment src line number after unrecognized #directive
* as recommended by Bob Kayel.
* 
* Revision 2.2  91/06/13  12:53:59  saul
* recognize ansi keywords (static, const)
* 
* Revision 2.1  91/06/13  12:39:16  saul
* Propagate to version 2.0
* 
* Revision 1.1  91/06/12  20:25:48  saul
* Aug 1990 baseline
* 
*-----------------------------------------------end of log
*/

/* forward declarations */
int scan_popScope(void);
int yylex(void);

static int expandedMacro(void);
static int scanAssignOp(int c);
static int scanNumber(FILE *srcin, size_t offset, int nextC);
static int scanSpaces(int c);
static int scanWhiteSpace(int c);
static void initCharTable(void);
static void poundDirective(void);
static void scanComment(void);
static void scanQuotedString(FILE *srcin, size_t offset, int quote);
void scan_end(char **uprefix);
void scan_init(FILE *srcfile);
void scan_pushScope(void);
void scan_setType(char *name);
void yyerror(char *string);

#define CHECK_MALLOC(p) ((p)?1:internal_error(NULL, "Out of memory\n"))

#define BUF_MALLOC_SIZE	40
#define MAX_DIRECTIVE_SIZE  20

typedef struct {
    ID_TYPE *idType;
    int oldType;
} typeFix_t;

#define BADCHAR		0
#define LETTER		1
#define DIGIT		2
#define WHITESPACE	3
#define STARTWS		4
#define SINGLEQUOTE	5
#define DOUBLEQUOTE	6
#define ENDOFFILE	7

/*
* Keyword table.  Order is deliberate (not necessarily optimal).  Current
* strtab algorithm uses a binary search tree.  Keywords should not be
* inserted in sorted order because the search tree will degenerate to a
* linked list.
*/
/* *INDENT-OFF* */
static struct {
	const char	*name;
	int	type;
} keyword[] = {
	{ "long", LONG },
	{ "for", FOR },
	{ "continue", CONTINUE },
	{ "break", BREAK },
	{ "auto", AUTO },
	{ "char", CHAR_KW },
	{ "case", CASE },
	{ "const", CONST },
	{ "else", ELSE },
	{ "default", DEFAULT },
	{ "do", DO },
	{ "double", DOUBLE },
	{ "extern", EXTERN },
	{ "enum", ENUM },
	{ "float", FLOAT },
	{ "goto", GOTO },
	{ "if", IF },
	{ "int", INT },
	{ "short", SHORT },
	{ "static", STATIC },
	{ "register", REGISTER },
	{ "return", RETURN },
	{ "typedef", TYPEDEF },
	{ "sizeof", SIZEOF },
	{ "signed", SIGNED },
	{ "struct", STRUCT },
	{ "switch", SWITCH },
	{ "void", VOID },
	{ "union", UNION },
	{ "unsigned", UNSIGNED },
	{ "volatile", VOLATILE },
	{ "while", WHILE },
#ifdef MVS
	{ "_Packed", TOK_PACKED },		/* MVS */
	{ "__offsetof", OFFSET },
#endif /* MVS */
	{ "__volatile__", VOLATILE },		/* GCC */
	{ "__signed__", SIGNED },		/* GCC */
	{ "__inline__", INLINE },		/* GCC: inline */
	{ "__inline", INLINE },			/* GCC: inline */
	{ "__asm__", ASM },			/* GCC: asm */
	{ "__attribute__", ATTRIBUTE },		/* GCC */
};
/* *INDENT-ON* */

static int charTable[MAXCHAR + 1];	/* MAXCHAR is in portable.h */

static FILE *srcin = NULL;	/* input file; */
static char *buf = NULL;
static size_t bufSize = 0;
static struct strtab *strtab = NULL;
static SRCPOS src =
{-1, 0, 0};
static SRCPOS m_begin;
static SRCPOS m_end;
static int macro_nesting = 0;
static LIST *scopeList;
static LIST *typeFixList;

/*
* initCharTable:  One time initialization of character table.
*	Classifies each character.
*/
static void
initCharTable(void)
{
    int i;

    /*
     * Default: BADCHAR.
     */
    for (i = 0; i <= MAXCHAR; ++i)
	charTable[i] = BADCHAR;

    /*
     * EOF or '\377'
     */
    charTable[CHAR(EOF)] = ENDOFFILE;

    /*
     * Whitespace.
     */
    charTable[' '] = WHITESPACE;
    charTable['\t'] = WHITESPACE;
    charTable['\f'] = WHITESPACE;
    charTable['\b'] = WHITESPACE;
    charTable['\r'] = WHITESPACE;
#ifdef __STDC__
    charTable['\v'] = WHITESPACE;	/* ANSI allows vertical tab as white space */
#else /* not __STDC__ */
    charTable['\013'] = WHITESPACE;	/* ANSI allows vertical tab as white space */
#endif /* not __STDC__ */

    /*
     * Potential whitespace.  (Not including comments.)
     */
    charTable['\n'] = STARTWS;
    charTable['\\'] = STARTWS;
    charTable['@'] = STARTWS;

    /*
     * Alphabetic.
     */
    charTable['_'] = LETTER;
    charTable['$'] = LETTER;	/* Some compilers permit this; why not? */

    for (i = 'a'; i <= 'z'; ++i)
	charTable[i] = LETTER;
    for (i = 'A'; i <= 'Z'; ++i)
	charTable[i] = LETTER;
    /*
     * Numeric.
     */
    for (i = '0'; i <= '9'; ++i)
	charTable[i] = DIGIT;

    /*
     * Quote Marks.
     */
    charTable['\''] = SINGLEQUOTE;
    charTable['"'] = DOUBLEQUOTE;

    /*
     * Potential decimal point.
     * OR Could be sign in floating number.
     * OR Possible first character of "other" multiple character token.
     * OR Always single character token.
     */
    charTable['('] = TOK_LPAREN;
    charTable[')'] = TOK_RPAREN;
    charTable['['] = TOK_LSQUARE;
    charTable[']'] = TOK_RSQUARE;
    charTable['{'] = TOK_LCURLY;
    charTable['}'] = TOK_RCURLY;
    charTable[','] = TOK_COMMA;
    charTable['='] = TOK_EQUALS;
    charTable['?'] = TOK_QMARK;
    charTable[':'] = TOK_COLON;
    charTable['|'] = TOK_VERTICAL;
    charTable['^'] = TOK_CARROT;
    charTable['&'] = TOK_AMPER;
    charTable['>'] = TOK_GREATER;
    charTable['<'] = TOK_LESSER;
    charTable['+'] = TOK_PLUS;
    charTable['-'] = TOK_DASH;
    charTable['*'] = TOK_STAR;
    charTable['%'] = TOK_PERCENT;
    charTable['/'] = TOK_SLASH;
    charTable['!'] = TOK_EXCLAIM;
    charTable['~'] = TOK_TILDE;
    charTable['.'] = TOK_PERIOD;
    charTable[';'] = TOK_SEMICOLON;
#ifdef MVS
    charTable[0x6a] = TOK_VERTICAL;	/* EBCDIC representation of the broken bar, used
					   interchageably with the solid bar under C/370 */
#endif /* MVS */
}

/*
* scanComment: Skip input characters up to and including comment terminator.
*/
static void
scanComment(void)
{
    int prevC;
    int c;

    prevC = ' ';

    while (1) {
	c = getc(srcin);
	++src.col;

	switch (c) {
	case '/':
	    if (prevC == '*')
		return;
	    break;
	case '\n':
	    src.line++;
	    src.col = 0;
	    break;
	case EOF:
	    lexical_error(&src, "unterminated comment");
	    return;
	}

	prevC = c;
    }
}

/*
* expandedMacro: Return the next input character (including c) that is
*	part of the macro expansion (i.e. not part of the original unexpanded
*	macro text or macro expansion markers).
*	An expanded macro apears in the input as:
*
*		@< unexpanded-macro-text @| macro-expansion @>
*
*	On EOF return EOF.
*/
static int
expandedMacro(void)
{
    register int c;

    c = getc(srcin);
    switch (c) {
    case '<':
	if (macro_nesting == 0) {
	    m_begin.file = src.file;
	    m_begin.line = src.line;
	    m_begin.col = src.col;
	    m_end.file = src.file;
	    m_end.line = src.line;
	    m_end.col = src.col - 1;	/* Exclude @ */
	    while (1) {
		c = getc(srcin);
		++m_end.col;
		if (c == EOF) {
		    lexical_error(&m_end, "unexpected EOF");
		    return EOF;
		}
		if (c == '\n') {
		    ++m_end.line;
		    m_end.col = 0;
		    continue;
		}
		if (c != '@')
		    continue;
		c = getc(srcin);
		++m_end.col;
		if (c == EOF) {
		    lexical_error(&m_end, "unexpected EOF");
		    return EOF;
		}
		if (c == '|')
		    break;
		else
		    ungetc(c, srcin);	/* in case its an @ */
	    }
	    m_end.col -= 2;	/* Exclude @| */
	} else {
	    while (1) {
		if ((c = getc(srcin)) == '@') {
		    if ((c = getc(srcin)) == '|')
			break;
		    else if (c == '>') {
			--macro_nesting;
		    } else
			ungetc(c, srcin);	/* @ ? */
		} else if (c == EOF) {
		    lexical_error(&m_end, "unexpected EOF");
		    return EOF;
		}
	    }
	}
	++macro_nesting;
	c = getc(srcin);
	if (c == '@') {		/* chk for @> i.e. empty expansion <@...@|@> */
	    c = getc(srcin);
	    if (c == '>') {	/* see case '>' below */
		if (--macro_nesting == 0)
		    src.col = m_end.col + 1;
		c = getc(srcin);
	    } else {
		ungetc(c, srcin);	/* @ ? */
		c = '@';
	    }
	}
	return c;
    case '>':
	if (--macro_nesting == 0) {
	    /*
	     * Now that we are outside the macro, where are we in the input?
	     * Cpp provides line directives (or extra lines) to make sure that
	     * src.line and src.file reflect the original source file input
	     * position.  Src.col must be reset to reflect the column in the
	     * original source file rather than the column in the replacement
	     * text.
	     */
	    src.col = m_end.col + 1;
	}
	c = getc(srcin);
	return c;
    default:
	ungetc(c, srcin);
	--src.col;
	return '@';
    }
}

/*
* scanSpaces: Return the next input character (including c) that is
*	not whitespace (excluding newline).  On EOF return EOF.
*/
static int
scanSpaces(int c)
{
    while (1) {
	switch (c) {
	case '@':
	    c = expandedMacro();
	    if (c == '@')
		return c;
	    continue;
	case '/':
	    c = getc(srcin);
	    ++src.col;
	    if (c != '*') {
		ungetc(c, srcin);
		--src.col;
		return '/';
	    }
	    scanComment();
	    break;
	case ' ':
	case '\t':
	case '\b':
	case '\f':
	case '\r':
#ifdef __STDC__
	case '\v':		/* ANSI allows vertical tab as white space */
#else /* not __STDC__ */
	case '\013':		/* ANSI allows vertical tab as white space */
#endif /* not __STDC__ */
	    break;
	case '\\':
	    c = getc(srcin);
	    ++src.col;
	    if (c == '\n') {
		src.line++;
		src.col = 0;
		break;
	    } else
		continue;	/* Ignore invalid backslash. */
	default:
	    return (c);
	}

	c = getc(srcin);
	++src.col;
    }
}

/*
* poundDirective: Scan # directives:
*
*	#				[ignore]
*	#pragma ...			[ignore]
*	#ident "..." ...		[ignore]
*	#line n				[set line number]
*	#line n "file" ...		[set line number and set file name]
*	#n				[set line number]
*	#n "file" ...			[set line number and set file name]
*
*	Always return with input at beginning of a line.
*/
static void
poundDirective(void)
{
    int c;
    int token;
    int i;
    char directive[MAX_DIRECTIVE_SIZE];
    int charType;
#ifdef MVS
    static FILE *altfile;
#endif

    c = scanSpaces(' ');

    /*
     * If # is the only nonwhite char on the line ignore it.
     */
    if (c == '\n') {
	src.line++;
	src.col = 0;
	return;
    }

    charType = charTable[CHAR(c)];
    if (charType == LETTER) {
	directive[0] = c;
	for (i = 1; i < MAX_DIRECTIVE_SIZE; ++i) {
	    c = getc(srcin);
	    ++src.col;
	    charType = charTable[CHAR(c)];
	    if (charType == LETTER || charType == DIGIT) {
		directive[i] = c;
	    } else {
		directive[i] = '\0';
		break;
	    }
	}
	if (i == MAX_DIRECTIVE_SIZE) {
	    lexical_error(&src, "unknown #-directive");
	    while ((c = scanSpaces(' ')) != EOF && c != '\n') ;
	    src.line++;
	    src.col = 0;
	    return;
	}

	c = scanSpaces(c);

	if (strncmp(directive, "pragma", MAX_DIRECTIVE_SIZE) == 0) {
#ifndef MVS
	    while (c != '\n' && (c = scanSpaces(' ')) != EOF) ;
#else /* MVS */
	    if (altfile == NULL) {
		altfile = fopen("DD:PRAGMAS", "w");
		if (altfile == NULL)
		    altfile = (FILE *) -1;
	    }
	    if (altfile == (FILE *) -1)
		while (c != '\n' && (c = scanSpaces(' ')) != EOF) ;
	    else {
		fprintf(altfile, "%s", "#pragma ");
		do {
		    putc(c, altfile);
		}
		while (c != '\n' && (c = getc(srcin)) != EOF);

	    }
#endif /* MVS */
	    src.line++;
	    src.col = 0;
	    return;
	}

	if (strncmp(directive, "ident", MAX_DIRECTIVE_SIZE) == 0) {
	    /*
	     * A string constant should follow.
	     * If no argument, ignore the line.
	     */
	    if (c != '\n') {
		ungetc(c, srcin);
		token = yylex();
		if (token != STRING)
		    lexical_error(&src, "invalid #ident");

		/* Skip the rest of this line.  */
		while ((c = scanSpaces(' ')) != EOF && c != '\n') ;
	    }
	    src.line++;
	    src.col = 0;
	    return;
	}

	if (strncmp(directive, "line", MAX_DIRECTIVE_SIZE) != 0) {
	    lexical_error(&src, "unknown #-directive");
	    while (c != '\n' && (c = scanSpaces(' ')) != EOF) ;
	    src.line++;
	    src.col = 0;
	    return;
	}
    }

    /*
     * Here we should have either `#line' or `# <nonletter>'.
     * In either case, it should be a line number; a digit should follow.
     */

    ungetc(c, srcin);
    token = yylex();

    if (token == ICON) {
	/*
	 * The value is the line number of the next input line.
	 */
	int line = atoi(yylval.token.text);

	/* Is this the last nonwhite stuff on the line?  */
	c = scanSpaces(' ');
	ungetc(c, srcin);
	if (c != '\n') {

	    /* More follows: must be a string constant (filename). */

	    token = yylex();
	    if (token != STRING)
		lexical_error(&src, "invalid #line");

	    src.file = store_filename(yylval.token.text);
	}
	src.line = line - 1;
    } else
	lexical_error(&src, "invalid #line");

    /* skip the rest of this line.  */
    while ((c = scanSpaces(' ')) != EOF && c != '\n') ;
    ++src.line;
    src.col = 0;
    return;
}

/*
* scanWhiteSpace: Return the next input character (including c) that is
*	not whitespace or newline or a #-directive.
*	On EOF return EOF.
*/
static int
scanWhiteSpace(int c)
{
    int firstNonWhite = 0;

    while (1) {
	c = scanSpaces(c);

	switch (c) {
	case '\n':
	    firstNonWhite = 1;
	    ++src.line;
	    src.col = 0;
	    break;
	case '#':
	    if (firstNonWhite == 0)
		return c;
	    poundDirective();
	    break;
	default:
	    return c;
	}

	c = getc(srcin);
	++src.col;
    }
}

void
scan_setType(char *name)
{
    ID_TYPE *idType;
    typeFix_t *typeFix;

    strtab_insert(strtab, name, &idType);

    typeFix = (typeFix_t *) malloc(sizeof *typeFix);
    CHECK_MALLOC(typeFix);
    typeFix->idType = idType;
    typeFix->oldType = *idType;
    if (typeFixList == 0)
	typeFixList = list_create();
    list_put(typeFixList, typeFix);

    *idType = T_NAME;
}

void
scan_pushScope(void)
{
    list_put(scopeList, typeFixList);
    typeFixList = 0;
}

int
scan_popScope(void)
{
    LIST *t;
    typeFix_t *typeFix;

    if (typeFixList) {
	for (t = 0; LIST_NEXT(typeFixList, &t, &typeFix);) {
	    *typeFix->idType = typeFix->oldType;
	    free(typeFix);
	    list_delete(typeFixList, &t);
	}
	list_free(typeFixList, NULL);
    }

    t = 0;
    if (LIST_PREV(scopeList, &t, &typeFixList)) {
	list_delete(scopeList, &t);
	return 1;
    } else {
	typeFixList = 0;
	return 0;
    }
}

/*
* scan_init: Initialize scanner.  Must be called before yylex().
*/
void
scan_init(
	     FILE *srcfile)
{
    int c;
    ID_TYPE *idType;
    static int firstCall = 1;
    size_t i;

    if (firstCall) {
	firstCall = 0;

	/*
	 * Initialize char table.
	 */
	initCharTable();
    }

    srcin = srcfile;

    bufSize = BUF_MALLOC_SIZE;
    buf = (char *) malloc(bufSize);
    CHECK_MALLOC(buf);

    /*
       * Start line number at 0, because scanWhiteSpace() is
       * called at the very beginning and will increment it to 1.
     */
    src.line = 0;
    src.col = 0;

    /*
       * Create type fix list and scope list.
     */
    typeFixList = 0;
    scopeList = list_create();

    /*
       * Create string table.
     */
    strtab = (struct strtab *) strtab_create();

    /*
       * Insert keywords in string table.
     */
    for (i = 0; i < (sizeof keyword / sizeof *keyword); ++i) {
	strtab_insert(strtab, keyword[i].name, &idType);
	*idType = keyword[i].type;
    }

    /*
       * Scan upto first token.
     */
    c = scanWhiteSpace('\n');	/* Pretend first char was preceeded by \n */
    ungetc(c, srcin);
    --src.col;
}

void
scan_end(char **uprefix)
{
    *uprefix = (char *) strtab_upfix(strtab);

    free(buf);

    while (scan_popScope()) ;	/* free all scopes. */

    list_free(scopeList, NULL);
}

/*
* scanQuotedString:  Scan from srcin into buf + offset to and including quote
*	matching "quote" is seen.  
*/
static void
scanQuotedString(
		    FILE *srcin2,
		    size_t offset,
		    int quote)
{
    int c;
    size_t i;
    int escape;

    i = offset;
    escape = 0;

    while (1) {
	c = getc(srcin2);
	++src.col;
	buf[i++] = c;
	if (i == bufSize) {
	    bufSize += BUF_MALLOC_SIZE;
	    buf = (char *) realloc(buf, bufSize);
	    CHECK_MALLOC(buf);
	}
	if (c == EOF) {
	    lexical_error(&src, "unterminated quoted literal");
	    buf[i] = '\0';
	    return;
	}
	if (c == '\n') {
	    src.line++;
	    src.col = 0;
	} else if (c == '\\') {
	    if (escape)
		escape = 0;
	    else
		escape = 1;
	} else if (c == quote && !escape) {
	    buf[i] = '\0';
	    return;
	} else
	    escape = 0;
    }
}

static int
scanAssignOp(int c)
{
    int value;

    switch (c) {
    case '<':
	value = LT_EQ;
	break;
    case '>':
	value = GT_EQ;
	break;
    case '!':
	value = BANG_EQUAL;
	break;
    case '+':
	value = PLUS_EQ;
	break;
    case '-':
	value = MINUS_EQ;
	break;
    case '*':
	value = MUL_EQ;
	break;
    case '/':
	value = DIV_EQ;
	break;
    case '%':
	value = MOD_EQ;
	break;
    case '&':
	value = AND_EQ;
	break;
    case '|':
#ifdef MVS
    case '\x6a':		/* EBCDIC representation of the broken bar, used
				   interchageably with the solid bar under C/370 */
#endif /* MVS */
	value = OR_EQ;
	break;
    case '^':
	value = ER_EQ;
	break;
    default:
	value = 0;
	break;
    }

    return value;
}

/*
* scanNumber:  Scan number suffix from srcin into buf + offset.  Return
*	token type ICON or FCON.
*	Number suffix is of the form "[xXlLuU][0-9A-Z_.]*" or "" for ICON.
*	Number suffix is of the form "[.eEFf][0-9A-Z_.]*" for FCON.
*	(Many matches are not legal numbers, but they aren't legal
*	anything else either.  They will get checked by semantic analysis.)
*/
static int
scanNumber(
	      FILE *srcin2,
	      size_t offset,
	      int nextC)
{
    int c;
    size_t i;
    int value;
    int charType;

    i = offset;
    c = nextC;

    switch (c) {
    case 'x':
    case 'X':
    case 'l':
    case 'L':
    case 'u':
    case 'U':
	value = ICON;
	break;
    case '.':
    case 'e':
    case 'E':
    case 'f':
    case 'F':
	value = FCON;
	break;
    default:
	ungetc(c, srcin2);
	--src.col;
	buf[i] = '\0';
	return ICON;
    }

    do {
	buf[i++] = c;
	if (i == bufSize) {
	    bufSize += BUF_MALLOC_SIZE;
	    buf = (char *) realloc(buf, bufSize);
	    CHECK_MALLOC(buf);
	}
	c = getc(srcin2);
	++src.col;
	if ((c == '+' || c == '-') && (buf[i - 1] == 'e' || buf[i - 1] == 'E'))
	    charType = LETTER;
	else
	    charType = charTable[CHAR(c)];
    } while (charType == DIGIT || charType == LETTER ||
	     charType == TOK_PERIOD);
    ungetc(c, srcin2);
    --src.col;
    buf[i] = '\0';

    return value;
}

/*
* yylex:  Return type of next token.  yylval.token.text points to string table
*	entry for identifiers and literals.  yylval.token.srcpos identifies
*	the original source position of the token.
*/
int
yylex(void)
{
    int c;
    int charType;
    ID_TYPE *idType;
    int value = 0;

    yylval.token.text = NULL;

    /*
       * Skip white space.
     */
    do {
	c = getc(srcin);
	++src.col;
	charType = charTable[CHAR(c)];
    } while (charType == WHITESPACE);

    if (charType == STARTWS || charType == TOK_SLASH) {
	c = scanWhiteSpace(c);
	charType = charTable[CHAR(c)];
    }

    if (macro_nesting) {
	yylval.token.srcpos[LEFT_SRCPOS].file = m_begin.file;
	yylval.token.srcpos[LEFT_SRCPOS].line = m_begin.line;
	yylval.token.srcpos[LEFT_SRCPOS].col = m_begin.col;
    } else {
	yylval.token.srcpos[LEFT_SRCPOS].file = src.file;
	yylval.token.srcpos[LEFT_SRCPOS].line = src.line;
	yylval.token.srcpos[LEFT_SRCPOS].col = src.col;
    }

    switch (charType) {
    case LETTER:
	{
	    size_t i = 0;

	    if (c == 'L') {
		/*
		 * Check for ANSI wide string or wide character constant.
		 */
		c = getc(srcin);
		++src.col;
		if (c == '\'') {
		    buf[0] = 'L';
		    buf[1] = c;
		    scanQuotedString(srcin, 2, c);
		    yylval.token.text = strtab_insert(strtab, buf, &idType);
		    value = ICON;
		    *idType = value;
		    break;
		} else if (c == '"') {
		    buf[0] = 'L';
		    buf[1] = c;
		    scanQuotedString(srcin, 2, c);
		    yylval.token.text = strtab_insert(strtab, buf, &idType);
		    value = STRING;
		    *idType = value;
		    break;
		} else {
		    ungetc(c, srcin);
		    --src.col;
		    c = 'L';
		}
	    }
	    /*
	       * Get rest of identifier.
	     */
	    do {
		buf[i++] = c;
		if (i == bufSize) {
		    bufSize += BUF_MALLOC_SIZE;
		    buf = (char *) realloc(buf, bufSize);
		    CHECK_MALLOC(buf);
		}
		c = getc(srcin);
		++src.col;
		charType = charTable[CHAR(c)];
	    } while (charType == LETTER || charType == DIGIT);
	    ungetc(c, srcin);
	    --src.col;
	    buf[i] = '\0';
	    yylval.token.text = strtab_insert(strtab, buf, &idType);
	    value = *idType;

	    if (value == 0) {
		value = NAME;
		*idType = value;
	    }
	}
	break;
	/*
	   * Single character self representing tokens.
	 */
    case TOK_LPAREN:
    case TOK_RPAREN:
    case TOK_COMMA:
    case TOK_COLON:
    case TOK_SEMICOLON:
    case TOK_QMARK:
    case TOK_LSQUARE:
    case TOK_RSQUARE:
    case TOK_LCURLY:
    case TOK_RCURLY:
    case TOK_TILDE:
	value = charType;
	break;

    case DIGIT:
	{
	    size_t i = 0;

	    /*
	     * Get integer part of number.
	     */
	    do {
		buf[i++] = c;
		if (i == bufSize) {
		    bufSize += BUF_MALLOC_SIZE;
		    buf = (char *) realloc(buf, bufSize);
		    CHECK_MALLOC(buf);
		}
		c = getc(srcin);
		++src.col;
		charType = charTable[CHAR(c)];
	    } while (charType == DIGIT);

	    /*
	       * Check for suffix (x, X, l, L, u, U, ., e, E, f, F).
	     */
	    if (charType == TOK_PERIOD || charType == LETTER) {
		value = scanNumber(srcin, i, c);
	    } else {
		buf[i] = '\0';
		ungetc(c, srcin);
		--src.col;
		value = ICON;
	    }
	    yylval.token.text = strtab_insert(strtab, buf, &idType);
	    *idType = value;
	}
	break;
    case TOK_PERIOD:
	/*
	   * Fractional part of floating point constant, just a dot, or ellipsis?
	 */
	c = getc(srcin);
	++src.col;
	if (c == '.') {
	    c = getc(srcin);
	    ++src.col;
	    if (c == '.') {
		value = ELLIPSIS;
	    } else {
		lexical_error(&src, "extraneous or missing period");
	    }
	} else {
	    ungetc(c, srcin);
	    --src.col;
	    if (charTable[CHAR(c)] == DIGIT) {
		value = scanNumber(srcin, 0, '.');
		yylval.token.text = strtab_insert(strtab, buf, &idType);
		*idType = value;
	    } else {
		value = TOK_PERIOD;
	    }
	}
	break;

	/*
	   * Character that may begin single or multi char token.
	 */
    case TOK_SLASH:		/* c is '/' but not a comment */
    case TOK_PLUS:
    case TOK_DASH:
    case TOK_EXCLAIM:
    case TOK_PERCENT:
    case TOK_AMPER:
    case TOK_STAR:
    case TOK_LESSER:
    case TOK_EQUALS:
    case TOK_GREATER:
    case TOK_CARROT:
    case TOK_VERTICAL:
	{
	    int nextC;

	    value = 0;

	    nextC = getc(srcin);
	    ++src.col;

	    if (c == nextC) {
		switch (c) {
		case '+':
		    value = PLUSPLUS;
		    break;
		case '-':
		    value = MINUSMINUS;
		    break;
		case '=':
		    value = EQUAL;
		    break;
		case '&':
		    value = ANDAND;
		    break;
		case '|':
#ifdef MVS
		case '\x6a':	/* EBCDIC representation of the broken bar, used
				   interchageably with the solid bar under C/370 */
#endif /* MVS */
		    value = OROR;
		    break;
		case '<':
		    nextC = getc(srcin);
		    ++src.col;
		    if (nextC == '=') {
			value = LS_EQ;
		    } else {
			value = LSHIFT;
			ungetc(nextC, srcin);
			--src.col;
		    }
		    break;
		case '>':
		    nextC = getc(srcin);
		    ++src.col;
		    if (nextC == '=') {
			value = RS_EQ;
		    } else {
			value = RSHIFT;
			ungetc(nextC, srcin);
			--src.col;
		    }
		    break;
		}
	    } else if (nextC == '=') {
		value = scanAssignOp(c);
	    } else if ((c == '-') && (nextC == '>')) {
		value = STREF;
		break;
	    }
	    if (value == 0) {
		ungetc(nextC, srcin);
		--src.col;
		value = charType;
	    }
	}
	break;
    case SINGLEQUOTE:
	buf[0] = c;
	scanQuotedString(srcin, 1, c);
	yylval.token.text = strtab_insert(strtab, buf, &idType);
	value = ICON;
	*idType = value;
	break;
    case DOUBLEQUOTE:
	buf[0] = c;
	scanQuotedString(srcin, 1, c);
	yylval.token.text = strtab_insert(strtab, buf, &idType);
	value = STRING;
	*idType = value;
	break;
    case ENDOFFILE:
	if (c == EOF)
	    value = -1;
	else
	    lexical_error(&src, "invalid character");	/* '\0377' */
	break;
    case BADCHAR:
    default:
	lexical_error(&src, "invalid character");
	break;
    }

    if (macro_nesting) {
	yylval.token.srcpos[RIGHT_SRCPOS].file = m_end.file;
	yylval.token.srcpos[RIGHT_SRCPOS].line = m_end.line;
	yylval.token.srcpos[RIGHT_SRCPOS].col = m_end.col;
    } else {
	yylval.token.srcpos[RIGHT_SRCPOS].file = src.file;
	yylval.token.srcpos[RIGHT_SRCPOS].line = src.line;
	yylval.token.srcpos[RIGHT_SRCPOS].col = src.col;
    }

#if DEBUG
    fprintf(stderr, "parse %d:%s\n", value, yylval.token.text);
#endif
    if (value == ASM || value == ATTRIBUTE) {
	int nest = 0;
	int done = FALSE;
#if DEBUG
	fprintf(stderr, "IGNORING...\n");
#endif
	while (((value = yylex()) >= 0) && !done) {
	    if (value == TOK_LPAREN)
		nest++;
	    else if (value == TOK_RPAREN)
		done = (--nest <= 0);
	}
#if DEBUG
	fprintf(stderr, "DONE, pass %d:%s\n", value, yylval.token.text);
#endif
    }
    return value;
}

void
yyerror(char *string)
{
    if (yylval.token.text)
	parse_error(&src, "%s at <%.20s>", string, buf);
    else
	parse_error(&src, string, 0, 0);
}
