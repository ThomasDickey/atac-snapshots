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

#if HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "portable.h"
#include "version.h"
#include "atacysis.h"
#ifndef vms
#ifndef MVS
#include <sys/stat.h>
#include <fcntl.h>
#endif /* not MVS */
#endif /* not vms */

static char const atac_to_bin_c[] = "$Id: atac_to_bin.c,v 3.12 2013/12/09 00:33:02 tom Exp $";
static char const bellcoreCopyRight[] =
"Copyright (c) 1993 Bell Communications Research, Inc. (Bellcore)";

/*
* @Log: atac_to_bin.c,v @
* Revision 3.9  2005/08/14 13:48:31  tom
* gcc warnings
*
* Revision 3.8  1995/12/29 21:25:35  tom
* adjust headers, prototyped for autoconfig
*
*Revision 3.7  94/04/04  13:51:16  saul
*Fix binary copyright.
*
*Revision 3.6  94/04/04  10:24:42  jrh
*Add Release Copyright
*
*Revision 3.5  93/08/11  14:38:34  saul
*atacysis.h include must precede sys/stat.h to avoid multiple sys/types.h
*
*Revision 3.4  93/08/10  14:48:54  ewk
*Fixed definition of time_t for vms, MVS, and unix.
*
*Revision 3.3  93/08/04  15:52:16  ewk
*Added MVS and solaris support.  Squelched some ANSI warnings.
*
* Revision 3.2  93/03/31  11:42:24  saul
*Void function should be int.
*
*Revision 3.1  93/03/26  11:15:04  saul
*Initial version.
*
*-----------------------------------------------end of log
*/

/* forward declarations */
static int putBinDotAtac(int fd, T_MODULE * mod);

#define ROUND(n) 	/* round up to char pointer boundary */		\
    (((n) + sizeof(char *) - 1) & ~(sizeof(char *) - 1))

static char *zeros = NULL;
#define PAD(fd,n)	write((fd), &zeros, ROUND(n) - (n))

