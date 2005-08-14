dnl Macros for auto-configure script.
dnl ---------------------------------------------------------------------------
dnl ---------------------------------------------------------------------------
dnl CF_ADD_CFLAGS version: 7 updated: 2004/04/25 17:48:30
dnl -------------
dnl Copy non-preprocessor flags to $CFLAGS, preprocessor flags to $CPPFLAGS
dnl The second parameter if given makes this macro verbose.
dnl
dnl Put any preprocessor definitions that use quoted strings in $EXTRA_CPPFLAGS,
dnl to simplify use of $CPPFLAGS in compiler checks, etc., that are easily
dnl confused by the quotes (which require backslashes to keep them usable).
AC_DEFUN([CF_ADD_CFLAGS],
[
cf_fix_cppflags=no
cf_new_cflags=
cf_new_cppflags=
cf_new_extra_cppflags=

for cf_add_cflags in $1
do
case $cf_fix_cppflags in
no)
	case $cf_add_cflags in #(vi
	-undef|-nostdinc*|-I*|-D*|-U*|-E|-P|-C) #(vi
		case $cf_add_cflags in
		-D*)
			cf_tst_cflags=`echo ${cf_add_cflags} |sed -e 's/^-D[[^=]]*='\''\"[[^"]]*//'`

			test "${cf_add_cflags}" != "${cf_tst_cflags}" \
			&& test -z "${cf_tst_cflags}" \
			&& cf_fix_cppflags=yes

			if test $cf_fix_cppflags = yes ; then
				cf_new_extra_cppflags="$cf_new_extra_cppflags $cf_add_cflags"
				continue
			elif test "${cf_tst_cflags}" = "\"'" ; then
				cf_new_extra_cppflags="$cf_new_extra_cppflags $cf_add_cflags"
				continue
			fi
			;;
		esac
		case "$CPPFLAGS" in
		*$cf_add_cflags) #(vi
			;;
		*) #(vi
			cf_new_cppflags="$cf_new_cppflags $cf_add_cflags"
			;;
		esac
		;;
	*)
		cf_new_cflags="$cf_new_cflags $cf_add_cflags"
		;;
	esac
	;;
yes)
	cf_new_extra_cppflags="$cf_new_extra_cppflags $cf_add_cflags"

	cf_tst_cflags=`echo ${cf_add_cflags} |sed -e 's/^[[^"]]*"'\''//'`

	test "${cf_add_cflags}" != "${cf_tst_cflags}" \
	&& test -z "${cf_tst_cflags}" \
	&& cf_fix_cppflags=no
	;;
esac
done

if test -n "$cf_new_cflags" ; then
	ifelse($2,,,[CF_VERBOSE(add to \$CFLAGS $cf_new_cflags)])
	CFLAGS="$CFLAGS $cf_new_cflags"
fi

if test -n "$cf_new_cppflags" ; then
	ifelse($2,,,[CF_VERBOSE(add to \$CPPFLAGS $cf_new_cppflags)])
	CPPFLAGS="$cf_new_cppflags $CPPFLAGS"
fi

if test -n "$cf_new_extra_cppflags" ; then
	ifelse($2,,,[CF_VERBOSE(add to \$EXTRA_CPPFLAGS $cf_new_extra_cppflags)])
	EXTRA_CPPFLAGS="$cf_new_extra_cppflags $EXTRA_CPPFLAGS"
fi

AC_SUBST(EXTRA_CPPFLAGS)

])dnl
dnl ---------------------------------------------------------------------------
dnl CF_ADD_INCDIR version: 4 updated: 2002/12/21 14:25:52
dnl -------------
dnl Add an include-directory to $CPPFLAGS.  Don't add /usr/include, since it's
dnl redundant.  We don't normally need to add -I/usr/local/include for gcc,
dnl but old versions (and some misinstalled ones) need that.  To make things
dnl worse, gcc 3.x gives error messages if -I/usr/local/include is added to
dnl the include-path).
AC_DEFUN([CF_ADD_INCDIR],
[
for cf_add_incdir in $1
do
	while true
	do
		case $cf_add_incdir in
		/usr/include) # (vi
			;;
		/usr/local/include) # (vi
			if test "$GCC" = yes
			then
				cf_save_CPPFLAGS="$CPPFLAGS"
				CPPFLAGS="$CPPFLAGS -I$cf_add_incdir"
				AC_TRY_COMPILE([#include <stdio.h>],
						[printf("Hello")],
						[],
						[CPPFLAGS="$cf_save_CPPFLAGS"])
			fi
			;;
		*) # (vi
			CPPFLAGS="$CPPFLAGS -I$cf_add_incdir"
			;;
		esac
		cf_top_incdir=`echo $cf_add_incdir | sed -e 's%/include/.*$%/include%'`
		test "$cf_top_incdir" = "$cf_add_incdir" && break
		cf_add_incdir="$cf_top_incdir"
	done
