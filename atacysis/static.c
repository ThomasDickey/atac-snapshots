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

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include "portable.h"

#ifndef MVS
#include <sys/stat.h>
#endif /* MVS */

#include "atacysis.h"

static char const static_c[] =
	"$Header: /users/source/archives/atac.vcs/atacysis/RCS/static.c,v 3.14 2005/08/14 13:46:46 tom Exp $";
/*
* $Log: static.c,v $
* Revision 3.14  2005/08/14 13:46:46  tom
* gcc warnings
*
* Revision 3.13  1997/12/10 01:51:44  tom
* simplified ifdef
*
* Revision 3.12  1997/11/03 19:31:30  tom
* quick hack to prototype the qsort compare-function
*
* Revision 3.11  1995/12/29 21:24:41  tom
* adjust headers, prototyped for autoconfig
* fix compiler warnings (casts).
*
* Revision 3.10  94/04/04  10:26:20  jrh
* Add Release Copyright
* 
* Revision 3.9  94/01/31  14:51:24  saul
* Use cfile calls instead of getfields
* fstat changed to stat for binary .atac (broken by MVS changes in 3.3)
* 
* Revision 3.8  93/09/03  09:10:37  saul
* put back typedefs removed as in 3.6.
* 
* Revision 3.7  93/08/23  15:40:43  ewk
* Eliminated many casts for Solaris warnings by modifying type decls.
* 
* Revision 3.6  93/08/11  12:59:31  saul
* atacysis.h include must precede sys/stat.h to avoid multiple sys/types.h
* 
* Revision 3.5  93/08/11  10:53:16  saul
* fix CHECK_MALLOC
* 
* Revision 3.4  93/08/11  10:42:01  saul
* need #include <sys/types.h> for fstat
* 
* Revision 3.3  93/08/04  15:58:39  ewk
* Added MVS and solaris support.  Squelched some ANSI warnings.
* 
* Revision 3.2  93/03/26  11:18:17  saul
* Read binary .atac files.
* 
* Revision 3.1  92/12/03  08:52:58  saul
* Patch memory leak.  Enlarge initial memory allocations.  Comments.
* 
* Revision 3.0  92/11/06  07:47:39  saul
* propagate to version 3.0
* 
* Revision 2.6  92/10/30  09:55:40  saul
* include portable.h
* 
* Revision 2.5  92/10/08  10:08:12  saul
*  change file time stamp checking to work with compression
* 
* Revision 2.4  92/09/08  08:42:23  saul
* Coverage vector data structure.
* 
* Revision 2.3  92/07/10  11:20:43  saul
* file/line/col for new D,C,P display;detected infeasable counts; obsolete T_DECIS
* 
* Revision 2.2  92/03/17  15:27:14  saul
* copyright
* 
* Revision 2.1  91/06/19  13:10:09  saul
* Propagate to version 2.0
* 
* Revision 1.1  91/06/12  16:58:36  saul
* Aug 1990 baseline
* 
*-----------------------------------------------end of log
*/

/* forward declarations */
static void module P_((char *filename, T_MODULE *t_module, char *funcSelect));
static void fix_puse P_((T_VAR *var, int n_var, T_PUSE *puse, int n_puse, T_PUSE **new_puse, unsigned short *n_new_puse));
static void fix_cuse P_((T_VAR *var, int n_var, T_CUSE *cuse, int n_cuse, T_CUSE **new_cuse, unsigned short *n_new_cuse));
static int puse_cmp P_((T_PUSE *a, T_PUSE *b));
static int cuse_cmp P_((T_CUSE *a, T_CUSE *b));
static char *t_copy P_((char *from, int size, int number));
static char *stralloc P_((char *str));
static char *extend P_((char *p, size_t size));
static void getBinDotAtac P_((char *filename, T_MODULE *t_module, char *funcSelect));
static void bufSize_error P_((char *filename, int bufSize));
static void input_error P_((char *filename, int recno, char *s, char *arg1, char *arg2));

#if CC_HAS_PROTOS
#define QSORT_CMP (int (*)(const void *, const void *))
#else
#define QSORT_CMP /* nothing */
#endif

#define FIELD_ERROR(filename, recno, msg, field, ignored) \
	input_error(filename, recno, msg, (char *)field, (char *)ignored)

#define CHECK_MALLOC(p) ((p)?1:(internal_error("Out of memory\n", 0, 0),0))

#define MAXFIELDS	6

#define DECIS_VAR	"=decis="

