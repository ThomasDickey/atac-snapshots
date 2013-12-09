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

#if HAVE_STDLIB_H
#include <stdlib.h>
#endif

#include <stdio.h>

#include "portable.h"
#include "atacysis.h"
#include "ramfile.h"
#include "man.h"

static char const mem_c[] = "$Id: mem.c,v 3.5 2013/12/08 20:10:23 tom Exp $";
/*
* @Log: mem.c,v @
* Revision 3.4  1995/12/29 21:24:41  tom
* adjust headers, prototyped for autoconfig
*
*Revision 3.3  94/04/04  10:25:42  jrh
*Add Release Copyright
*
*Revision 3.2  93/08/04  15:56:44  ewk
*Added MVS and solaris support.  Squelched some ANSI warnings.
*
* Revision 3.1  93/03/26  10:58:37  saul
*Add iCovNext  lStampNext fields for coverage vector packing.
*
*Revision 3.0  92/11/06  07:47:24  saul
*propagate to version 3.0
*
*Revision 2.4  92/10/30  09:55:12  saul
*include portable.h
*
*Revision 2.3  92/10/06  13:27:56  saul
*Not checking malloc vs. realloc correctly.
*
*Revision 2.2  92/09/08  10:12:16  saul
*changed trace format and data structures
*
*Revision 2.1  92/09/08  09:59:33  saul
*Purdue trace management
*
*-----------------------------------------------end of log
*/

/*
* check_block:  Allocate space necessary to include iBlock in func->blocks.
*	Initialize if necessary.
*/
void
check_block(functype * func,
	    int iBlock)
{
    int i;

    if (iBlock >= func->iBlockPool) {
	func->iBlockPool = ((iBlock / BLOCK_POOL_SIZE) + 1) * BLOCK_POOL_SIZE;
	if (NULL == func->blocks) {
	    func->blocks = (blocktype *)
		malloc(func->iBlockPool * sizeof(blocktype));
	} else {
	    func->blocks = (blocktype *)
		realloc(func->blocks, func->iBlockPool * sizeof(blocktype));
	}
	if (NULL == func->blocks) {
	    memoryError("allocating blocks");
	}
    }

    if (iBlock >= func->iBlockCount) {
	for (i = func->iBlockCount; i <= iBlock; ++i) {
	    func->blocks[i].coverage = (coveragetype *) NULL;
	    func->blocks[i].iTestCount = 0;
	    func->blocks[i].iCovNext = 0;
	}
	func->iBlockCount = iBlock + 1;
    }
}

/*
* check_var:  Allocate space necessary to include iVar in func->vars.
*	Initialize if necessary.
*/
void
check_var(functype * func,
	  int iVar)
{
    int i;

    if (iVar >= func->iVarPool) {
	func->iVarPool = ((iVar / VAR_POOL_SIZE) + 1) * VAR_POOL_SIZE;
	if (NULL == func->vars) {
	    func->vars = (vartype *) malloc(func->iVarPool * sizeof(vartype));
	} else {
	    func->vars = (vartype *)
		realloc(func->vars, func->iVarPool * sizeof(vartype));
	}
	if (NULL == func->vars) {
	    memoryError("allocating vars");
	}
    }

    if (iVar >= func->iVarCount) {
	for (i = func->iVarCount; i <= iVar; ++i) {
	    func->vars[i].cuses = (cusetype *) NULL;
	    func->vars[i].iCuseCount = 0;
	    func->vars[i].iCusePool = 0;
	    func->vars[i].puses = (pusetype *) NULL;
	    func->vars[i].iPuseCount = 0;
	    func->vars[i].iPusePool = 0;
	}
	func->iVarCount = iVar + 1;
    }
}

