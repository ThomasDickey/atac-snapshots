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
# $Header: /users/source/archives/atac.vcs/RCS/Makefile,v 3.21 1994/04/05 09:22:08 saul Exp $
# 
# Makefile for ATAC.  See README.
#

#
#-----------------------------------------------$Log: Makefile,v $
#-----------------------------------------------Revision 3.21  1994/04/05 09:22:08  saul
#-----------------------------------------------FROM_KEYS
#-----------------------------------------------
# Revision 3.21  94/04/05  09:22:08  saul
# Don't append contact info to README
# 
# Revision 3.20  94/04/04  09:46:08  jrh
# Add Release Copyright
# 
# Revision 3.19  93/09/01  16:59:41  ewk
# Change installation of tutorial/makefile to tutorial/Makefile.
# 
# Revision 3.18  93/08/09  12:49:11  saul
# loguse
# 
# Revision 3.17  93/07/13  11:13:26  ewk
# Add atacLD and atacLD.1 information.
# 
# Revision 3.16  93/07/11  09:03:27  saul
# continue after chmod on man1 fails
# drop install date from Version
# 
# Revision 3.15  93/06/07  15:38:20  ewk
# Now ignores failed attempts to remove directories in .../lib/atac.
# 
# Revision 3.14  93/05/17  12:24:57  ewk
# Added chmod calls to make lib/src, lib/tutorial, man/man1,
# and man/cat1 directories executable (755).
# 
# Revision 3.13  93/05/14  16:02:28  ewk
# Revised chmod calls to explicitly set permissions in bin, lib, and man.
# 
# Revision 3.12  93/05/05  11:28:32  ewk
# Removed the "print" target.  Specified all "rm's" and "chmod's"
# in the bin and man/man1 directories to avoid effecting user
# created executables and man pages.  Added a "chmod" to tutorial
# target.  Changed permissions so man pages are not made executable.
# 
# Revision 3.11  93/05/03  16:40:26  ewk
# Removed comments at end of lines not portable to Pyramid.
# Also, moved and added "rm" and "chmod" calls to avoid
# problems overwriting or appending to existing files during
# installation.
# 
# Revision 3.10  93/04/29  17:01:57  ewk
# Changed to reflect the name changes from atacLib
# to ataclib and atacMin to atacmin.
# 
# Revision 3.9  93/04/15  10:46:32  saul
# Avoid sh cannot create durring installation.
# Change contact to atac@****.bellcore.com
# 
# Revision 3.8  93/04/08  10:51:46  saul
# install all .1 files (man pages).
# 
# Revision 3.7  93/03/31  11:57:29  saul
# syntax error.
# 
# Revision 3.6  93/03/31  11:36:23  saul
# Change sence of if so false is in else branch.  Pyramid needs this.
# 
# Revision 3.5  93/03/26  10:52:41  saul
# Install atac_to_bin.
# 
# Revision 3.4  93/03/16  07:48:21  saul
# need "else; true;".  Correct err msg for ommitted INSTALLDIR
# 
# Revision 3.3  93/03/15  15:43:48  saul
# Installation improvements (LIBDIR, MANDIR, etc.)
# 
# Revision 3.2  93/02/23  13:13:51  saul
# traditional vs ansi cpp configurations
# 
# Revision 3.1  92/12/03  08:59:29  saul
# newform/pg portability
# 
# Revision 3.0  92/11/06  07:42:49  saul
# propagate to version 3.0
# 
# Revision 2.23  92/10/30  10:00:44  saul
# add CFLAGS=-I.. remove -Dvoid=int
# 
# Revision 2.22  92/10/29  11:18:32  saul
# New installation procedure.
# 
#-----------------------------------------------end of log

CFLAGS=-I.. -O
MAKE=make
INSTALLDIR=.
BINDIR=$(INSTALLDIR)/bin
LIBDIR=$(INSTALLDIR)/lib
SRCDIR=$(LIBDIR)/atac/src
TUTORIALDIR=$(LIBDIR)/atac/tutorial
MANDIR=$(INSTALLDIR)/man

system:	ATAC_I ATACYSIS ATAC_CPP TOOLS

clean:
	cd atac_i; $(MAKE) clean
	cd atacysis; $(MAKE) clean
	cd atac_cpp; $(MAKE) clean
	cd tools; $(MAKE) clean

install:system DIRS LIB BIN MAN TUTORIAL SOURCE

uninstall:
	-@$(LIBDIR)/atac/loguse $(LIBDIR)/atac install uninstall $(BINDIR) \
		>/dev/null 2>/dev/null
	rm -fr $(LIBDIR)/atac
	rm -f  $(BINDIR)/atac
	rm -f  $(BINDIR)/atacCC
	rm -f  $(BINDIR)/atacLD
	rm -f  $(BINDIR)/ataclib
	rm -f  $(BINDIR)/atacmin
	rm -f  $(BINDIR)/atactm
	rm -f  $(MANDIR)/man1/atac.1
	rm -f  $(MANDIR)/man1/atacCC.1
	rm -f  $(MANDIR)/man1/atacLD.1
	rm -f  $(MANDIR)/man1/ataclib.1
	rm -f  $(MANDIR)/man1/atacmin.1
	rm -f  $(MANDIR)/man1/atactm.1
	rm -f  $(MANDIR)/cat1/atac*

