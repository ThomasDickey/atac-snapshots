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
static char vms_lbr_c[] = 
	"$Header: /users/source/archives/atac.vcs/atac_cpp/RCS/vms_lbr.c,v 3.2 1994/04/04 10:22:45 jrh Exp $";
/*
*$Log: vms_lbr.c,v $
*Revision 3.2  1994/04/04 10:22:45  jrh
*FROM_KEYS
*
*Revision 3.2  94/04/04  10:22:45  jrh
*Add Release Copyright
*
*Revision 3.1  93/07/09  09:15:28  saul
*Initial revision.
*
*-----------------------------------------------end of log
*/
#include <stdio.h>
#include <stsdef.h>
#include <ssdef.h>
#include <lbrdef.h>
#include <descrip.h>
#include <rms.h>

/*
* libGet returns a buffer containing the whole header file named by
* memName.  If libGet fails, NULL is returned.  The includePath is a
* comma separated list of places to look for the header file.
* Lines of the header file are separated by '\n' in the buffer.  The
* buffer is terminated by '\0'.  If includeLen is not NULL, the length
* of the buffer is returned there.  The buffer is freed by the caller.
* If fname is not NULL, the full pathname where the header file was
* found is returned there.  This is used on the #line directive to
* indicate the origin of the header file.  (For VMS we return
* pathname(membername) where pathname is the name of the library and
* membername is the name of the library member.)  *fname is freed by
* the caller.
*/

char *
libGet(memName, includePath, includeLen, fnamePtr)
char	*memName;
char	*includePath;
int	*includeLen;
char	**fnamePtr;
{
    int		txtrfa[2];
    int		status;
    int		libIndex;
    char	*basep;
    int		bsize;
    int		st_size;
    char	*fname;
    $DESCRIPTOR(libNameDsc, NULL);
    $DESCRIPTOR(libMemDsc, NULL);
    $DESCRIPTOR(recDsc, "");

    status = LBR$INI_CONTROL(&libIndex, &LBR$C_READ, &LBR$C_TYP_TXT, 0);
    if (! $VMS_STATUS_SUCCESS(status))
	return 0;

    libNameDsc.dsc$a_pointer = includePath;
    libNameDsc.dsc$w_length = strlen(includePath);
    status = LBR$OPEN(&libIndex, &libNameDsc, 0);
    if (! $VMS_STATUS_SUCCESS(status))
	return 0;

    libMemDsc.dsc$a_pointer = memName;
    libMemDsc.dsc$w_length = strlen(memName);
    status = LBR$LOOKUP_KEY(&libIndex, &libMemDsc, txtrfa);	
    if (! $VMS_STATUS_SUCCESS(status)) {
	LBR$CLOSE(&libIndex);
	return 0;
    }

    fname = (char *)xmalloc(strlen(includePath) + strlen(memName) + 3);
    sprintf(fname, "%s(%s)", includePath, memName);

    status = LBR$SET_LOCATE(&libIndex);
    if (! $VMS_STATUS_SUCCESS(status)) {
	LBR$CLOSE(&libIndex);
	return NULL;
    }

    bsize = 2000;
    st_size = 0;
    basep = (char *)xmalloc(bsize);

    for (;;) {
	status = LBR$GET_RECORD(&libIndex, 0, &recDsc);
	if (! $VMS_STATUS_SUCCESS(status)) {
	    if (status == RMS$_EOF) break;
	    free(basep);
	    LBR$CLOSE(&libIndex);
	    return NULL;
	}
	if (st_size + recDsc.dsc$w_length + 1 > bsize) { /* 1 extra for '\n' */
	    bsize *= 2;
	    basep = (char *)xrealloc(basep, bsize);
	}
	bcopy(recDsc.dsc$a_pointer, basep + st_size, recDsc.dsc$w_length);
	st_size += recDsc.dsc$w_length;
	basep[st_size++] = '\n';
    }
    basep = xrealloc(basep, st_size + 1);	/* 1 extra for trailing null */
    basep[st_size] = '\0';

    LBR$CLOSE(&libIndex);
    if (includeLen != NULL) *includeLen = st_size;
    if (fnamePtr != NULL) *fnamePtr = fname;
    return basep;
}
