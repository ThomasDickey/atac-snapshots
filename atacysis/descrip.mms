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
#	$Header: /users/source/archives/atac.vcs/atacysis/RCS/descrip.mms,v 3.2 1994/04/04 10:28:30 jrh Exp $
#
#$Log: descrip.mms,v $
#Revision 3.2  1994/04/04 10:28:30  jrh
#FROM_KEYS
#
Revision 3.2  94/04/04  10:28:30  jrh
Add Release Copyright

Revision 3.1  93/07/09  13:24:34  saul
Derived from atacysis/makefile revision 3.1

#-----------------------------------------------end of log
#
#
# makefile for VMS (use "mms" to build.)
#
INCLUDES=[-]
CFLAGS=/debug/noopt/include=($(INCLUDES))

AOBJ=getfields.obj, print.obj, static.obj, trace.obj, main.obj,		-
	summary.obj, bdisp.obj,	rlist.obj, ddisp.obj, vms_disp.obj,	-
	vector.obj, cdisp.obj, pdisp.obj, risk.obj, error.obj,		-
	gmatch.obj, tab_disp.obj, pat_match.obj, fdisp.obj,		-
	srcfile_name.obj, highest.obj, greedy.obj, lib.obj,		-
	[-.atac_i]filestamp.obj
TMOBJ=atactm.obj, prev.obj, init.obj, tmerror.obj, lib.obj, mem.obj,	-
	pro.obj, dump.obj, columns.obj, pat_match.obj, gmatch.obj

ASRC=getfields.c print.c static.c trace.c main.c summary.c bdisp.c 	-
	rlist.c ddisp.c vms_disp.c vector.c cdisp.c pdisp.c risk.c	-
	error.c	gmatch.c tab_disp.c pat_match.c fdisp.c srcfile_name.c	-
	highest.c greedy.c lib.c atacysis.h disp.h [-.atac_i]filestamp.c

TMSRC=atactm.c prev.c init.c tmerror.c lib.c mem.c pro.c dump.c		-
	columns.c pat_match.c gmatch.c man.h ramfile.h

system :		atacysis.exe atactm.exe

print :
		@echo $(ASRC) $(TMSRC)

clean :
		 delete *.obj.*, atacysis.exe;*, atactm.exe;*

atacysis.exe :	$(AOBJ)
		$(LINK)/exe=atacysis $(AOBJ)

atactm.exe :	$(TMOBJ)
		$(LINK)/exe=atactm $(TMOBJ)

#
# ATACYSIS files
#

main.obj :	main.c atacysis.h

vms_disp.obj :	vms_disp.c disp.h $(INCLUDES)portable.h

bdisp.obj :	bdisp.c atacysis.h disp.h $(INCLUDES)portable.h

bdisp.obj :	fdisp.c atacysis.h $(INCLUDES)portable.h

ddisp.obj :	ddisp.c atacysis.h disp.h $(INCLUDES)portable.h

summary.obj :	summary.c atacysis.h $(INCLUDES)portable.h

print.obj :	print.c atacysis.h $(INCLUDES)portable.h

static.obj :	static.c atacysis.h $(INCLUDES)portable.h

trace.obj :	trace.c atacysis.h $(INCLUDES)portable.h

risk.obj :	risk.c atacysis.h $(INCLUDES)portable.h

vector.obj :	vector.c atacysis.h $(INCLUDES)portable.h

cdisp.obj :	cdisp.c atacysis.h $(INCLUDES)portable.h

pdisp.obj :	pdisp.c atacysis.h $(INCLUDES)portable.h

error.obj :	error.c $(INCLUDES)portable.h

tab_disp.obj :	tab_disp.c atacysis.h $(INCLUDES)portable.h

srcfile_name.obj : srcfile_name.c disp.h $(INCLUDES)portable.h

highest.obj :	highest.c atacysis.h $(INCLUDES)portable.h

greedy.obj :	greedy.c atacysis.h $(INCLUDES)portable.h

#
# ATACTM files
#

atactm.obj :	atactm.c man.h ramfile.h $(INCLUDES)portable.h

prev.obj :	man.h prev.c ramfile.h $(INCLUDES)portable.h

init.obj :	init.c man.h ramfile.h $(INCLUDES)portable.h

tmerror.obj :	tmerror.c man.h ramfile.h $(INCLUDES)portable.h

lib.obj :	lib.c man.h ramfile.h $(INCLUDES)portable.h

mem.obj :	man.h mem.c ramfile.h $(INCLUDES)portable.h

dump.obj :	dump.c man.h ramfile.h $(INCLUDES)version.h

pro.obj :	man.h pro.c ramfile.h $(INCLUDES)portable.h

#
# Shared files
#

pat_match.obj :	pat_match.c $(INCLUDES)portable.h

gmatch.obj :	gmatch.c $(INCLUDES)portable.h