ATAC_I:
	@cd atac_i; $(MAKE) "CFLAGS=$(CFLAGS)"

ATACYSIS:	ATAC_I
	@cd atacysis; $(MAKE) "CFLAGS=$(CFLAGS)"

ATAC_CPP:
	@cd atac_cpp; $(MAKE) "CFLAGS=$(CFLAGS)"

TOOLS:
	@cd tools; $(MAKE) "CFLAGS=$(CFLAGS)"

#
# create install directories as needed
#
DIRS:
	@if expr \( x$(BINDIR) != x \) \& \( x$(LIBDIR) != x \) \&	\
		 \( x$(MANDIR) != x \)  >/dev/null;			\
	then								\
		true;							\
	else								\
		echo "*****************************************" >&2;	\
		echo "*** use: make INSTALLDIR=/... install ***" >&2;	\
		echo "*** INSTALLDIR must be specified.     ***" >&2;	\
		echo "*** See README.                       ***" >&2;	\
		echo "*****************************************" >&2;	\
		false;							\
	fi
	@if expr "x$(LIBDIR)" : x/ >/dev/null;				\
	then								\
		true;							\
	else								\
		echo "*****************************************" >&2;	\
		echo "*** use: make INSTALLDIR=/... install ***" >&2;	\
		echo "*** INSTALLDIR and LIBDIR must use    ***" >&2;	\
		echo "*** absolute paths.  See README.      ***" >&2;	\
		echo "*****************************************" >&2;	\
		false;							\
	fi
	-@test -d $(BINDIR) || mkdir $(BINDIR)
	-@test -d $(LIBDIR) || mkdir $(LIBDIR)
	-@test -d $(LIBDIR)/atac || mkdir $(LIBDIR)/atac
	-@test -d $(SRCDIR) || mkdir $(SRCDIR)
	-@test -d $(TUTORIALDIR) || mkdir $(TUTORIALDIR)
	-@test -d $(MANDIR) || mkdir $(MANDIR)
	-@test -d $(MANDIR)/man1 || mkdir $(MANDIR)/man1
	-@test -d $(MANDIR)/cat1 || mkdir $(MANDIR)/cat1

