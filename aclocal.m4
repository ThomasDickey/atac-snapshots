dnl Macros for auto-configure script.
dnl ---------------------------------------------------------------------------
dnl ---------------------------------------------------------------------------
dnl This is adapted from the macros 'fp_PROG_CC_STDC' and 'fp_C_PROTOTYPES'
dnl in the sharutils 4.2 distribution.
AC_DEFUN([CF_ANSI_CC_CHECK],
[
AC_MSG_CHECKING(for ${CC-cc} option to accept ANSI C)
AC_CACHE_VAL(cf_cv_ansi_cc,[
cf_cv_ansi_cc=no
cf_save_CFLAGS="$CFLAGS"
# Don't try gcc -ansi; that turns off useful extensions and
# breaks some systems' header files.
# AIX			-qlanglvl=ansi
# Ultrix and OSF/1	-std1
# HP-UX			-Aa -D_HPUX_SOURCE
# SVR4			-Xc
# UnixWare 1.2		(cannot use -Xc, since ANSI/POSIX clashes)
for cf_arg in "-DCC_HAS_PROTOS" "" -qlanglvl=ansi -std1 "-Aa -D_HPUX_SOURCE" -Xc
do
	CFLAGS="$cf_save_CFLAGS $cf_arg"
	AC_TRY_COMPILE(
[
#ifndef CC_HAS_PROTOS
#if !defined(__STDC__) || __STDC__ != 1
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
])
AC_MSG_RESULT($cf_cv_ansi_cc)

if test "$cf_cv_ansi_cc" != "no"; then
if test ".$cf_cv_ansi_cc" != ".-DCC_HAS_PROTOS"; then
	CFLAGS="$CFLAGS $cf_cv_ansi_cc"
else
	AC_DEFINE(CC_HAS_PROTOS)
fi
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl Check if we're accidentally using a cache from a different machine.
dnl Derive the system name, as a check for reusing the autoconf cache.
dnl
dnl If we've packaged config.guess and config.sub, run that (since it does a
dnl better job than uname). 
AC_DEFUN([CF_CHECK_CACHE],
[
if test -f $srcdir/config.guess ; then
	AC_CANONICAL_HOST
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
test -n "$cf_cv_system_name" && AC_MSG_RESULT("Configuring for $cf_cv_system_name")

if test ".$system_name" != ".$cf_cv_system_name" ; then
	AC_MSG_RESULT(Cached system name ($system_name) does not agree with actual ($cf_cv_system_name))
	AC_ERROR("Please remove config.cache and try again.")
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl Check for tgetent function in termcap library.  If we cannot find this,
dnl we'll use the $LINES and $COLUMNS environment variables to pass screen
dnl size information to subprocesses.  (We cannot use terminfo's compatibility
dnl function, since it cannot provide the termcap-format data).
AC_DEFUN([CF_FUNC_TGETENT],
[
AC_MSG_CHECKING(for workable tgetent function)
AC_CACHE_VAL(cf_cv_func_tgetent,[
cf_save_LIBS="$LIBS"
cf_cv_func_tgetent=no
cf_TERMLIB="termcap termlib ncurses curses"
for cf_termlib in $cf_TERMLIB
do
	LIBS="$cf_save_LIBS -l$cf_termlib"
	AC_TRY_RUN([
/* terminfo implementations ignore the buffer argument, making it useless for
 * the xterm application, which uses this information to make a new TERMCAP
 * environment variable.
 */
int main()
{
	char buffer[1024];
	buffer[0] = 0;
	tgetent(buffer, "vt100");
	exit(buffer[0] == 0); }],
	[echo "yes, there is a termcap/tgetent present" 1>&AC_FD_CC
	 cf_cv_func_tgetent=yes
	 break],
	[echo "no, there is no termcap/tgetent present" 1>&AC_FD_CC
	 cf_cv_func_tgetent=no],
	[echo "cross-compiling, cannot verify if a termcap/tgetent is present" 1>&AC_FD_CC
	 cf_cv_func_tgetent=no])
done
# If there was no workable (termcap) version, maybe there is a terminfo version
if test $cf_cv_func_tgetent = no ; then
	for cf_termlib in $cf_TERMLIB
	do
		AC_TRY_LINK([],[tgetent(0, 0)],
			[echo "there is a terminfo/tgetent present" 1>&AC_FD_CC
			 cf_cv_func_tgetent=$cf_termlib
			 break],
			[LIBS="$cf_save_LIBS"])
	done
fi
])
AC_MSG_RESULT($cf_cv_func_tgetent)
# If we found any sort of tgetent, check for the termcap.h file.  If this is
# linking against ncurses, we'll trigger the ifdef in resize.c that turns the
# termcap stuff back off.  Including termcap.h should otherwise be harmless.
if test $cf_cv_func_tgetent != no ; then
	AC_CHECK_HEADERS(termcap.h)
	if test $cf_cv_func_tgetent != yes ; then
		AC_DEFINE(USE_TERMINFO)
	fi
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl Check if the compiler supports useful warning options.  There's a few that
dnl we don't use, simply because they're too noisy:
dnl
dnl	-Wconversion (useful in older versions of gcc, but not in gcc 2.7.x)
dnl	-Wredundant-decls (system headers make this too noisy)
dnl	-Wtraditional (combines too many unrelated messages, only a few useful)
dnl	-Wwrite-strings (too noisy, but should review occasionally)
dnl	-pedantic
dnl
AC_DEFUN([CF_GCC_WARNINGS],
[
if test -n "$GCC"
then
	changequote(,)dnl
	cat > conftest.$ac_ext <<EOF
#line __oline__ "configure"
int main(int argc, char *argv[]) { return argv[argc-1] == 0; }
EOF
	changequote([,])dnl
	AC_CHECKING([for gcc warning options])
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
		Wstrict-prototypes $cf_warn_CONST
	do
		CFLAGS="$cf_save_CFLAGS $EXTRA_CFLAGS -$cf_opt"
		if AC_TRY_EVAL(ac_compile); then
			test -n "$verbose" && AC_MSG_RESULT(... -$cf_opt)
			EXTRA_CFLAGS="$EXTRA_CFLAGS -$cf_opt"
			test "$cf_opt" = Wcast-qual && EXTRA_CFLAGS="$EXTRA_CFLAGS -DXTSTRINGDEFINES"
		fi
	done
	rm -f conftest*
	CFLAGS="$cf_save_CFLAGS"
fi
AC_SUBST(EXTRA_CFLAGS)
])dnl
dnl ---------------------------------------------------------------------------
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
dnl	Remove "-g" option from the compiler options
AC_DEFUN([CF_STRIP_G_OPT],
[$1=`echo ${$1} | sed -e 's/-g //' -e 's/-g$//'`])dnl
dnl ---------------------------------------------------------------------------
dnl	Remove "-O" option from the compiler options
AC_DEFUN([CF_STRIP_O_OPT],[
changequote(,)dnl
$1=`echo ${$1} | sed -e 's/-O[1-9]\? //' -e 's/-O[1-9]\?$//'`
changequote([,])dnl
])dnl
dnl ---------------------------------------------------------------------------
dnl Check for return and param type of 3rd -- OutChar() -- param of tputs().
AC_DEFUN([CF_TYPE_OUTCHAR],
[AC_MSG_CHECKING([declaration of tputs 3rd param])
AC_CACHE_VAL(cf_cv_type_outchar,[
cf_cv_type_outchar="int OutChar(int)"
cf_cv_found=no
for P in int void; do
for Q in int void; do
for R in int char; do
for S in "" const; do
	AC_TRY_COMPILE([
#ifdef USE_TERMINFO
#include <curses.h>
#if HAVE_TERM_H
#include <term.h>
#endif
#else
#if HAVE_CURSES_H
#include <curses.h>	/* FIXME: this should be included only for terminfo */
#endif
#if HAVE_TERMCAP_H
#include <termcap.h>
#endif
#endif ],
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
AC_MSG_RESULT($cf_cv_type_outchar)
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
])dnl