static void
input_error(filename, recno, s, arg1, arg2)
char	*filename;
int	recno;
char	*s;
char	*arg1;
char	*arg2;
{
	fprintf(stderr, "%s: corrupted .atac format record %d\n",
		filename, recno);
#ifdef DEBUG
	fprintf(stderr, s, arg1, arg2);
#endif
	exit(1);
}

static void
bufSize_error(filename, bufSize)
char	*filename;
int	bufSize;
{ 
    fprintf(stderr, "%s: corrupted .atac format\n", filename);
    exit(1);
}

/*
* getBinDotAtac:  Read in static data from binary .atac file "filename" and
*	fill in t_module info.  If funcSelect is not NULL, disable functions
*	that do not match by the pattern.  The file begins with T_HEADER
*	which has pointers to T_FILE and T_FUNC.  T_FUNC has pointers
*	to T_BLK, T_VAR, T_CUSE, and T_PUSE.  All pointers (to structures
*	or to strings) in the file are offsets from the begining of the file.
*	The address of the buffer the file is read into must be added to
*	each offset to make it a real pointer.  T_module->header is set
*	to the address of the buffer so that FreeStatic() knows how to
*	free it.
*/
static void
getBinDotAtac(filename, t_module, funcSelect)
char		*filename;
T_MODULE	*t_module;
char		*funcSelect;
{
    FILE	*fp;
#ifndef MVS
    struct stat	sbuf;
#else    
    char	*bufp;
    int		readSize;
#endif /* MVS */
    char	*buf;
    int		bufSize;
    T_HEADER	*header;
    T_FILE	*file;
    T_FUNC	*func;
    T_VAR	*var;
    int		offset;
    int		i;
    int		j;

    /*
     * Allocate a buffer and read the whole file into it.
     */
    fp = fopen(filename, "r");
    if (fp == NULL) {
	perror(filename);
	exit(1);
    }
#ifndef MVS
    if (stat(filename, &sbuf) < 0) {
	perror(filename);
	exit(1);
    }
    buf = (char *)malloc((size_t)sbuf.st_size);
    CHECK_MALLOC(buf);
    bufSize = fread(buf, 1, (size_t)sbuf.st_size, fp);
    if (bufSize == 0) {
	perror(filename);
	exit(1);
    }
#else /* MVS */
#define READSIZE 1024
    buf = (char *)malloc(READSIZE);
    CHECK_MALLOC(buf);
    bufp = buf;
    bufSize = 0;
    while (1) {
	readSize = fread(bufp, 1, READSIZE, fp);
	if (readSize == 0)
	    break;
	bufSize += readSize;
	buf = (char *)realloc(buf, bufSize + READSIZE);
	CHECK_MALLOC(buf);
	bufp = buf + bufSize;
    }
    buf = (char *)realloc(buf, bufSize);
#endif /* MVS */
    if (fclose(fp) < 0) {
	perror(filename);
	exit(1);
    }
    
    /*
     * Check that the required header is present and that the file is not
     * too small to be valid.
     */
    if (bufSize < sizeof *header) 
	bufSize_error(filename, bufSize);
    buf[bufSize - 1] = '\0';	/* Make sure last string is null terminated. */
    header = (T_HEADER *)buf;
    if (strcmp(header->heading, DOT_ATAC_HEADING) != 0) {
	fprintf(stderr, "%s: not in .atac format\n", filename);
	exit(1);
    }
    if (bufSize < header->fileOffset + (header->nFiles * sizeof(T_FILE)))
	bufSize_error(filename, bufSize);
    if (bufSize < header->funcOffset + (header->nFuncs * sizeof(T_FUNC)))
	bufSize_error(filename, bufSize);

    /*
     * FILE: check sizes and change offsets to pointers.
     */
    file = (T_FILE *)(buf + header->fileOffset);
    for (i = 0; i < (int)header->nFiles; ++i) {
	if (bufSize <= (unsigned int)file[i].filename)
	    bufSize_error(filename, bufSize);
	file[i].filename = buf + (unsigned int)file[i].filename;
    }

    /*
     * FUNC: check sizes and change offsets to pointers.
     */
    func = (T_FUNC *)(buf + header->funcOffset);
    for (i = 0; i < (int)header->nFuncs; ++i) {
	/*
	 * FNAME: check sizes and change offsets to pointers.
	 * Mark unselected funcs to ignore.
	 */
	if (bufSize <= (unsigned int)func[i].fname)
	    bufSize_error(filename, bufSize);
	func[i].fname = buf + (unsigned int)func[i].fname;
	if (funcSelect && !patMatch(funcSelect, func[i].fname, 0))
	    func[i].ignore = 1;		/* 0 in input. */
	/*
	 * BLK: check sizes and change offsets to pointers.
	 */
	offset = (unsigned int)func[i].blk + func[i].n_blk * sizeof(T_BLK);
	if (bufSize < offset)
	    bufSize_error(filename, bufSize);
	func[i].blk = (T_BLK *)(buf + (unsigned int)func[i].blk);
	/*
	 * VAR: check sizes and change offsets to pointers.
	 */
	offset = (unsigned int)func[i].var + func[i].n_var * sizeof(T_VAR);
	if (bufSize < offset)
	    bufSize_error(filename, bufSize);
	func[i].var = (T_VAR *)(buf + (unsigned int)func[i].var);
	var = func[i].var;
	for (j = 0; j < (int)func[i].n_var; ++j) {
	    if (bufSize <= (unsigned int)var[j].vname)
		bufSize_error(filename, bufSize);
	    var[j].vname = buf + (unsigned int)var[j].vname;
	}
	/*
	 * CUSE: check sizes and change offsets to pointers.
	 */
	offset = (unsigned int)func[i].cuse + func[i].n_cuse * sizeof(T_CUSE);
	if (bufSize < offset)
	    bufSize_error(filename, bufSize);
	func[i].cuse = (T_CUSE *)(buf + (unsigned int)func[i].cuse);
	/*
	 * PUSE: check sizes and change offsets to pointers.
	 */
	offset = (unsigned int)func[i].puse + func[i].n_puse * sizeof(T_PUSE);
	if (bufSize < offset)
	    bufSize_error(filename, bufSize);
	func[i].puse = (T_PUSE *)(buf + (unsigned int)func[i].puse);
    }

    /*
     * Fill in data to return to caller.
     */
    t_module->header = header;
    t_module->atacfile = filename;
    t_module->n_file = header->nFiles;
    t_module->file = file;
    t_module->n_func = header->nFuncs;
    t_module->func = func;
    t_module->ignore = 0;		/* Fixed up by caller. */

    return;
}