done
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_ANSI_CC_CHECK version: 9 updated: 2001/12/30 17:53:34
dnl ----------------
dnl This is adapted from the macros 'fp_PROG_CC_STDC' and 'fp_C_PROTOTYPES'
dnl in the sharutils 4.2 distribution.
AC_DEFUN([CF_ANSI_CC_CHECK],
[
AC_CACHE_CHECK(for ${CC-cc} option to accept ANSI C, cf_cv_ansi_cc,[
cf_cv_ansi_cc=no
cf_save_CFLAGS="$CFLAGS"
cf_save_CPPFLAGS="$CPPFLAGS"
# Don't try gcc -ansi; that turns off useful extensions and
# breaks some systems' header files.
# AIX			-qlanglvl=ansi
# Ultrix and OSF/1	-std1
# HP-UX			-Aa -D_HPUX_SOURCE
# SVR4			-Xc
# UnixWare 1.2		(cannot use -Xc, since ANSI/POSIX clashes)
for cf_arg in "-DCC_HAS_PROTOS" \
	"" \
	-qlanglvl=ansi \
	-std1 \
	-Ae \
	"-Aa -D_HPUX_SOURCE" \
	-Xc
do
	CF_ADD_CFLAGS($cf_arg)
	AC_TRY_COMPILE(
[
#ifndef CC_HAS_PROTOS
#if !defined(__STDC__) || (__STDC__ != 1)
choke me
#endif
#endif
],[
	int test (int i, double x);
	struct s1 {int (*f) (int a);};
	struct s2 {int (*f) (double a);};],
	[cf_cv_ansi_cc="$cf_arg"; break])
done
CFLAGS="$cf_save_CFLAGS"
CPPFLAGS="$cf_save_CPPFLAGS"
])

if test "$cf_cv_ansi_cc" != "no"; then
if test ".$cf_cv_ansi_cc" != ".-DCC_HAS_PROTOS"; then
	CF_ADD_CFLAGS($cf_cv_ansi_cc)
else
	AC_DEFINE(CC_HAS_PROTOS)
fi
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_ARG_DISABLE version: 3 updated: 1999/03/30 17:24:31
dnl --------------
dnl Allow user to disable a normally-on option.
AC_DEFUN([CF_ARG_DISABLE],
[CF_ARG_OPTION($1,[$2],[$3],[$4],yes)])dnl
dnl ---------------------------------------------------------------------------
dnl CF_ARG_OPTION version: 3 updated: 1997/10/18 14:42:41
dnl -------------
dnl Restricted form of AC_ARG_ENABLE that ensures user doesn't give bogus
dnl values.
dnl
dnl Parameters:
dnl $1 = option name
dnl $2 = help-string
dnl $3 = action to perform if option is not default
dnl $4 = action if perform if option is default
dnl $5 = default option value (either 'yes' or 'no')
AC_DEFUN([CF_ARG_OPTION],
[AC_ARG_ENABLE($1,[$2],[test "$enableval" != ifelse($5,no,yes,no) && enableval=ifelse($5,no,no,yes)
  if test "$enableval" != "$5" ; then
ifelse($3,,[    :]dnl
,[    $3]) ifelse($4,,,[
  else
    $4])
  fi],[enableval=$5 ifelse($4,,,[
  $4
])dnl
  ])])dnl
dnl ---------------------------------------------------------------------------
dnl CF_CHECK_CACHE version: 10 updated: 2004/05/23 13:03:31
dnl --------------
dnl Check if we're accidentally using a cache from a different machine.
dnl Derive the system name, as a check for reusing the autoconf cache.
dnl
dnl If we've packaged config.guess and config.sub, run that (since it does a
dnl better job than uname).  Normally we'll use AC_CANONICAL_HOST, but allow
dnl an extra parameter that we may override, e.g., for AC_CANONICAL_SYSTEM
dnl which is useful in cross-compiles.
dnl
dnl Note: we would use $ac_config_sub, but that is one of the places where
dnl autoconf 2.5x broke compatibility with autoconf 2.13
AC_DEFUN([CF_CHECK_CACHE],
[
if test -f $srcdir/config.guess || test -f $ac_aux_dir/config.guess ; then
	ifelse([$1],,[AC_CANONICAL_HOST],[$1])
	system_name="$host_os"
else
	system_name="`(uname -s -r) 2>/dev/null`"
	if test -z "$system_name" ; then
		system_name="`(hostname) 2>/dev/null`"
	fi
fi
test -n "$system_name" && AC_DEFINE_UNQUOTED(SYSTEM_NAME,"$system_name")
AC_CACHE_VAL(cf_cv_system_name,[cf_cv_system_name="$system_name"])

test -z "$system_name" && system_name="$cf_cv_system_name"
test -n "$cf_cv_system_name" && AC_MSG_RESULT(Configuring for $cf_cv_system_name)

if test ".$system_name" != ".$cf_cv_system_name" ; then
	AC_MSG_RESULT(Cached system name ($system_name) does not agree with actual ($cf_cv_system_name))
	AC_ERROR("Please remove config.cache and try again.")
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_CURSES_TERMCAP version: 10 updated: 2002/01/13 16:02:19
dnl -----------------
dnl Check if we should include <curses.h> to pick up prototypes for termcap
dnl functions.  On terminfo systems, these are normally declared in <curses.h>,
dnl but may be in <term.h>.  We check for termcap.h as an alternate, but it
dnl isn't standard (usually associated with GNU termcap).
dnl
dnl The 'tgoto()' function is declared in both terminfo and termcap.
dnl
dnl See CF_TYPE_OUTCHAR for more details.
AC_DEFUN([CF_CURSES_TERMCAP],
[
AC_REQUIRE([CF_CURSES_TERM_H])
AC_CACHE_CHECK(if we should include curses.h or termcap.h, cf_cv_need_curses_h,[
cf_save_CPPFLAGS="$CPPFLAGS"
cf_cv_need_curses_h=no

for cf_t_opts in "" "NEED_TERMCAP_H"
do
for cf_c_opts in "" "NEED_CURSES_H"
do

    CPPFLAGS="$cf_save_CPPFLAGS $CHECK_DECL_FLAG"
    test -n "$cf_c_opts" && CPPFLAGS="$CPPFLAGS -D$cf_c_opts"
    test -n "$cf_t_opts" && CPPFLAGS="$CPPFLAGS -D$cf_t_opts"

    AC_TRY_LINK([/* $cf_c_opts $cf_t_opts */
$CHECK_DECL_HDRS],
	[char *x = (char *)tgoto("")],
	[test "$cf_cv_need_curses_h" = no && {
	     cf_cv_need_curses_h=maybe
	     cf_ok_c_opts=$cf_c_opts
	     cf_ok_t_opts=$cf_t_opts
	}],
	[echo "Recompiling with corrected call (C:$cf_c_opts, T:$cf_t_opts)" >&AC_FD_CC
	AC_TRY_LINK([
$CHECK_DECL_HDRS],
	[char *x = (char *)tgoto("",0,0)],
	[cf_cv_need_curses_h=yes
	 cf_ok_c_opts=$cf_c_opts
	 cf_ok_t_opts=$cf_t_opts])])

	CPPFLAGS="$cf_save_CPPFLAGS"
	test "$cf_cv_need_curses_h" = yes && break
done
	test "$cf_cv_need_curses_h" = yes && break
done

if test "$cf_cv_need_curses_h" != no ; then
	echo "Curses/termcap test = $cf_cv_need_curses_h (C:$cf_ok_c_opts, T:$cf_ok_t_opts)" >&AC_FD_CC
	if test -n "$cf_ok_c_opts" ; then
		if test -n "$cf_ok_t_opts" ; then
			cf_cv_need_curses_h=both
		else
			cf_cv_need_curses_h=curses.h
		fi
	elif test -n "$cf_ok_t_opts" ; then
		cf_cv_need_curses_h=termcap.h
	elif test "$cf_cv_term_header" != no ; then
		cf_cv_need_curses_h=term.h
	else
		cf_cv_need_curses_h=no
	fi
fi
])

case $cf_cv_need_curses_h in
both) #(vi
	AC_DEFINE_UNQUOTED(NEED_CURSES_H)
	AC_DEFINE_UNQUOTED(NEED_TERMCAP_H)
	;;
curses.h) #(vi
	AC_DEFINE_UNQUOTED(NEED_CURSES_H)
	;;
term.h) #(vi
	AC_DEFINE_UNQUOTED(NEED_TERM_H)
	;;
termcap.h) #(vi
	AC_DEFINE_UNQUOTED(NEED_TERMCAP_H)
	;;
esac

])dnl
dnl ---------------------------------------------------------------------------
dnl CF_CURSES_TERM_H version: 6 updated: 2003/11/06 19:59:57
dnl ----------------
dnl SVr4 curses should have term.h as well (where it puts the definitions of
dnl the low-level interface).  This may not be true in old/broken implementations,
dnl as well as in misconfigured systems (e.g., gcc configured for Solaris 2.4
dnl running with Solaris 2.5.1).
AC_DEFUN([CF_CURSES_TERM_H],
[
AC_CACHE_CHECK(for term.h, cf_cv_term_header,[

AC_REQUIRE([CF_CURSES_CPPFLAGS])dnl
# If we found <ncurses/curses.h>, look for <ncurses/term.h>, but always look
# for <term.h> if we do not find the variant.
for cf_header in \
	`echo ${cf_cv_ncurses_header-curses.h} | sed -e 's%/.*%/%'`term.h \
	term.h
do
	AC_TRY_COMPILE([
#include <${cf_cv_ncurses_header-curses.h}>
#include <${cf_header}>],
	[WINDOW *x],
	[cf_cv_term_header=$cf_header
	 break],
	[cf_cv_term_header=no])
done
])

case $cf_cv_term_header in #(vi
term.h) #(vi
	AC_DEFINE(HAVE_TERM_H)
	;;
ncurses/term.h)
	AC_DEFINE(HAVE_NCURSES_TERM_H)
	;;
ncursesw/term.h)
	AC_DEFINE(HAVE_NCURSESW_TERM_H)
	;;
