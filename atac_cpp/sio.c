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

static char sio_c[] = 
	"$Header: /users/source/archives/atac.vcs/atac_cpp/RCS/sio.c,v 1.2 1994/04/04 10:22:31 jrh Exp $";
/*
*$Log: sio.c,v $
*Revision 1.2  1994/04/04 10:22:31  jrh
*FROM_KEYS
*
*Revision 1.2  94/04/04  10:22:31  jrh
*Add Release Copyright
*
*Revision 1.1  93/08/04  15:40:25  ewk
*Initial revision
*
*-----------------------------------------------end of log
*/
/*
* Replacement functions for open, close, read, write for systems that don't
* have these but DO have fopen, fclose, fread, fwrite (e.g. MVS). 
* Add the following to the caller:
*
*	#define O_RDONLY	"r"
*	#define O_WRONLY	"w"
*	#define O_RDWR		"r+"
*	#define open(path,oflag,mode)	sioOpen(path,oflag,mode)
*	#define close(fildes)		sioClose(fildes)
*	#define read(fildes,buf,nbyte)	sioRead(fildes,buf,nbyte)
*	#define write(fildes,buf,nbyte)	sioWrite(fildes,buf,nbyte)
*/
#include <stdio.h>

/* forward declarations */
int sioWrite();
int sioRead();
int sioClose();
int sioOpen();

#define MAX_FILES	50

static FILE *files[MAX_FILES] = { NULL, };

int
sioOpen(path, oflag, mode)
char	*path;
char	*oflag;		/* "r", "w", "a", "r+", "w+", "a+" as in fopen */
int	mode;		/* ignored */
{
    int	i;

    for (i = 0; i < MAX_FILES; ++i)
	if (files[i] == NULL) break;
    if (i >= MAX_FILES) {
	fprintf(stderr, "%s: too many open files\n", path);
	return -1;
    }

    files[i] = fopen(path, oflag);
    if (files[i] == NULL)
	return -1;

    return i;
}

int
sioClose(fildes)
int	fildes;
{
    FILE	*f;

    if (fildes >= MAX_FILES || fildes < 0)
	return -1;

    f  = files[fildes];

    if (f == NULL)
	return -1;

    files[fildes] = NULL;

    return fclose(f);
}

int
sioRead(fildes, buf, nbyte)
int		fildes;
char		*buf;
unsigned	nbyte;
{
    FILE	*f;

    if (fildes >= MAX_FILES || fildes < 0)
	return -1;

    f  = files[fildes];

    if (f == NULL)
	return -1;

    return fread(buf, 1, nbyte, f);
}

int
sioWrite(fildes, buf, nbyte)
int		fildes;
char		*buf;
unsigned	nbyte;
{
    FILE	*f;

    if (fildes >= MAX_FILES || fildes < 0)
	return -1;

    f  = files[fildes];

    if (f == NULL)
	return -1;

    return fwrite(buf, 1, nbyte, f);
}