#define T_CUSE_ALLOC	500
#define T_PUSE_ALLOC	400
#define T_VAR_ALLOC	100
#define T_BLK_ALLOC	500
#define T_FUNC_ALLOC	50
#define T_FILE_ALLOC	25

static char *
extend(p, size)
char	*p;
size_t	size;
{
    register char *newP;

    if (p) {
#ifdef DEBUG
	fprintf(stderr, "extend: %d\n", (int)size);
#endif /* DEBUG */
	newP = (char *)realloc(p, size);
    }
    else newP = (char *)malloc(size);

    CHECK_MALLOC(newP);

    return newP;
}

static char *
stralloc(str)
char *str;
{
	char *s;

	s = (char *)malloc(strlen(str) + 1);
	CHECK_MALLOC(s);
	strcpy(s, str);

	return s;
}

static char *
t_copy(from, size, number)
char *from;
int size;
int number;
{
	char	*buf;
	size_t	len;

	len = size * number;
	buf = (char *)malloc(len);
	CHECK_MALLOC(buf);
	memcpy(buf, from, len);

	return buf;
}

static int
cuse_cmp(a, b)
T_CUSE	*a;
T_CUSE	*b;
{
	if (a->varno != b->varno)
		return a->varno - b->varno;
	if (a->blk1 != b->blk1)
		return a->blk1 - b->blk1;
	else return a->blk2 - b->blk2;
}

static int
puse_cmp(a, b)
T_PUSE	*a;
T_PUSE	*b;
{
	if (a->varno != b->varno)
		return a->varno - b->varno;
	if (a->blk1 != b->blk1)
		return a->blk1 - b->blk1;
	if (a->blk2 != b->blk2)
		return a->blk2 - b->blk2;
	else return a->blk3 - b->blk3;
}
		
static void
fix_cuse(var, n_var, cuse, n_cuse, new_cuse, n_new_cuse)
T_VAR		*var;
int		n_var;
T_CUSE		*cuse;
int		n_cuse;
T_CUSE		**new_cuse;
unsigned short	*n_new_cuse;
{
	int	i;
	int	j;
	int	v;

	/*
	* Sort and drop duplicates.
	*/
	qsort(cuse, (size_t)n_cuse, sizeof(T_CUSE), QSORT_CMP cuse_cmp);
	j = 1;
	for (i = 1; i < n_cuse; ++i)
		if (cuse_cmp(cuse+i-1, cuse+i))
			cuse[j++] = cuse[i];

	/*
	* Build index in var.cstart
	*/
	v = -1;
	for (i = 0; i < j; ++i) {
		while (cuse[i].varno != v) {
			++v;
			var[v].cstart = i;
		}
	}
	while (++v < n_var) var[v].cstart = j;

	/*
	* Copy to exact fit storage.
	*/
	*new_cuse = (T_CUSE *)t_copy((char *)cuse, sizeof(T_CUSE), j);
	*n_new_cuse = j;
}
		