#
# install lib
#  
LIB:
	@cp tools/loguse $(LIBDIR)/atac
	@chmod 755 $(LIBDIR)/atac/loguse
	-@$(LIBDIR)/atac/loguse $(LIBDIR)/atac install old $(BINDIR) \
		>/dev/null 2>/dev/null
	-@rm -f $(LIBDIR)/atac/*
	@:
	@: README
	@:
	@echo "See $(MANDIR)/man1/atac.1 for more info." >>$(LIBDIR)/atac/README
	@echo "#$(LIBDIR)/atac/README created" >&2
	@:
	@: Version
	@:
	cp Version $(LIBDIR)/atac/Version
	@:
	@: ansi_cpp
	@:
	@rm -f cpp_ansi.c cpp_ansi
	@echo '#define m(a) "a"' >cpp_ansi.c
	@chmod u+w cpp_ansi.c
	@echo "main(){return *(m(x))=='x';}" >>cpp_ansi.c
	@cc -o cpp_ansi cpp_ansi.c
	-@if ./cpp_ansi;						\
	then								\
		echo "Default ANSI cpp" >$(LIBDIR)/atac/cpp_ansi;	\
		echo "#Default ANSI cpp" >&2;				\
	else								\
		echo "#Default non-ANSI cpp" >&2;			\
	fi
	-@rm -f cpp_ansi cpp_ansi.c
	@:
	@: lib
	@:
	cp atac_i/atac_i $(LIBDIR)/atac
	cp atac_cpp/atac_cpp $(LIBDIR)/atac
	cp atacysis/atacysis $(LIBDIR)/atac
	cp atacysis/atac_to_bin $(LIBDIR)/atac
	cp tools/minimize $(LIBDIR)/atac
	cp tools/hili $(LIBDIR)/atac
	cp tools/predefs.c $(LIBDIR)/atac
	cp tools/atac_rt.o $(LIBDIR)/atac
	cp tools/loguse $(LIBDIR)/atac
	@touch $(LIBDIR)/atac/log
	$(CC) -E tools/predefs.c | sed -n 's/^XxX//p' >$(LIBDIR)/atac/predefs
	@chmod 644 $(LIBDIR)/atac/*
	@chmod 666 $(LIBDIR)/atac/log
	@chmod 755 $(SRCDIR)
	@chmod 755 $(TUTORIALDIR)
	@chmod 755 $(LIBDIR)/atac/atac_i
	@chmod 755 $(LIBDIR)/atac/atac_cpp
	@chmod 755 $(LIBDIR)/atac/atacysis
	@chmod 755 $(LIBDIR)/atac/atac_to_bin
	@chmod 755 $(LIBDIR)/atac/minimize
	@chmod 755 $(LIBDIR)/atac/hili
	@chmod 755 $(LIBDIR)/atac/loguse
	-@$(LIBDIR)/atac/loguse $(LIBDIR)/atac install new $(BINDIR)	\
		>/dev/null 2>/dev/null
#
# install bin
#  
BIN:
	@rm -f $(BINDIR)/atac
	@rm -f $(BINDIR)/atacCC
	@rm -f $(BINDIR)/atacLD
	@rm -f $(BINDIR)/ataclib
	@rm -f $(BINDIR)/atacmin
	@rm -f $(BINDIR)/atactm
	@:
	@: atac
	@:
	@sed '/^EXPAND/,$$d' tools/atac >$(BINDIR)/atac
	@chmod u+w $(BINDIR)/atac
	-@if newform </dev/null;					\
	then								\
		echo "EXPAND='newform -i'" >>$(BINDIR)/atac;		\
	else								\
		echo "EXPAND='expand'" >>$(BINDIR)/atac;		\
	fi 2>/dev/null >/dev/null
	-@if more -s </dev/null;					\
	then								\
		echo "MORE='more -s'" >>$(BINDIR)/atac;			\
	else								\
		echo "MORE='pg -f'" >>$(BINDIR)/atac;			\
	fi
	@sed '1,/^MORE/d' tools/atac >>$(BINDIR)/atac
	@echo "#$(BINDIR)/atac created" >&2
	@:
	@: ataclib
	@:	
	@echo "#!/bin/sh" >$(BINDIR)/ataclib
	@chmod u+w $(BINDIR)/ataclib
	echo "echo $(LIBDIR)/atac" >>$(BINDIR)/ataclib
	@:
	@: bin
	@:	
	cp tools/atacCC $(BINDIR)
	cp tools/atacLD $(BINDIR)
	cp tools/atacmin $(BINDIR)
	cp atacysis/atactm $(BINDIR)
	@chmod 755 $(BINDIR)/atac
	@chmod 755 $(BINDIR)/atacCC
	@chmod 755 $(BINDIR)/atacLD
	@chmod 755 $(BINDIR)/ataclib
	@chmod 755 $(BINDIR)/atacmin
	@chmod 755 $(BINDIR)/atactm

#
# install man
#  
MAN:
	-@chmod 755 $(MANDIR)/man1 2>/dev/null
	-@chmod 755 $(MANDIR)/cat1 2>/dev/null
	rm -f $(MANDIR)/man1/atac.1
	rm -f $(MANDIR)/man1/atacCC.1
	rm -f $(MANDIR)/man1/atacLD.1
	rm -f $(MANDIR)/man1/ataclib.1
	rm -f $(MANDIR)/man1/atacmin.1
	rm -f $(MANDIR)/man1/atactm.1
	cp atac*.1 $(MANDIR)/man1
	@chmod 644 $(MANDIR)/man1/atac.1
	@chmod 644 $(MANDIR)/man1/atacCC.1
	@chmod 644 $(MANDIR)/man1/atacLD.1
	@chmod 644 $(MANDIR)/man1/ataclib.1
	@chmod 644 $(MANDIR)/man1/atacmin.1
	@chmod 644 $(MANDIR)/man1/atactm.1
	rm -f $(MANDIR)/cat1/atac*

#
# install tutorial
#  
TUTORIAL:
	@chmod 755 $(TUTORIALDIR)
	rm -f $(TUTORIALDIR)/*
	cp tutorial/Makefile $(TUTORIALDIR)
	cp tutorial/input1 $(TUTORIALDIR)
	cp tutorial/input2 $(TUTORIALDIR)
	cp tutorial/input3 $(TUTORIALDIR)
	cp tutorial/main.c $(TUTORIALDIR)
	cp tutorial/wc.c $(TUTORIALDIR)
	@chmod 644 $(TUTORIALDIR)/*

#
# install source
#  
SOURCE:
	@chmod 755 $(SRCDIR)
	rm -f $(SRCDIR)/*
	@cp atac_cpp/cccp.c $(SRCDIR)
	@cp atac_cpp/cexp.y $(SRCDIR)
	@cp atac_cpp/config.h $(SRCDIR)
	@cp atac_cpp/obstack.c $(SRCDIR)
	@cp atac_cpp/obstack.h $(SRCDIR)
	@cp atac_cpp/tm.h $(SRCDIR)
	@cp atac_cpp/version.c $(SRCDIR)
	@chmod 644 $(SRCDIR)/*
