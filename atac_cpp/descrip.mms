#****************************************************************
#Copyright (c) 1993 Bell Communications Research, Inc. (Bellcore)
#
#Permission to use, copy, modify, and distribute this material
#for any purpose and without fee is hereby granted, provided
#that the above copyright notice and this permission notice
#appear in all copies, and that the name of Bellcore not be
#used in advertising or publicity pertaining to this
#material without the specific, prior written permission
#of an authorized representative of Bellcore.  BELLCORE
#MAKES NO REPRESENTATIONS ABOUT THE ACCURACY OR SUITABILITY
#OF THIS MATERIAL FOR ANY PURPOSE.  IT IS PROVIDED "AS IS",
#WITHOUT ANY EXPRESS OR IMPLIED WARRANTIES.
#****************************************************************
#	$Header: /users/source/archives/atac.vcs/atac_cpp/RCS/descrip.mms,v 3.2 1994/04/04 10:22:55 jrh Exp $
#
# Makefile for VMS.  Use "mms" to build it.
#
#$Log: descrip.mms,v $
#Revision 3.2  1994/04/04 10:22:55  jrh
#FROM_KEYS
#
# Revision 3.2  94/04/04  10:22:55  jrh
# Add Release Copyright
# 
# Revision 3.1  93/07/09  10:25:45  saul
# Derived from atac_cpp/Makefile revision 3.0.
# 
#-----------------------------------------------end of log
#
INCLUDES=[-]
CFLAGS=/debug/noopt/include=($(INCLUDES))/define=ATAC_MODS

OBSTACK=obstack.obj

system :	atac_cpp.exe

clean :
		delete *.obj.*, atac_cpp.exe.*

atac_cpp.exe :	cccp.obj cexp.obj version.obj $(OBSTACKS) vms_lbr.obj
		$(LINK)/exe=atac_cpp cccp.obj, cexp.obj, version.obj, -
			vms_lbr.obj
		
obstack.obj :	obstack.c obstack.h $(INCLUDES)portable.h

cexp.c :	cexp.y
		$(YACC) cexp.y
		mv y.tab.c cexp.c

cexp.obj :	cexp.c config.h $(INCLUDES)portable.h

cccp.obj :	cccp.c alloca.c tm.h config.h $(INCLUDES)portable.h