static void
fix_puse(var, n_var, puse, n_puse, new_puse, n_new_puse)
T_VAR		*var;
int		n_var;
T_PUSE		*puse;
int		n_puse;
T_PUSE		**new_puse;
unsigned short	*n_new_puse;
{
	int	i;
	int	j;
	int	v;

	/*
	* Sort and drop duplicates.
	*/
	qsort(puse, (size_t)n_puse, sizeof(T_PUSE), QSORT_CMP puse_cmp);
	j = 1;
	for (i = 1; i < n_puse; ++i)
		if (puse_cmp(puse+i-1, puse+i))
			puse[j++] = puse[i];

	/*
	* Build index in var.pstart
	*/
	v = -1;
	for (i = 0; i < j; ++i) {
		while (puse[i].varno != v) {
			++v;
			var[v].pstart = i;
		}
	}
	while (++v < n_var) var[v].pstart = j;

	/*
	* Copy to exact fit storage.
	*/
	*new_puse = (T_PUSE *)t_copy((char *)puse, sizeof(T_PUSE), j);
	*n_new_puse = j;
}
		
/*
* module:  Read in static data from .atac file "filename" and fill in
*	t_module info.  If funcSelect is not NULL, consider only functions
*	match by this pattern.
*/
static void
module(filename, t_module, funcSelect)
char		*filename;
T_MODULE	*t_module;
char		*funcSelect;
{
    static T_CUSE	*t_cuse = NULL;
    static int		t_cuse_size = 0;
    static T_PUSE	*t_puse = NULL;
    static int		t_puse_size = 0;
    static T_VAR	*t_var = NULL;
    static int		t_var_size = 0;
    static T_BLK	*t_blk = NULL;
    static int		t_blk_size = 0;
    static T_FUNC	*t_func = NULL;
    static int		t_func_size = 0;
    static T_FILE	*t_file = NULL;
    static int		t_file_size = 0;
    int			n_cuse;
    int			n_puse;
    int			n_var;
    int			n_blk;
    int			n_func;
    int			n_file;
    struct cfile	*cf;
    int			ifield[MAXFIELDS];
    int			i;
    int			func_ignore = 0;
    int			c;
    path_t		stringBuf;

    /*
     * It is not known initially how many functions are in the module
     * or how many blocks, variables, cuses or puses are in each function.
     * Temp areas t_* are used to store info for these objects.  Extend()
     * is called when it is necessary to enlarge a temp area.
     * t_*_size stores the allocated size of the temp area. n_* stores
     * the number of entries actually in use. When the object is complete,
     * an exact sized storage area is created
     * by t_copy().  The exact sized areas are returned in t_module.
     * If there are no entries, the t_module pointer is garbage.
     * The temp areas are freed when this function is called with
     * filename == NULL.  The static areas are freed when freeStatic()
     * is called with a list of modules.
     *
     * t_file and t_func are complete just before returning.
     * t_blk, t_var, t_cuse, t_puse are complete when a new function
     * is encountered (at 'F') and just before returning.
     */

    if (filename == NULL) {
	/*
	 * Free temp storage.
	 */
	if (t_cuse_size != 0) {
	    free(t_cuse);
	    t_cuse_size = 0;
	}
	if (t_puse_size != 0) {
	    free(t_puse);
	    t_puse_size = 0;
	}
	if (t_var_size != 0) {
	    free(t_var);
	    t_var_size = 0;
	}
	if (t_blk_size != 0) {
	    free(t_blk);
	    t_blk_size = 0;
	}
	if (t_func_size != 0) {
	    free(t_func);
	    t_func_size = 0;
	}
	if (t_file_size != 0) {
	    free(t_file);
	    t_file_size = 0;
	}

	return;
    }

    cf = (struct cfile *)cf_openIn(filename);
    if (cf == NULL) {
	perror(filename);
	exit(1);
    }

    /*
     * Binary .atac file ?
     */
    if ((c = cf_getFirstChar(cf)) == 'A') {
	cf_close(cf);
	getBinDotAtac(filename, t_module, funcSelect);
	return;
    }
    cf_close(cf);
    cf = (struct cfile *)cf_openIn(filename);
    if (cf == NULL) {
	perror(filename);
	exit(1);
    }

    n_cuse = 0;
    n_puse = 0;
    n_var = 0;
    n_blk = 0;
    n_file = 0;
    n_func = 0;
    while ((c = cf_getFirstChar(cf)) != EOF) {
	switch (c)
	{
	case 'v':		/* ignore version for now. */
	    break;
	case 'S':
	    if (t_file_size <= n_file) {
		t_file = (T_FILE *)extend((char *)t_file,
					  (n_file + T_FILE_ALLOC) *
					  sizeof(T_FILE));
#ifdef DEBUG
		fprintf(stderr, "t_file %s %d %d %d %d\n", filename, cf_lineNo(cf), n_file, T_FILE_ALLOC, sizeof(T_FILE));
#endif				/* DEBUG */
		t_file_size += T_FILE_ALLOC;
	    }
	    cf_getString(cf, stringBuf, sizeof stringBuf);
	    t_file[n_file].filename = stralloc(stringBuf);
	    t_file[n_file].chgtime = cf_getLong(cf);
	    ++n_file;
	    break;
	case 'F':
	    if (n_file == 0) input_error(filename, cf_lineNo(cf), 
		 "func input before file\n", 0, 0);
	    cf_getString(cf, stringBuf, sizeof stringBuf);
	    for (i = 0; i < 6; ++i) ifield[i] = cf_getLong(cf);
	    if (ifield[0] >= n_file)
		input_error(filename, cf_lineNo(cf),
			    "invalid fileno: %s\n", stringBuf, 0);
	    if (ifield[3] >= n_file)
		FIELD_ERROR(filename, cf_lineNo(cf),
			    "invalid fileno: %d\n", ifield[3], 0);
	    if (n_puse) {
		if ((int)t_func[n_func - 1].formalN_puse < n_puse)
		    t_func[n_func - 1].formalN_puse = n_puse;
		fix_puse(t_var, n_var, t_puse, n_puse, 
			 &t_func[n_func - 1].puse,
			 &t_func[n_func - 1].n_puse);
		n_puse = 0;
	    }
	    if (n_cuse) {
		if ((int)t_func[n_func - 1].formalN_cuse < n_cuse)
		    t_func[n_func - 1].formalN_cuse = n_cuse;
		fix_cuse(t_var, n_var, t_cuse, n_cuse, 
			 &t_func[n_func - 1].cuse,
			 &t_func[n_func - 1].n_cuse);
		n_cuse = 0;
	    }
	    if (n_var) {
		t_func[n_func - 1].n_var = n_var;
		t_func[n_func - 1].var = (T_VAR *)
		    t_copy((char *)t_var, sizeof(T_VAR), n_var);
		n_var = 0;
	    }
	    if (n_blk) {
		t_func[n_func - 1].n_blk = n_blk;
		t_func[n_func - 1].blk = (T_BLK *)
		    t_copy((char *)t_blk, sizeof(T_BLK), n_blk);
		n_blk = 0;
	    }
	    if (t_func_size <= n_func) {
		t_func = (T_FUNC *)extend((char *)t_func,
					  (n_func + T_FUNC_ALLOC) *
					  sizeof(T_FUNC));
#ifdef DEBUG
		fprintf(stderr, "t_func %s %d %d %d %d\n", filename, cf_lineNo(cf), n_func, T_FUNC_ALLOC, sizeof(T_FUNC));
#endif				/* DEBUG */
		t_func_size += T_FUNC_ALLOC;
	    }
	    t_func[n_func].fname = stralloc(stringBuf);
	    t_func[n_func].decis_var = -1;
	    t_func[n_func].n_blk = 0;
	    t_func[n_func].n_var = 0;
	    t_func[n_func].n_puse = 0;
	    t_func[n_func].n_cuse = 0;
	    t_func[n_func].formalN_puse = 0;
	    t_func[n_func].formalN_cuse = 0;
	    t_func[n_func].pos.start.file = ifield[0];
	    t_func[n_func].pos.start.line = ifield[1];
	    t_func[n_func].pos.start.col = ifield[2];
	    t_func[n_func].pos.end.file = ifield[3];
	    t_func[n_func].pos.end.line = ifield[4];
	    t_func[n_func].pos.end.col = ifield[5];
	    t_func[n_func].ignore = 0;
	    func_ignore = 0;
	    if (funcSelect && !patMatch(funcSelect, stringBuf, 0)) {
		t_func[n_func].ignore = 1;
		func_ignore = 1;
	    }
	    ++n_func;
	    break;
	case 'B':
	    if (func_ignore)	/* don't care about this func */
		break;
	    if (n_func == 0) input_error(filename, cf_lineNo(cf), 
			 "blk input before func\n", 0, 0);
	    for (i = 0; i < 6; ++i) ifield[i] = cf_getLong(cf);
	    if (ifield[0] >= n_file)
		FIELD_ERROR(filename, cf_lineNo(cf),
			    "invalid fileno: %d\n", ifield[0], 0);
	    if (ifield[3] >= n_file)
		FIELD_ERROR(filename, cf_lineNo(cf),
			    "invalid fileno: %d\n", ifield[3], 0);
	    if (t_blk_size <= n_blk) {
		t_blk = (T_BLK *)extend((char *)t_blk,
					(n_blk + T_BLK_ALLOC) *
					sizeof(T_BLK));
#ifdef DEBUG
		fprintf(stderr, "t_blk %s %d %d %d %d\n", filename, cf_lineNo(cf), n_blk, T_BLK_ALLOC, sizeof(T_BLK));
#endif				/* DEBUG */
		t_blk_size += T_BLK_ALLOC;
	    }
	    t_blk[n_blk].pos.start.file = ifield[0];
	    t_blk[n_blk].pos.start.line = ifield[1];
	    t_blk[n_blk].pos.start.col = ifield[2];
	    t_blk[n_blk].pos.end.file = ifield[3];
	    t_blk[n_blk].pos.end.line = ifield[4];
	    t_blk[n_blk].pos.end.col = ifield[5];
	    ++n_blk;
	    break;
	case 'V':
	    if (func_ignore)	/* don't care about this func */
		break;
	    if (n_func == 0) input_error(filename, cf_lineNo(cf), 
		 "var input before func\n", 0, 0);
	    if (t_var_size <= n_var) {
		t_var = (T_VAR *)extend((char *)t_var,
					(n_var + T_VAR_ALLOC) *
					sizeof(T_VAR));
#ifdef DEBUG
		fprintf(stderr, "t_var %s %d %d %d %d\n", filename, cf_lineNo(cf), n_var, T_VAR_ALLOC, sizeof(T_VAR));
#endif				/* DEBUG */
		t_var_size += T_VAR_ALLOC;
	    }
	    cf_getString(cf, stringBuf, sizeof stringBuf);
	    t_var[n_var].vname = stralloc(stringBuf);
	    if (strcmp(stringBuf, DECIS_VAR) == 0)
		t_func[n_func-1].decis_var = n_var;
	    ++n_var;
	    break;
	case 'c':
	    if (func_ignore)	/* don't care about this func */
		break;
	    if (n_func == 0) input_error(filename, cf_lineNo(cf), 
		 "cuse count before func\n", 0, 0);
	    t_func[n_func - 1].formalN_cuse = cf_getLong(cf);
	    break;
	case 'C':
	    if (func_ignore)	/* don't care about this func */
		break;
	    if (n_func == 0) input_error(filename, cf_lineNo(cf), 
		 "Cuse input before func\n", 0, 0);
	    for (i = 0; i < 3; ++i)
		ifield[i] = cf_getLong(cf);
	    if (ifield[0] >= n_var)
		FIELD_ERROR(filename, cf_lineNo(cf),
			    "C varno: %d\n", ifield[0], 0);
	    if (ifield[1] >= n_blk)
		FIELD_ERROR(filename, cf_lineNo(cf),
			    "C blkno: %d\n", ifield[1], 0);
	    if (ifield[2] >= n_blk)
		FIELD_ERROR(filename, cf_lineNo(cf),
			    "C blkno: %s\n", ifield[2], 0);
	    if (t_cuse_size <= n_cuse) {
		t_cuse = (T_CUSE *)extend((char *)t_cuse,
					  (n_cuse + T_CUSE_ALLOC) *
					  sizeof(T_CUSE));
#ifdef DEBUG
		fprintf(stderr, "t_cuse %s %d %d %d %d\n", filename, cf_lineNo(cf), n_cuse, T_CUSE_ALLOC, sizeof(T_CUSE));
#endif				/* DEBUG */
		t_cuse_size += T_CUSE_ALLOC;
	    }
	    t_cuse[n_cuse].varno = ifield[0];
	    t_cuse[n_cuse].blk1 = ifield[1];
	    t_cuse[n_cuse].blk2 = ifield[2];
	    if (cf_atFirstChar(cf)) {
		t_cuse[n_cuse].def = t_blk[ifield[1]].pos;
		t_cuse[n_cuse].use = t_blk[ifield[2]].pos;
	    } else {
		t_cuse[n_cuse].def.start.file = cf_getLong(cf);
		t_cuse[n_cuse].def.start.line = cf_getLong(cf);
		t_cuse[n_cuse].def.start.col = cf_getLong(cf);
		t_cuse[n_cuse].def.end.file = cf_getLong(cf);
		t_cuse[n_cuse].def.end.line = cf_getLong(cf);
		t_cuse[n_cuse].def.end.col = cf_getLong(cf);
		t_cuse[n_cuse].use.start.file = cf_getLong(cf);
		t_cuse[n_cuse].use.start.line = cf_getLong(cf);
		t_cuse[n_cuse].use.start.col = cf_getLong(cf);
		t_cuse[n_cuse].use.end.file = cf_getLong(cf);
		t_cuse[n_cuse].use.end.line = cf_getLong(cf);
		t_cuse[n_cuse].use.end.col = cf_getLong(cf);
	    }
	    ++n_cuse;
	    break;
	case 'p':
	    if (func_ignore)	/* don't care about this func */
		break;
	    if (n_func == 0) input_error(filename, cf_lineNo(cf), 
		 "puse count before func\n", 0, 0);
	    t_func[n_func - 1].formalN_puse = cf_getLong(cf);
	    break;
	case 'P':
	    if (func_ignore)	/* don't care about this func */
		break;
	    if (n_func == 0) input_error(filename, cf_lineNo(cf), 
		 "Puse input before func\n", 0, 0);
	    for (i = 0; i < 4; ++i)
		ifield[i] = cf_getLong(cf);
	    if (ifield[0] >= n_var)
		FIELD_ERROR(filename, cf_lineNo(cf),
			    "P varno: %d\n", ifield[0], 0);
	    if (ifield[1] >= n_blk)
		FIELD_ERROR(filename, cf_lineNo(cf),
			    "P blkno: %s\n", ifield[1], 0);
	    if (ifield[2] >= n_blk)
		FIELD_ERROR(filename, cf_lineNo(cf),
			    "P blkno: %s\n", ifield[2], 0);
	    if (ifield[3] >= n_blk)
		FIELD_ERROR(filename, cf_lineNo(cf),
			    "P blkno: %s\n", ifield[3], 0);
	    if (t_puse_size <= n_puse) {
		t_puse = (T_PUSE *)extend((char *)t_puse,
					  (n_puse + T_PUSE_ALLOC) *
					  sizeof(T_PUSE));
#ifdef DEBUG
		fprintf(stderr, "t_puse %s %d %d %d %d\n", filename, cf_lineNo(cf), n_puse, T_PUSE_ALLOC, sizeof(T_PUSE));
#endif				/* DEBUG */
		t_puse_size += T_PUSE_ALLOC;
	    }
	    t_puse[n_puse].varno = ifield[0];
	    t_puse[n_puse].blk1 = ifield[1];
	    t_puse[n_puse].blk2 = ifield[2];
	    t_puse[n_puse].blk3 = ifield[3];
	    if (cf_atFirstChar(cf)) {
		t_puse[n_puse].def = t_blk[ifield[1]].pos;
		t_puse[n_puse].use = t_blk[ifield[2]].pos;
		*t_puse[n_puse].value = '\0';
	    } else {
		t_puse[n_puse].def.start.file = cf_getLong(cf);
		t_puse[n_puse].def.start.line = cf_getLong(cf);
		t_puse[n_puse].def.start.col = cf_getLong(cf);
		t_puse[n_puse].def.end.file = cf_getLong(cf);
		t_puse[n_puse].def.end.line = cf_getLong(cf);
		t_puse[n_puse].def.end.col = cf_getLong(cf);
		t_puse[n_puse].use.start.file = cf_getLong(cf);
		t_puse[n_puse].use.start.line = cf_getLong(cf);
		t_puse[n_puse].use.start.col = cf_getLong(cf);
		t_puse[n_puse].use.end.file = cf_getLong(cf);
		t_puse[n_puse].use.end.line = cf_getLong(cf);
		t_puse[n_puse].use.end.col = cf_getLong(cf);
		cf_getString(cf, t_puse[n_puse].value, sizeof t_puse[0].value);
		t_puse[n_puse].value[sizeof t_puse[0].value - 1] = '\0';
	    }
	    ++n_puse;
	    break;
	case '#':
	    /* comment */
	    break;
	default:
	    FIELD_ERROR(filename, cf_lineNo(cf), "format: %c\n",
			c, 0);
	}
    }
	
    if (n_puse) {
	if ((int)t_func[n_func - 1].formalN_puse < n_puse)
	    t_func[n_func - 1].formalN_puse = n_puse;
	fix_puse(t_var, n_var, t_puse, n_puse, 
		 &t_func[n_func - 1].puse,
		 &t_func[n_func - 1].n_puse);
    }
    if (n_cuse) {
	if ((int)t_func[n_func - 1].formalN_cuse < n_cuse)
	    t_func[n_func - 1].formalN_cuse = n_cuse;
	fix_cuse(t_var, n_var, t_cuse, n_cuse, 
		 &t_func[n_func - 1].cuse,
		 &t_func[n_func - 1].n_cuse);
    }
    if (n_var) {
	t_func[n_func - 1].n_var = n_var;
	t_func[n_func - 1].var = (T_VAR *)
	    t_copy((char *)t_var, sizeof(T_VAR), n_var);
    }
    if (n_blk) {
	t_func[n_func - 1].n_blk = n_blk;
	t_func[n_func - 1].blk = (T_BLK *)
	    t_copy((char *)t_blk, sizeof(T_BLK), n_blk);
    }

    t_module->header = NULL;	/* used for binary .atac files */
    t_module->atacfile = filename;

    t_module->n_func = n_func;
    if (n_func) {
	t_module->func = (T_FUNC *)
	    t_copy((char *)t_func, sizeof(T_FUNC), n_func);
    }
    t_module->n_file = n_file;
    if (n_file) {
	t_module->file = (T_FILE *)
	    t_copy((char *)t_file, sizeof(T_FILE), n_file);
    } else input_error(filename, cf_lineNo(cf), "Empty file\n", 0, 0);

    /* now sort puse, cuse and setup d_start, p_start, c_start */
	
    cf_close(cf);
    return;
}