/*
* putBinDotAtac:  Write out the data in mod in the following binary format.
*
*	T_HEADER T_FILE {T_BLK T_VAR T_CUSE T_PUSE}* T_FUNC strings "end\n"
*
*	T_HEADER has pointers to T_FILE and T_FUNC.  T_FUNC has pointers
*	to T_BLK, T_VAR, T_CUSE, and T_PUSE.  T_FILE, T_VAR, and T_FUNC
*	have pointers to strings.  All pointers (to structures or to strings)
*	are changed to offsets from the begining of the file.  Except for
*	string pointers, offsets are on "char *" boundaries
*	(i.e. divisible by 4).  Zero padding is added where necessary.
*/
static int			/* 0 for success; -1 for failure */
putBinDotAtac(int fd,
	      T_MODULE * mod)
{
    char *charPtr;
    T_FILE *file;
    T_FUNC *func;
    T_VAR *var;
    T_BLK *blkPtr;
    T_VAR *varPtr;
    T_CUSE *cusePtr;
    T_PUSE *pusePtr;
    int offset;
    int stringOffset;
    size_t writeSize;
    T_HEADER header;

    /*
     * Fill in T_HEADER and write it to the file.  Pointer offsets are
     * calculated based on where the data will be placed in the file.
     */
    strcpy(header.heading, DOT_ATAC_HEADING);
    strcpy(header.version, VERSION);
    header.nFiles = mod->n_file;
    header.nFuncs = mod->n_func;
    header.fileOffset = ROUND(sizeof header);
    offset = header.fileOffset + ROUND(mod->n_file * sizeof(T_FILE));
    for (func = mod->func; func < mod->func + mod->n_func; ++func) {
	offset +=
	    ROUND(func->n_blk * sizeof(T_BLK)) +
	    ROUND(func->n_var * sizeof(T_VAR)) +
	    ROUND(func->n_cuse * sizeof(T_CUSE)) +
	    ROUND(func->n_puse * sizeof(T_PUSE));
    }
    header.funcOffset = offset;

    writeSize = sizeof header;
    write(fd, &header, writeSize);
    PAD(fd, writeSize);

    /*
     * Calculate offset of strings at end of file.
     */
    stringOffset = offset + (mod->n_func * sizeof(T_FUNC));

    /*
     * T_FILE: Change pointers to offsets and write.
     */
    for (file = mod->file; file < mod->file + mod->n_file; ++file) {
	charPtr = file->filename;	/* Hold vname ptr. */
	file->filename = (char *) stringOffset;
	stringOffset += (int) strlen(charPtr) + 1;
	write(fd, file, sizeof *file);
	file->filename = charPtr;	/* Put it back. */
    }
    writeSize = mod->n_file * sizeof(T_FILE);
    PAD(fd, writeSize);

    /*
     * Do T_BLK, T_VAR, T_CUSE, T_PUSE for each T_FUNC.
     */
    for (func = mod->func; func < mod->func + mod->n_func; ++func) {
	/*
	 * T_BLK: Change pointers to offsets and write.
	 */
	writeSize = func->n_blk * sizeof(T_BLK);
	write(fd, func->blk, writeSize);
	PAD(fd, writeSize);
	/*
	 * T_VAR: Change pointers to offsets and write.
	 */
	for (var = func->var; var < func->var + func->n_var; ++var) {
	    charPtr = var->vname;	/* Hold vname ptr. */
	    var->vname = (char *) stringOffset;
	    stringOffset += (int) strlen(charPtr) + 1;
	    write(fd, var, sizeof *var);
	    var->vname = charPtr;	/* Put it back. */
	}
	writeSize = func->n_var * sizeof(T_VAR);
	PAD(fd, writeSize);
	/*
	 * T_CUSE: Change pointers to offsets and write.
	 */
	writeSize = func->n_cuse * sizeof(T_CUSE);
	write(fd, func->cuse, writeSize);
	PAD(fd, writeSize);
	/*
	 * T_PUSE: Change pointers to offsets and write.
	 */
	writeSize = func->n_puse * sizeof(T_PUSE);
	write(fd, func->puse, writeSize);
	PAD(fd, writeSize);
    }

    /*
     * T_FUNC: Change pointers to offsets and write.
     */
    offset = header.fileOffset + ROUND(mod->n_file * sizeof(T_FILE));
    for (func = mod->func; func < mod->func + mod->n_func; ++func) {
	charPtr = func->fname;	/* Hold fname ptr. */
	func->fname = (char *) stringOffset;
	stringOffset += (int) strlen(charPtr) + 1;

	blkPtr = func->blk;	/* Hold blk ptr. */
	func->blk = (T_BLK *) offset;
	offset += ROUND(func->n_blk * sizeof(T_BLK));

	varPtr = func->var;	/* Hold var ptr. */
	func->var = (T_VAR *) offset;
	offset += ROUND(func->n_var * sizeof(T_VAR));

	cusePtr = func->cuse;	/* Hold cuse ptr. */
	func->cuse = (T_CUSE *) offset;
	offset += ROUND(func->n_cuse * sizeof(T_CUSE));

	pusePtr = func->puse;	/* Hold puse ptr. */
	func->puse = (T_PUSE *) offset;
	offset += ROUND(func->n_puse * sizeof(T_PUSE));

	write(fd, func, sizeof *func);

	func->fname = charPtr;	/* put fname ptr back. */
	func->blk = blkPtr;	/* put blk ptr back. */
	func->var = varPtr;	/* put var ptr back. */
	func->cuse = cusePtr;	/* put cuse ptr back. */
	func->puse = pusePtr;	/* put puse ptr back. */
    }
    if (offset != header.funcOffset) {
	fprintf(stderr, "bad offset calculation: %d %d\n",
		offset, header.funcOffset);
	return -1;
    }

    /*
     * Strings: Write them out in the order that their offsets were
     * calculated above.
     */
    stringOffset = offset + (mod->n_func * sizeof(T_FUNC));
    for (file = mod->file; file < mod->file + mod->n_file; ++file) {
	writeSize = strlen(file->filename) + 1;
	write(fd, file->filename, writeSize);
	stringOffset += (int) writeSize;
    }
    for (func = mod->func; func < mod->func + mod->n_func; ++func) {
	for (var = func->var; var < func->var + func->n_var; ++var) {
	    writeSize = strlen(var->vname) + 1;
	    write(fd, var->vname, writeSize);
	    stringOffset += (int) writeSize;
	}
    }
    for (func = mod->func; func < mod->func + mod->n_func; ++func) {
	writeSize = strlen(func->fname) + 1;
	write(fd, func->fname, writeSize);
	stringOffset += (int) writeSize;
    }

    /*
     * Write trailer (not checked by reader).
     */
    if (write(fd, "end\n", 4) != 4) {
	fprintf(stderr, "write failure\n");
	return -1;
    }

    return 0;
}

int
main(int argc,
     char *argv[])
{
    int covCount;
    T_MODULE *mod;
    int fd;
    int i;

    /*
     * Check command line args.
     */
    if (argc <= 1 || *argv[1] == '-') {
	fprintf(stderr, "usage: %s file.atac ...\n", argv[0]);
	exit(1);
    }

    /*
     * Get T_MODULE data from .atac files.
     */
    mod = static_data(argc - 1, argv + 1, NULL,
		      OPTION_BLOCK | OPTION_PUSE | OPTION_CUSE, &covCount);

    /*
     * Call putBinDotAtac for each T_MODULE to write the binary .atac file.
     */
    for (i = 1; i < argc; ++i) {
	if (argc != 2)
	    fprintf(stderr, "%s:\n", argv[i]);
	fd = creat(argv[i], 0666);
	if (fd >= 0) {
	    putBinDotAtac(fd, mod + i - 1);
	    if (close(fd) != 0)
		perror(argv[i]);
	} else
	    perror(argv[i]);
    }

    /*
     * Free everything.
     */
    freeStatic(mod, argc - 1);

    return 0;
}
