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

#include <stdio.h>

#include "portable.h"
#include "atacysis.h"

static char const print_c[] =
	"$Header: /users/source/archives/atac.vcs/atacysis/RCS/print.c,v 3.5 1995/12/27 20:13:58 tom Exp $";
/*
* $Log: print.c,v $
* Revision 3.5  1995/12/27 20:13:58  tom
* adjust headers, prototyped for autoconfig
* correct gcc warnings (long vs int).
*
* Revision 3.4  94/04/04  10:26:01  jrh
* Add Release Copyright
* 
* Revision 3.3  93/11/02  11:53:51  saul
* Same as revision 3.1
* 
* Revision 3.3  93/11/02  11:49:08  saul
* Same as revision 3.1
* 
* Revision 3.1  93/08/04  15:57:47  ewk
* Added MVS and solaris support.  Squelched some ANSI warnings.
* 
* Revision 3.0  92/11/06  07:47:33  saul
* propagate to version 3.0
* 
* Revision 2.6  92/10/30  09:55:26  saul
* include portable.h
* 
* Revision 2.5  92/10/08  10:07:09  saul
* change file time stamp checking to work with compression
* 
* Revision 2.4  92/09/08  08:46:29  saul
* New coverage vector data structure.
* 
* Revision 2.3  92/07/10  11:09:35  saul
* removed obsolete T_DECIS; new POSITION struct
* 
* Revision 2.2  92/03/17  15:27:12  saul
* copyright
* 
* Revision 2.1  91/06/19  13:10:05  saul
* Propagate to version 2.0
* 
* Revision 1.1  91/06/12  16:58:31  saul
* Aug 1990 baseline
* 
*-----------------------------------------------end of log
*/

void
print_mod(mod, covVector)
T_MODULE	*mod;
int		*covVector;
{
	int i;
	int j;
	int cov;
	int tot;

	for (i = 0; i < mod->n_func; ++i) {
		printf("Func %d <%s>\n", i, mod->func[i].fname);
		cov = 0;
		tot = 0;
		for(j = 0; j < (int)mod->func[i].n_blk; ++j) {
		    if (covVector && 
			covVector[mod->func[i].blkCovStart + j] != -1)
		    {
			++tot;
			if (covVector[mod->func[i].blkCovStart + j] == 1) {
			    ++cov;
			}
		    }
		}
		printf("\tBlock %d of %d\n", cov, tot);
		cov = 0;
		tot = 0;
		for(j = 0; j < (int)mod->func[i].n_cuse; ++j) {
		    if (covVector && 
			covVector[mod->func[i].cUseCovStart + j] != -1)
		    {
			if (covVector[mod->func[i].cUseCovStart + j] == 1) {
			    ++cov;
			}
		    }
		}
		printf("\tCuse %d of %d\n", cov, tot);
		cov = 0;
		tot = 0;
		for(j = 0; j < (int)mod->func[i].n_puse; ++j) {
		    if (covVector &&
			covVector[mod->func[i].pUseCovStart + j] != -1)
		    {
			++tot;
			if (covVector[mod->func[i].pUseCovStart + j] == 1) {
			    ++cov;
			}
		    }
		}
		printf("\tPuse %d of %d\n", cov, tot);
	}
	for (i = 0; i < (int)mod->n_file; ++i)
		printf("File %d <%s> <%ld>\n", i, mod->file[i].filename,
			(long)(mod->file[i].chgtime));
	for (i = 0; i < (int)mod->n_func; ++i) {
		printf("Func %d <%s>\n", i, mod->func[i].fname);
		for(j = 0; j < (int)mod->func[i].n_blk; ++j)
			printf("\tBlock %d <%d %d %d> <%d %d %d> %d\n", j,
				mod->func[i].blk[j].pos.start.file,
				mod->func[i].blk[j].pos.start.col,
				mod->func[i].blk[j].pos.start.line,
				mod->func[i].blk[j].pos.end.file,
				mod->func[i].blk[j].pos.end.col,
				mod->func[i].blk[j].pos.end.line,
			        covVector ? 
			            covVector[mod->func[i].blkCovStart + j]
			            : 0);
		for(j = 0; j < (int)mod->func[i].n_var; ++j)
			printf("\tVar %d <%s> (%d %d)\n", j,
				mod->func[i].var[j].vname,
				mod->func[i].var[j].cstart,
				mod->func[i].var[j].pstart);
		for(j = 0; j < (int)mod->func[i].n_puse; ++j)
			printf("\tPuse %d <%d: %d %d %d> %d\n", j,
				mod->func[i].puse[j].varno,
				mod->func[i].puse[j].blk1,
				mod->func[i].puse[j].blk2,
				mod->func[i].puse[j].blk3,
				covVector ?
			            covVector[mod->func[i].pUseCovStart + j]
			            : 0);
		for(j = 0; j < (int)mod->func[i].n_cuse; ++j)
			printf("\tCuse %d <%d: %d %d> %d\n", j,
				mod->func[i].cuse[j].varno,
				mod->func[i].cuse[j].blk1,
				mod->func[i].cuse[j].blk2,
			        covVector ?
			            covVector[mod->func[i].cUseCovStart + j]
			            : 0);
	}
}