T_MODULE *
static_data(nfiles, files, funcSelect, options, covCount)
int	nfiles;
char	*files[];
char	*funcSelect;
int	options;
int	*covCount;
{
	int		i;
	int		j;
	T_MODULE	*t_module;
	int		covStart;
	int		modCovStart;

	/*
	* Allocate module structure for each static file.
	*/
	t_module = (T_MODULE *)malloc(nfiles * sizeof(T_MODULE));
	CHECK_MALLOC(t_module);

	/*
	* Read static files.
	*/
	for (i = 0; i < nfiles; ++i)
		module(files[i], t_module + i, funcSelect);

	module(NULL, NULL, NULL);	/* free temp storage */

	/*
	*  Assign covStart numbers.
	*/
	covStart = 0;
	for (i = 0; i < nfiles; ++i) {
	    modCovStart = covStart;
	    for (j = 0; j < (int)t_module[i].n_func; ++j) {
		if (t_module[i].func[j].ignore) continue;
		t_module[i].func[j].blkCovStart = covStart;
		if (options & OPTION_BLOCK) {
		    covStart += t_module[i].func[j].n_blk;
		}
		t_module[i].func[j].cUseCovStart = covStart;
		if (options & OPTION_CUSE) {
		    covStart += t_module[i].func[j].n_cuse;
		}
		t_module[i].func[j].pUseCovStart = covStart;
		if (options & OPTION_PUSE) {
		    covStart += t_module[i].func[j].n_puse;
		}
	    }
	    if (covStart == modCovStart) {
		t_module[i].ignore = 1;
	    } else {
		t_module[i].ignore = 0;
	    }
	}

	*covCount = covStart;

	return t_module;
}

void
freeStatic(mod, n_mod)
T_MODULE	*mod;
int		n_mod;
{
    int	i;
    int j;
    int k;

    for (i = 0; i < n_mod; ++i) {
	if (mod[i].header) {	/* binary.atac file */
	    free(mod[i].header);
	    continue;
	} else {
	    for (j = 0; j < (int)mod[i].n_file; ++j)
		free(mod[i].file[j].filename);
	    if (mod[i].n_file != 0)
		free(mod[i].file);
	    for (j = 0; j < (int)mod[i].n_func; ++j) {
		free(mod[i].func[j].fname);
		if (mod[i].func[j].n_blk != 0)
		    free(mod[i].func[j].blk);
		for (k = 0; k < (int)mod[i].func[j].n_var; ++k)
		    free(mod[i].func[j].var[k].vname);
		if (mod[i].func[j].n_var != 0)
		    free(mod[i].func[j].var);
		if (mod[i].func[j].n_cuse != 0)
		    free(mod[i].func[j].cuse);
		if (mod[i].func[j].n_puse != 0)
		    free(mod[i].func[j].puse);
	    }
	    if (mod[i].n_func != 0)
		free(mod[i].func);
	}
    }

    if (n_mod != 0)
	free(mod);
}