esac
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_DIRNAME version: 4 updated: 2002/12/21 19:25:52
dnl ----------
dnl "dirname" is not portable, so we fake it with a shell script.
AC_DEFUN([CF_DIRNAME],[$1=`echo $2 | sed -e 's%/[[^/]]*$%%'`])dnl
dnl ---------------------------------------------------------------------------
dnl CF_DISABLE_ECHO version: 10 updated: 2003/04/17 22:27:11
dnl ---------------
dnl You can always use "make -n" to see the actual options, but it's hard to
dnl pick out/analyze warning messages when the compile-line is long.
dnl
dnl Sets:
dnl	ECHO_LT - symbol to control if libtool is verbose
dnl	ECHO_LD - symbol to prefix "cc -o" lines
dnl	RULE_CC - symbol to put before implicit "cc -c" lines (e.g., .c.o)
dnl	SHOW_CC - symbol to put before explicit "cc -c" lines
dnl	ECHO_CC - symbol to put before any "cc" line
dnl
AC_DEFUN([CF_DISABLE_ECHO],[
AC_MSG_CHECKING(if you want to see long compiling messages)
CF_ARG_DISABLE(echo,
	[  --disable-echo          display "compiling" commands],
	[
    ECHO_LT='--silent'
    ECHO_LD='@echo linking [$]@;'
    RULE_CC='	@echo compiling [$]<'
    SHOW_CC='	@echo compiling [$]@'
    ECHO_CC='@'
],[
    ECHO_LT=''
    ECHO_LD=''
    RULE_CC='# compiling'
    SHOW_CC='# compiling'
    ECHO_CC=''
])
AC_MSG_RESULT($enableval)
AC_SUBST(ECHO_LT)
AC_SUBST(ECHO_LD)
AC_SUBST(RULE_CC)
AC_SUBST(SHOW_CC)
AC_SUBST(ECHO_CC)
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_FIND_LIBRARY version: 8 updated: 2004/11/23 20:14:58
dnl ---------------
dnl Look for a non-standard library, given parameters for AC_TRY_LINK.  We
dnl prefer a standard location, and use -L options only if we do not find the
dnl library in the standard library location(s).
dnl	$1 = library name
dnl	$2 = library class, usually the same as library name
dnl	$3 = includes
dnl	$4 = code fragment to compile/link
dnl	$5 = corresponding function-name
dnl	$6 = flag, nonnull if failure should not cause an error-exit
dnl
dnl Sets the variable "$cf_libdir" as a side-effect, so we can see if we had
dnl to use a -L option.
AC_DEFUN([CF_FIND_LIBRARY],
[
	eval 'cf_cv_have_lib_'$1'=no'
	cf_libdir=""
	AC_CHECK_FUNC($5,
		eval 'cf_cv_have_lib_'$1'=yes',[
		cf_save_LIBS="$LIBS"
		AC_MSG_CHECKING(for $5 in -l$1)
		LIBS="-l$1 $LIBS"
		AC_TRY_LINK([$3],[$4],
			[AC_MSG_RESULT(yes)
			 eval 'cf_cv_have_lib_'$1'=yes'
			],
			[AC_MSG_RESULT(no)
			CF_LIBRARY_PATH(cf_search,$2)
			for cf_libdir in $cf_search
			do
				AC_MSG_CHECKING(for -l$1 in $cf_libdir)
				LIBS="-L$cf_libdir -l$1 $cf_save_LIBS"
				AC_TRY_LINK([$3],[$4],
					[AC_MSG_RESULT(yes)
			 		 eval 'cf_cv_have_lib_'$1'=yes'
					 break],
					[AC_MSG_RESULT(no)
					 LIBS="$cf_save_LIBS"])
			done
			])
		])
eval 'cf_found_library=[$]cf_cv_have_lib_'$1
ifelse($6,,[
if test $cf_found_library = no ; then
	AC_ERROR(Cannot link $1 library)
fi
])
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_GCC_VERSION version: 3 updated: 2003/09/06 19:16:57
dnl --------------
dnl Find version of gcc
AC_DEFUN([CF_GCC_VERSION],[
AC_REQUIRE([AC_PROG_CC])
GCC_VERSION=none
if test "$GCC" = yes ; then
	AC_MSG_CHECKING(version of $CC)
	GCC_VERSION="`${CC} --version|sed -e '2,$d' -e 's/^[[^0-9.]]*//' -e 's/[[^0-9.]].*//'`"
	test -z "$GCC_VERSION" && GCC_VERSION=unknown
	AC_MSG_RESULT($GCC_VERSION)
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_GCC_WARNINGS version: 20 updated: 2005/08/06 18:37:29
dnl ---------------
dnl Check if the compiler supports useful warning options.  There's a few that
dnl we don't use, simply because they're too noisy:
dnl
dnl	-Wconversion (useful in older versions of gcc, but not in gcc 2.7.x)
dnl	-Wredundant-decls (system headers make this too noisy)
dnl	-Wtraditional (combines too many unrelated messages, only a few useful)
dnl	-Wwrite-strings (too noisy, but should review occasionally).  This
dnl		is enabled for ncurses using "--enable-const".
dnl	-pedantic
dnl
dnl Parameter:
dnl	$1 is an optional list of gcc warning flags that a particular
dnl		application might want to use, e.g., "no-unused" for
dnl		-Wno-unused
dnl Special:
dnl	If $with_ext_const is "yes", add a check for -Wwrite-strings
dnl
AC_DEFUN([CF_GCC_WARNINGS],
[
AC_REQUIRE([CF_GCC_VERSION])
CF_INTEL_COMPILER(GCC,INTEL_COMPILER,CFLAGS)

cat > conftest.$ac_ext <<EOF
#line __oline__ "configure"
int main(int argc, char *argv[[]]) { return (argv[[argc-1]] == 0) ; }
EOF

if test "$INTEL_COMPILER" = yes
then
# The "-wdXXX" options suppress warnings:
# remark #1419: external declaration in primary source file
# remark #1682: implicit conversion of a 64-bit integral type to a smaller integral type (potential portability problem)
# remark #1683: explicit conversion of a 64-bit integral type to a smaller integral type (potential portability problem)
# remark #1684: conversion from pointer to same-sized integral type (potential portability problem)
# remark #193: zero used for undefined preprocessing identifier
# remark #593: variable "curs_sb_left_arrow" was set but never used
# remark #810: conversion from "int" to "Dimension={unsigned short}" may lose significant bits
# remark #869: parameter "tw" was never referenced
# remark #981: operands are evaluated in unspecified order
# warning #269: invalid format string conversion

	AC_CHECKING([for $CC warning options])
	cf_save_CFLAGS="$CFLAGS"
	EXTRA_CFLAGS="-Wall"
	for cf_opt in $1 \
		wd1419 \
		wd1682 \
		wd1683 \
		wd1684 \
		wd193 \
		wd279 \
		wd593 \
		wd810 \
		wd869 \
		wd981
	do
		CFLAGS="$cf_save_CFLAGS $EXTRA_CFLAGS -$cf_opt"
		if AC_TRY_EVAL(ac_compile); then
			test -n "$verbose" && AC_MSG_RESULT(... -$cf_opt)
			EXTRA_CFLAGS="$EXTRA_CFLAGS -$cf_opt"
		fi
	done
	CFLAGS="$cf_save_CFLAGS"

elif test "$GCC" = yes
then
	AC_CHECKING([for $CC warning options])
	cf_save_CFLAGS="$CFLAGS"
	EXTRA_CFLAGS="-W -Wall"
	cf_warn_CONST=""
	test "$with_ext_const" = yes && cf_warn_CONST="Wwrite-strings"
	for cf_opt in \
		Wbad-function-cast \
		Wcast-align \
		Wcast-qual \
		Winline \
		Wmissing-declarations \
		Wmissing-prototypes \
		Wnested-externs \
		Wpointer-arith \
		Wshadow \
		Wstrict-prototypes \
		Wundef $cf_warn_CONST $1
	do
		CFLAGS="$cf_save_CFLAGS $EXTRA_CFLAGS -$cf_opt"
		if AC_TRY_EVAL(ac_compile); then
			test -n "$verbose" && AC_MSG_RESULT(... -$cf_opt)
			case $cf_opt in #(vi
			Wcast-qual) #(vi
				CPPFLAGS="$CPPFLAGS -DXTSTRINGDEFINES"
				;;
			Winline) #(vi
				case $GCC_VERSION in
				3.3*)
					CF_VERBOSE(feature is broken in gcc $GCC_VERSION)
					continue;;
				esac
				;;
			esac
			EXTRA_CFLAGS="$EXTRA_CFLAGS -$cf_opt"
		fi
	done
	CFLAGS="$cf_save_CFLAGS"
fi
rm -f conftest*

AC_SUBST(EXTRA_CFLAGS)
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_HEADER_PATH version: 8 updated: 2002/11/10 14:46:59
dnl --------------
dnl Construct a search-list for a nonstandard header-file
AC_DEFUN([CF_HEADER_PATH],
[CF_SUBDIR_PATH($1,$2,include)
test "$includedir" != NONE && \
test "$includedir" != "/usr/include" && \
test -d "$includedir" && {
	test -d $includedir &&    $1="[$]$1 $includedir"
	test -d $includedir/$2 && $1="[$]$1 $includedir/$2"
}

test "$oldincludedir" != NONE && \
test "$oldincludedir" != "/usr/include" && \
test -d "$oldincludedir" && {
	test -d $oldincludedir    && $1="[$]$1 $oldincludedir"
	test -d $oldincludedir/$2 && $1="[$]$1 $oldincludedir/$2"
}

])dnl
dnl ---------------------------------------------------------------------------
dnl CF_INTEL_COMPILER version: 3 updated: 2005/08/06 18:37:29
dnl -----------------
dnl Check if the given compiler is really the Intel compiler for Linux.  It
dnl tries to imitate gcc, but does not return an error when it finds a mismatch
dnl between prototypes, e.g., as exercised by CF_MISSING_CHECK.
dnl
dnl This macro should be run "soon" after AC_PROG_CC or AC_PROG_CPLUSPLUS, to
dnl ensure that it is not mistaken for gcc/g++.  It is normally invoked from
dnl the wrappers for gcc and g++ warnings.
dnl
dnl $1 = GCC (default) or GXX
dnl $2 = INTEL_COMPILER (default) or INTEL_CPLUSPLUS
dnl $3 = CFLAGS (default) or CXXFLAGS
AC_DEFUN([CF_INTEL_COMPILER],[
ifelse($2,,INTEL_COMPILER,[$2])=no

if test "$ifelse($1,,[$1],GCC)" = yes ; then
	case $host_os in
	linux*|gnu*)
		AC_MSG_CHECKING(if this is really Intel ifelse($1,GXX,C++,C) compiler)
		cf_save_CFLAGS="$ifelse($3,,CFLAGS,[$3])"
		ifelse($3,,CFLAGS,[$3])="$ifelse($3,,CFLAGS,[$3]) -no-gcc"
		AC_TRY_COMPILE([],[
#ifdef __INTEL_COMPILER
#else
make an error
#endif
],[ifelse($2,,INTEL_COMPILER,[$2])=yes
cf_save_CFLAGS="$cf_save_CFLAGS -we147 -no-gcc"
],[])
		ifelse($3,,CFLAGS,[$3])="$cf_save_CFLAGS"
		AC_MSG_RESULT($ifelse($2,,INTEL_COMPILER,[$2]))
		;;
	esac
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_LIBRARY_PATH version: 7 updated: 2002/11/10 14:46:59
dnl ---------------
dnl Construct a search-list for a nonstandard library-file
AC_DEFUN([CF_LIBRARY_PATH],
[CF_SUBDIR_PATH($1,$2,lib)])dnl
dnl ---------------------------------------------------------------------------
dnl CF_MSG_LOG version: 3 updated: 1997/09/07 14:05:52
dnl ----------
dnl Write a debug message to config.log, along with the line number in the
dnl configure script.
AC_DEFUN([CF_MSG_LOG],[
echo "(line __oline__) testing $* ..." 1>&AC_FD_CC
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_NCURSES_CC_CHECK version: 3 updated: 2003/01/12 18:59:28
dnl -------------------
dnl Check if we can compile with ncurses' header file
dnl $1 is the cache variable to set
dnl $2 is the header-file to include
dnl $3 is the root name (ncurses or ncursesw)
AC_DEFUN([CF_NCURSES_CC_CHECK],[
	AC_TRY_COMPILE([
]ifelse($3,ncursesw,[
#define _XOPEN_SOURCE_EXTENDED
#undef  HAVE_LIBUTF8_H	/* in case we used CF_UTF8_LIB */
#define HAVE_LIBUTF8_H	/* to force ncurses' header file to use cchar_t */
])[
#include <$2>],[
#ifdef NCURSES_VERSION
]ifelse($3,ncursesw,[
#ifndef WACS_BSSB
	make an error
#endif
])[
printf("%s\n", NCURSES_VERSION);
#else
#ifdef __NCURSES_H
printf("old\n");
#else
	make an error
#endif
#endif
	]
	,[$1=$cf_header]
	,[$1=no])
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_NCURSES_CPPFLAGS version: 17 updated: 2003/11/06 19:59:57
dnl -------------------
dnl Look for the SVr4 curses clone 'ncurses' in the standard places, adjusting
dnl the CPPFLAGS variable so we can include its header.
dnl
dnl The header files may be installed as either curses.h, or ncurses.h (would
dnl be obsolete, except that some packagers prefer this name to distinguish it
dnl from a "native" curses implementation).  If not installed for overwrite,
dnl the curses.h file would be in an ncurses subdirectory (e.g.,
dnl /usr/include/ncurses), but someone may have installed overwriting the
dnl vendor's curses.  Only very old versions (pre-1.9.2d, the first autoconf'd
dnl version) of ncurses don't define either __NCURSES_H or NCURSES_VERSION in
dnl the header.
dnl
dnl If the installer has set $CFLAGS or $CPPFLAGS so that the ncurses header
dnl is already in the include-path, don't even bother with this, since we cannot
dnl easily determine which file it is.  In this case, it has to be <curses.h>.
dnl
dnl The optional parameter gives the root name of the library, in case it is
dnl not installed as the default curses library.  That is how the
dnl wide-character version of ncurses is installed.
AC_DEFUN([CF_NCURSES_CPPFLAGS],
[AC_REQUIRE([CF_WITH_CURSES_DIR])

AC_PROVIDE([CF_CURSES_CPPFLAGS])dnl
cf_ncuhdr_root=ifelse($1,,ncurses,$1)

test -n "$cf_cv_curses_dir" && \
test "$cf_cv_curses_dir" != "no" && \
CPPFLAGS="-I$cf_cv_curses_dir/include -I$cf_cv_curses_dir/include/$cf_ncuhdr_root $CPPFLAGS"

AC_CACHE_CHECK(for $cf_ncuhdr_root header in include-path, cf_cv_ncurses_h,[
	cf_header_list="$cf_ncuhdr_root/curses.h $cf_ncuhdr_root/ncurses.h"
	( test "$cf_ncuhdr_root" = ncurses || test "$cf_ncuhdr_root" = ncursesw ) && cf_header_list="$cf_header_list curses.h ncurses.h"
	for cf_header in $cf_header_list
	do
		CF_NCURSES_CC_CHECK(cf_cv_ncurses_h,$cf_header,$1)
		test "$cf_cv_ncurses_h" != no && break
	done
])

if test "$cf_cv_ncurses_h" != no ; then
	cf_cv_ncurses_header=$cf_cv_ncurses_h
else
AC_CACHE_CHECK(for $cf_ncuhdr_root include-path, cf_cv_ncurses_h2,[
	test -n "$verbose" && echo
	CF_HEADER_PATH(cf_search,$cf_ncuhdr_root)
	test -n "$verbose" && echo search path $cf_search
	cf_save2_CPPFLAGS="$CPPFLAGS"
	for cf_incdir in $cf_search
	do
		CF_ADD_INCDIR($cf_incdir)
		for cf_header in \
			ncurses.h \
			curses.h
		do
			CF_NCURSES_CC_CHECK(cf_cv_ncurses_h2,$cf_header,$1)
			if test "$cf_cv_ncurses_h2" != no ; then
				cf_cv_ncurses_h2=$cf_incdir/$cf_header
				test -n "$verbose" && echo $ac_n "	... found $ac_c" 1>&AC_FD_MSG
				break
			fi
			test -n "$verbose" && echo "	... tested $cf_incdir/$cf_header" 1>&AC_FD_MSG
		done
		CPPFLAGS="$cf_save2_CPPFLAGS"
		test "$cf_cv_ncurses_h2" != no && break
	done
	test "$cf_cv_ncurses_h2" = no && AC_ERROR(not found)
	])

	CF_DIRNAME(cf_1st_incdir,$cf_cv_ncurses_h2)
	cf_cv_ncurses_header=`basename $cf_cv_ncurses_h2`
	if test `basename $cf_1st_incdir` = $cf_ncuhdr_root ; then
		cf_cv_ncurses_header=$cf_ncuhdr_root/$cf_cv_ncurses_header
	fi
	CF_ADD_INCDIR($cf_1st_incdir)

fi

AC_DEFINE(NCURSES)

case $cf_cv_ncurses_header in # (vi
*ncurses.h)
	AC_DEFINE(HAVE_NCURSES_H)
	;;
esac

case $cf_cv_ncurses_header in # (vi
ncurses/curses.h|ncurses/ncurses.h)
	AC_DEFINE(HAVE_NCURSES_NCURSES_H)
	;;
ncursesw/curses.h|ncursesw/ncurses.h)
	AC_DEFINE(HAVE_NCURSESW_NCURSES_H)
	;;
esac

CF_NCURSES_VERSION
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_NCURSES_LIBS version: 12 updated: 2004/04/27 16:26:05
dnl ---------------
dnl Look for the ncurses library.  This is a little complicated on Linux,
dnl because it may be linked with the gpm (general purpose mouse) library.
dnl Some distributions have gpm linked with (bsd) curses, which makes it
dnl unusable with ncurses.  However, we don't want to link with gpm unless
dnl ncurses has a dependency, since gpm is normally set up as a shared library,
dnl and the linker will record a dependency.
dnl
dnl The optional parameter gives the root name of the library, in case it is
dnl not installed as the default curses library.  That is how the
dnl wide-character version of ncurses is installed.
AC_DEFUN([CF_NCURSES_LIBS],
[AC_REQUIRE([CF_NCURSES_CPPFLAGS])

cf_nculib_root=ifelse($1,,ncurses,$1)
	# This works, except for the special case where we find gpm, but
	# ncurses is in a nonstandard location via $LIBS, and we really want
	# to link gpm.
cf_ncurses_LIBS=""
cf_ncurses_SAVE="$LIBS"
AC_CHECK_LIB(gpm,Gpm_Open,
	[AC_CHECK_LIB(gpm,initscr,
		[LIBS="$cf_ncurses_SAVE"],
		[cf_ncurses_LIBS="-lgpm"])])

case $host_os in #(vi
freebsd*)
	# This is only necessary if you are linking against an obsolete
	# version of ncurses (but it should do no harm, since it's static).
	if test "$cf_nculib_root" = ncurses ; then
		AC_CHECK_LIB(mytinfo,tgoto,[cf_ncurses_LIBS="-lmytinfo $cf_ncurses_LIBS"])
	fi
	;;
esac

LIBS="$cf_ncurses_LIBS $LIBS"

if ( test -n "$cf_cv_curses_dir" && test "$cf_cv_curses_dir" != "no" )
then
	LIBS="-L$cf_cv_curses_dir/lib -l$cf_nculib_root $LIBS"
else
	CF_FIND_LIBRARY($cf_nculib_root,$cf_nculib_root,
		[#include <${cf_cv_ncurses_header-curses.h}>],
		[initscr()],
		initscr)
fi

if test -n "$cf_ncurses_LIBS" ; then
	AC_MSG_CHECKING(if we can link $cf_nculib_root without $cf_ncurses_LIBS)
	cf_ncurses_SAVE="$LIBS"
	for p in $cf_ncurses_LIBS ; do
		q=`echo $LIBS | sed -e "s%$p %%" -e "s%$p$%%"`
		if test "$q" != "$LIBS" ; then
			LIBS="$q"
		fi
	done
	AC_TRY_LINK([#include <${cf_cv_ncurses_header-curses.h}>],
		[initscr(); mousemask(0,0); tgoto((char *)0, 0, 0);],
		[AC_MSG_RESULT(yes)],
		[AC_MSG_RESULT(no)
		 LIBS="$cf_ncurses_SAVE"])
fi

CF_UPPER(cf_nculib_ROOT,HAVE_LIB$cf_nculib_root)
AC_DEFINE_UNQUOTED($cf_nculib_ROOT)
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_NCURSES_VERSION version: 11 updated: 2003/11/06 19:59:57
dnl ------------------
dnl Check for the version of ncurses, to aid in reporting bugs, etc.
dnl Call CF_CURSES_CPPFLAGS first, or CF_NCURSES_CPPFLAGS.  We don't use
dnl AC_REQUIRE since that does not work with the shell's if/then/else/fi.
AC_DEFUN([CF_NCURSES_VERSION],
[
AC_REQUIRE([CF_CURSES_CPPFLAGS])dnl
AC_CACHE_CHECK(for ncurses version, cf_cv_ncurses_version,[
	cf_cv_ncurses_version=no
	cf_tempfile=out$$
	rm -f $cf_tempfile
	AC_TRY_RUN([
#include <${cf_cv_ncurses_header-curses.h}>
#include <stdio.h>
int main()
{
	FILE *fp = fopen("$cf_tempfile", "w");
#ifdef NCURSES_VERSION
# ifdef NCURSES_VERSION_PATCH
	fprintf(fp, "%s.%d\n", NCURSES_VERSION, NCURSES_VERSION_PATCH);
# else
	fprintf(fp, "%s\n", NCURSES_VERSION);
# endif
#else
# ifdef __NCURSES_H
	fprintf(fp, "old\n");
# else
	make an error
# endif
#endif
	exit(0);
}],[
	cf_cv_ncurses_version=`cat $cf_tempfile`],,[

	# This will not work if the preprocessor splits the line after the
	# Autoconf token.  The 'unproto' program does that.
	cat > conftest.$ac_ext <<EOF
#include <${cf_cv_ncurses_header-curses.h}>
#undef Autoconf
#ifdef NCURSES_VERSION
Autoconf NCURSES_VERSION
#else
#ifdef __NCURSES_H
Autoconf "old"
#endif
;
#endif
EOF
	cf_try="$ac_cpp conftest.$ac_ext 2>&AC_FD_CC | grep '^Autoconf ' >conftest.out"
	AC_TRY_EVAL(cf_try)
	if test -f conftest.out ; then
		cf_out=`cat conftest.out | sed -e 's%^Autoconf %%' -e 's%^[[^"]]*"%%' -e 's%".*%%'`
		test -n "$cf_out" && cf_cv_ncurses_version="$cf_out"
		rm -f conftest.out
	fi
])
	rm -f $cf_tempfile
])
test "$cf_cv_ncurses_version" = no || AC_DEFINE(NCURSES)
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_OUTPUT_IF_CHANGED version: 2 updated: 1997/09/07 18:53:59
dnl --------------------
dnl Within AC_OUTPUT, check if the given file differs from the target, and
dnl update it if so.  Otherwise, remove the generated file.
dnl
dnl Parameters:
dnl $1 = input, which configure has done substitutions upon
dnl $2 = target file
dnl
AC_DEFUN([CF_OUTPUT_IF_CHANGED],[
if ( cmp -s $1 $2 2>/dev/null )
then
	echo "$2 is unchanged"
	rm -f $1
else
	echo "creating $2"
	rm -f $2
	mv $1 $2
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_PATH_SYNTAX version: 9 updated: 2002/09/17 23:03:38
dnl --------------
dnl Check the argument to see that it looks like a pathname.  Rewrite it if it
dnl begins with one of the prefix/exec_prefix variables, and then again if the
dnl result begins with 'NONE'.  This is necessary to work around autoconf's
dnl delayed evaluation of those symbols.
AC_DEFUN([CF_PATH_SYNTAX],[
case ".[$]$1" in #(vi
.\[$]\(*\)*|.\'*\'*) #(vi
  ;;
..|./*|.\\*) #(vi
  ;;
.[[a-zA-Z]]:[[\\/]]*) #(vi OS/2 EMX
  ;;
.\[$]{*prefix}*) #(vi
  eval $1="[$]$1"
  case ".[$]$1" in #(vi
  .NONE/*)
    $1=`echo [$]$1 | sed -e s%NONE%$ac_default_prefix%`
    ;;
  esac
  ;; #(vi
.NONE/*)
  $1=`echo [$]$1 | sed -e s%NONE%$ac_default_prefix%`
  ;;
*)
  ifelse($2,,[AC_ERROR([expected a pathname, not \"[$]$1\"])],$2)
  ;;
esac
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_SED_CONFIG_H version: 9 updated: 1998/05/17 00:30:22
dnl ---------------
dnl This bypasses the normal autoconf process because we're generating an
dnl arbitrary number of NEED_xxxx definitions with the CF_HAVE_FUNCS macro.
dnl Rather than populate an aclocal.h file with all of those definitions, we do
dnl it here.
dnl
dnl Parameters:
dnl $1 = input, which configure has done substitutions upon (will delete)
dnl $2 = target file
dnl $3 = preamble, if any (a 'here' document)
dnl $4 = trailer, if any (a 'here' document)
dnl
AC_DEFUN([CF_SED_CONFIG_H],[
cf_config_h=conf$$
rm -f $cf_config_h
## PREAMBLE
ifelse($3,,[
echo '/* generated by configure-script */' >$cf_config_h
],[cat >$cf_config_h <<CF_EOF
$3[]dnl
CF_EOF])
## DEFINITIONS
if test -n "$ac_cv_path_TD_CONFIG" ; then
	$ac_cv_path_TD_CONFIG $1 |egrep -v '^#' >$cf_config_h
	$ac_cv_path_TD_CONFIG $1 |egrep '^#' | sort >>$cf_config_h
else
grep -v '^ -D' $1 >>$cf_config_h
changequote(,)dnl
sed	-e '/^ -D/!d' \
	-e '/^# /d' \
	-e 's/ -D/\
#define /g' \
	-e 's/\(#define [A-Za-z_][A-Za-z0-9_]*\)=/\1	/g' \
	-e 's@\\@@g' \
	$1 | sort >>$cf_config_h
changequote([,])dnl
fi
## TRAILER
ifelse($4,,,
[cat >>$cf_config_h <<CF_EOF
$4[]dnl
CF_EOF])
CF_OUTPUT_IF_CHANGED($cf_config_h,$2)
rm -f $1 $cf_config_h
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_STRIP_G_OPT version: 3 updated: 2002/12/21 19:25:52
dnl --------------
dnl	Remove "-g" option from the compiler options
AC_DEFUN([CF_STRIP_G_OPT],
[$1=`echo ${$1} | sed -e 's%-g %%' -e 's%-g$%%'`])dnl
dnl ---------------------------------------------------------------------------
dnl CF_STRIP_O_OPT version: 3 updated: 2001/02/02 19:06:08
dnl --------------
dnl	Remove "-O" option from the compiler options
AC_DEFUN([CF_STRIP_O_OPT],[
$1=`echo ${$1} | sed -e 's/-O[[1-9]]\? //' -e 's/-O[[1-9]]\?$//'`
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_SUBDIR_PATH version: 3 updated: 2002/12/29 18:30:46
dnl --------------
dnl Construct a search-list for a nonstandard header/lib-file
dnl	$1 = the variable to return as result
dnl	$2 = the package name
dnl	$3 = the subdirectory, e.g., bin, include or lib
AC_DEFUN([CF_SUBDIR_PATH],
[$1=""

test -d [$]HOME && {
	test -n "$verbose" && echo "	... testing $3-directories under [$]HOME"
	test -d [$]HOME/$3 &&          $1="[$]$1 [$]HOME/$3"
	test -d [$]HOME/$3/$2 &&       $1="[$]$1 [$]HOME/$3/$2"
	test -d [$]HOME/$3/$2/$3 &&    $1="[$]$1 [$]HOME/$3/$2/$3"
}

# For other stuff under the home directory, it should be sufficient to put
# a symbolic link for $HOME/$2 to the actual package location:
test -d [$]HOME/$2 && {
	test -n "$verbose" && echo "	... testing $3-directories under [$]HOME/$2"
	test -d [$]HOME/$2/$3 &&       $1="[$]$1 [$]HOME/$2/$3"
	test -d [$]HOME/$2/$3/$2 &&    $1="[$]$1 [$]HOME/$2/$3/$2"
}

test "$prefix" != /usr/local && \
test -d /usr/local && {
	test -n "$verbose" && echo "	... testing $3-directories under /usr/local"
	test -d /usr/local/$3 &&       $1="[$]$1 /usr/local/$3"
	test -d /usr/local/$3/$2 &&    $1="[$]$1 /usr/local/$3/$2"
	test -d /usr/local/$3/$2/$3 && $1="[$]$1 /usr/local/$3/$2/$3"
	test -d /usr/local/$2/$3 &&    $1="[$]$1 /usr/local/$2/$3"
	test -d /usr/local/$2/$3/$2 && $1="[$]$1 /usr/local/$2/$3/$2"
}

test "$prefix" != NONE && \
test -d $prefix && {
	test -n "$verbose" && echo "	... testing $3-directories under $prefix"
	test -d $prefix/$3 &&          $1="[$]$1 $prefix/$3"
	test -d $prefix/$3/$2 &&       $1="[$]$1 $prefix/$3/$2"
	test -d $prefix/$3/$2/$3 &&    $1="[$]$1 $prefix/$3/$2/$3"
	test -d $prefix/$2/$3 &&       $1="[$]$1 $prefix/$2/$3"
	test -d $prefix/$2/$3/$2 &&    $1="[$]$1 $prefix/$2/$3/$2"
}

test "$prefix" != /opt && \
test -d /opt && {
	test -n "$verbose" && echo "	... testing $3-directories under /opt"
	test -d /opt/$3 &&             $1="[$]$1 /opt/$3"
	test -d /opt/$3/$2 &&          $1="[$]$1 /opt/$3/$2"
	test -d /opt/$3/$2/$3 &&       $1="[$]$1 /opt/$3/$2/$3"
	test -d /opt/$2/$3 &&          $1="[$]$1 /opt/$2/$3"
	test -d /opt/$2/$3/$2 &&       $1="[$]$1 /opt/$2/$3/$2"
}

test "$prefix" != /usr && \
test -d /usr && {
	test -n "$verbose" && echo "	... testing $3-directories under /usr"
	test -d /usr/$3 &&             $1="[$]$1 /usr/$3"
	test -d /usr/$3/$2 &&          $1="[$]$1 /usr/$3/$2"
	test -d /usr/$3/$2/$3 &&       $1="[$]$1 /usr/$3/$2/$3"
	test -d /usr/$2/$3 &&          $1="[$]$1 /usr/$2/$3"
}
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_TERMCAP_LIBS version: 10 updated: 2001/10/18 20:42:39
dnl ---------------
dnl Look for termcap libraries, or the equivalent in terminfo.
AC_DEFUN([CF_TERMCAP_LIBS],
[
AC_CACHE_VAL(cf_cv_termlib,[
cf_cv_termlib=none
AC_TRY_LINK([],[char *x=(char*)tgoto("",0,0)],
[AC_TRY_LINK([],[int x=tigetstr("")],
	[cf_cv_termlib=terminfo],
	[cf_cv_termlib=termcap])
	CF_VERBOSE(using functions in predefined $cf_cv_termlib LIBS)
],[
ifelse([$1],,,[
case "$1" in # (vi
ncurses*)
	CF_NCURSES_CPPFLAGS($1)
	CF_NCURSES_LIBS($1)
	cf_cv_termlib=terminfo
	;;
esac
])
if test "$cf_cv_termlib" = none; then
	# FreeBSD's linker gives bogus results for AC_CHECK_LIB, saying that
	# tgetstr lives in -lcurses when it is only an unsatisfied extern.
	cf_save_LIBS="$LIBS"
	for cf_lib in curses ncurses termlib termcap
	do
	LIBS="-l$cf_lib $cf_save_LIBS"
	for cf_func in tigetstr tgetstr
	do
		AC_MSG_CHECKING(for $cf_func in -l$cf_lib)
		AC_TRY_LINK([],[int x=$cf_func("")],[cf_result=yes],[cf_result=no])
		AC_MSG_RESULT($cf_result)
		if test "$cf_result" = yes ; then
			if test "$cf_func" = tigetstr ; then
				cf_cv_termlib=terminfo
			else
				cf_cv_termlib=termcap
			fi
			break
		fi
	done
		test "$cf_result" = yes && break
	done
	test "$cf_result" = no && LIBS="$cf_save_LIBS"
fi
if test "$cf_cv_termlib" = none; then
	# allow curses library for broken AIX system.
	AC_CHECK_LIB(curses, initscr, [LIBS="$LIBS -lcurses" cf_cv_termlib=termcap])
	AC_CHECK_LIB(termcap, tgoto, [LIBS="$LIBS -ltermcap" cf_cv_termlib=termcap])
fi
])
if test "$cf_cv_termlib" = none; then
	AC_MSG_WARN([Cannot find -ltermlib, -lcurses, or -ltermcap])
fi
])])dnl
dnl ---------------------------------------------------------------------------
dnl CF_TYPE_OUTCHAR version: 13 updated: 2002/10/09 20:00:37
dnl ---------------
dnl Check for return and param type of 3rd -- OutChar() -- param of tputs().
dnl
dnl For this check, and for CF_CURSES_TERMCAP, the $CHECK_DECL_HDRS variable
dnl must point to a header file containing this (or equivalent):
dnl
dnl	#ifdef NEED_CURSES_H
dnl	# ifdef HAVE_NCURSES_NCURSES_H
dnl	#  include <ncurses/ncurses.h>
dnl	# else
dnl	#  ifdef HAVE_NCURSES_H
dnl	#   include <ncurses.h>
dnl	#  else
dnl	#   include <curses.h>
dnl	#  endif
dnl	# endif
dnl	#endif
dnl
dnl	#ifdef HAVE_NCURSES_TERM_H
dnl	#  include <ncurses/term.h>
dnl	#else
dnl	# ifdef HAVE_TERM_H
dnl	#  include <term.h>
dnl	# endif
dnl	#endif
dnl
dnl	#if NEED_TERMCAP_H
dnl	# include <termcap.h>
dnl	#endif
dnl
AC_DEFUN([CF_TYPE_OUTCHAR],
[
AC_REQUIRE([CF_CURSES_TERMCAP])

AC_CACHE_CHECK(declaration of tputs 3rd param, cf_cv_type_outchar,[

cf_cv_type_outchar="int OutChar(int)"
cf_cv_found=no
cf_save_CPPFLAGS="$CPPFLAGS"
CPPFLAGS="$CPPFLAGS $CHECK_DECL_FLAG"

for P in int void; do
for Q in int void; do
for R in int char; do
for S in "" const; do
	CF_MSG_LOG(loop variables [P:[$]P, Q:[$]Q, R:[$]R, S:[$]S])
	AC_TRY_COMPILE([$CHECK_DECL_HDRS],
	[extern $Q OutChar($R);
	extern $P tputs ($S char *string, int nlines, $Q (*_f)($R));
	tputs("", 1, OutChar)],
	[cf_cv_type_outchar="$Q OutChar($R)"
	 cf_cv_found=yes
	 break])
done
	test $cf_cv_found = yes && break
done
	test $cf_cv_found = yes && break
done
	test $cf_cv_found = yes && break
done
])

case $cf_cv_type_outchar in
int*)
	AC_DEFINE(OUTC_RETURN)
	;;
esac
case $cf_cv_type_outchar in
*char*)
	AC_DEFINE(OUTC_ARGS,char c)
	;;
esac

CPPFLAGS="$cf_save_CPPFLAGS"
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_UPPER version: 5 updated: 2001/01/29 23:40:59
dnl --------
dnl Make an uppercase version of a variable
dnl $1=uppercase($2)
AC_DEFUN([CF_UPPER],
[
$1=`echo "$2" | sed y%abcdefghijklmnopqrstuvwxyz./-%ABCDEFGHIJKLMNOPQRSTUVWXYZ___%`
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_VERBOSE version: 2 updated: 1997/09/05 10:45:14
dnl ----------
dnl Use AC_VERBOSE w/o the warnings
AC_DEFUN([CF_VERBOSE],
[test -n "$verbose" && echo "	$1" 1>&AC_FD_MSG
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_WITH_CURSES_DIR version: 2 updated: 2002/11/10 14:46:59
dnl ------------------
dnl Wrapper for AC_ARG_WITH to specify directory under which to look for curses
dnl libraries.
AC_DEFUN([CF_WITH_CURSES_DIR],[
AC_ARG_WITH(curses-dir,
	[  --with-curses-dir=DIR   directory in which (n)curses is installed],
	[CF_PATH_SYNTAX(withval)
	 cf_cv_curses_dir=$withval],
	[cf_cv_curses_dir=no])
])dnl