/*
* check_cuse:  Allocate space necessary to include iCuse in var->cuses.
*	Initialize if necessary.
*/
void
check_cuse(vartype * var,
	   int iCuse)
{
    int i;

    if (iCuse >= var->iCusePool) {
	var->iCusePool = ((iCuse / CUSE_POOL_SIZE) + 1) * CUSE_POOL_SIZE;
	if (NULL == var->cuses) {
	    var->cuses = (cusetype *) malloc(var->iCusePool * sizeof(cusetype));
	} else {
	    var->cuses = (cusetype *)
		realloc(var->cuses, var->iCusePool * sizeof(cusetype));
	}
	if (NULL == var->cuses) {
	    memoryError("allocating cuses");
	}
    }

    if (iCuse >= var->iCuseCount) {
	for (i = var->iCuseCount; i <= iCuse; ++i) {
	    var->cuses[i].coverage = (coveragetype *) NULL;
	    var->cuses[i].iTestCount = 0;
	    var->cuses[i].iCovNext = 0;
	    var->cuses[i].iDefine = -1;
	    var->cuses[i].iUse = -1;
	}
	var->iCuseCount = iCuse + 1;
    }
}

/*
* check_puse:  Allocate space necessary to include iPuse in var->puses.
*	Initialize if necessary.
*/
void
check_puse(vartype * var,
	   int iPuse)
{
    int i;

    if (iPuse >= var->iPusePool) {
	var->iPusePool = ((iPuse / PUSE_POOL_SIZE) + 1) * PUSE_POOL_SIZE;
	if (NULL == var->puses) {
	    var->puses = (pusetype *) malloc(var->iPusePool * sizeof(pusetype));
	} else {
	    var->puses = (pusetype *)
		realloc(var->puses, var->iPusePool * sizeof(pusetype));
	}
	if (NULL == var->puses) {
	    memoryError("allocating puses");
	}
    }

    if (iPuse >= var->iPuseCount) {
	for (i = var->iPuseCount; i <= iPuse; ++i) {
	    var->puses[i].coverage = (coveragetype *) NULL;
	    var->puses[i].iTestCount = 0;
	    var->puses[i].iCovNext = 0;
	    var->puses[i].iDefine = -1;
	    var->puses[i].iTo = -1;
	}
	var->iPuseCount = iPuse + 1;
    }
}

/*
* check_func:  Allocate space necessary to include iFunc in file->funcs.
*	Initialize if necessary.
*/
void
check_func(filetype * file,
	   int iFunc)
{
    int i;

    if (iFunc >= file->iFuncPool) {
	file->iFuncPool = ((iFunc / FUNC_POOL_SIZE) + 1) * FUNC_POOL_SIZE;
	if (NULL == file->funcs) {
	    file->funcs = (functype *)
		malloc(file->iFuncPool * sizeof(functype));
	} else {
	    file->funcs = (functype *)
		realloc(file->funcs, file->iFuncPool * sizeof(functype));
	}
	if (NULL == file->funcs) {
	    memoryError("allocating funcs");
	}
    }

    if (iFunc >= file->iFuncCount) {
	for (i = file->iFuncCount; i <= iFunc; ++i) {
	    file->funcs[i].blocks = (blocktype *) NULL;
	    file->funcs[i].iBlockCount = 0;
	    file->funcs[i].iBlockPool = 0;
	    file->funcs[i].vars = (vartype *) NULL;
	    file->funcs[i].iVarCount = 0;
	    file->funcs[i].iVarPool = 0;
	}
	file->iFuncCount = iFunc + 1;
    }
}

/*
* check_file:  Allocate space necessary to include iFile in rf->files.
*	Initialize if necessary.
*/
void
check_file(RAMFILE * rf,
	   int iFile)
{
    int i;

    if (iFile >= rf->iFilePool) {
	rf->iFilePool = ((iFile / FILE_POOL_SIZE) + 1) * FILE_POOL_SIZE;
	if (NULL == rf->files) {
	    rf->files = (filetype *)
		malloc(rf->iFilePool * sizeof(filetype));
	} else {
	    rf->files = (filetype *)
		realloc(rf->files, rf->iFilePool * sizeof(filetype));
	}
	if (NULL == rf->files) {
	    memoryError("allocating files");
	}
    }

    if (iFile >= rf->iFileCount) {
	for (i = rf->iFileCount; i <= iFile; ++i) {
	    rf->files[i].funcs = (functype *) NULL;
	    rf->files[i].iFuncCount = 0;
	    rf->files[i].iFuncPool = 0;
	    rf->files[i].pName = (char *) NULL;
	    rf->files[i].hdr.headers = (headerstype *) NULL;
	    rf->files[i].hdr.iHeaderCount = 0;
	    rf->files[i].hdr.iHeaderPool = 0;
	}
	rf->iFileCount = iFile + 1;
    }
}

/*
* check_member:  Allocate space necessary to include iMember in mem->members.
*	Initialize if necessary.
*/
void
check_member(membertype * mem,
	     int iMember)
{
    int i;

    if (iMember >= mem->iMemberPool) {
	mem->iMemberPool = ((iMember / MEMBER_POOL_SIZE) + 1) *
	    MEMBER_POOL_SIZE;
	if (NULL == mem->members) {
	    mem->members = (memberstype *)
		malloc(mem->iMemberPool * sizeof(memberstype));
	} else {
	    mem->members = (memberstype *)
		realloc(mem->members, mem->iMemberPool * sizeof(memberstype));
	}
	if (NULL == mem->members) {
	    memoryError("allocating members");
	}
    }

    if (iMember >= mem->iMemberCount) {
	for (i = mem->iMemberCount; i <= iMember; ++i) {
	    mem->members[i].pDate = (char *) NULL;
	    mem->members[i].iFamily = -1;
	    mem->members[i].pName = (char *) NULL;
	    mem->members[i].pVersion = (char *) NULL;
	    mem->members[i].iCost = -1;
	    mem->members[i].iFreqFlag = -1;
	    mem->members[i].iCorrupted = -1;
	    mem->members[i].iDelete = 0;
	}
	mem->iMemberCount = iMember + 1;
    }
}

/*
* check_header:  Allocate space necessary to include iHeader in hdr->headers.
*	Initialize if necessary.
*/
void
check_header(headertype * hdr,
	     int iHeader)
{
    int i;

    if (iHeader >= hdr->iHeaderPool) {
	hdr->iHeaderPool = ((iHeader / HEADER_POOL_SIZE) + 1) *
	    HEADER_POOL_SIZE;
	if (NULL == hdr->headers) {
	    hdr->headers = (headerstype *)
		malloc(hdr->iHeaderPool * sizeof(headerstype));
	} else {
	    hdr->headers = (headerstype *)
		realloc(hdr->headers, hdr->iHeaderPool * sizeof(headerstype));
	}
	if (NULL == hdr->headers) {
	    memoryError("allocating headers");
	}
    }

    if (iHeader >= hdr->iHeaderCount) {
	for (i = hdr->iHeaderCount; i <= iHeader; ++i) {
	    hdr->headers[i].pPath = (char *) NULL;
	    hdr->headers[i].stampVector = (stampstype *) NULL;
	    hdr->headers[i].lStampNext = (time_t) 0;
	    hdr->headers[i].iStampCount = -1;
	}
	hdr->iHeaderCount = iHeader + 1;
    }
}

/*
* check_family:  Allocate space necessary to include iFamily in fam->families.
*	Initialize if necessary.
*/
void
check_family(familytype * fam,
	     int iFamily)
{
    int i;

    if (iFamily >= fam->iFamilyPool) {
	fam->iFamilyPool = ((iFamily / FAMILY_POOL_SIZE) + 1) *
	    FAMILY_POOL_SIZE;
	if (NULL == fam->families) {
	    fam->families = (familiestype *)
		malloc(fam->iFamilyPool * sizeof(familiestype));
	} else {
	    fam->families = (familiestype *)
		realloc(fam->families, fam->iFamilyPool * sizeof(familiestype));
	}
	if (NULL == fam->families) {
	    memoryError("allocating families");
	}
    }

    if (iFamily >= fam->iFamilyCount) {
	for (i = fam->iFamilyCount; i <= iFamily; ++i) {
	    fam->families[i].family = (char *) NULL;
	}
	fam->iFamilyCount = iFamily + 1;
    }
}
