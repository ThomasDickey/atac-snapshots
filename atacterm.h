/*
 * Used by tools/hili.c to get the proper prototype for 'tputs()'.
 *
 * $Id: atacterm.h,v 3.1 1998/08/23 02:40:49 tom Exp $
 */

#ifndef atacterm_H
#define atacterm_H 1

#ifdef NEED_CURSES_H
# if HAVE_NCURSES_H
#  include <ncurses.h>
# else
#  include <curses.h>
# endif
#endif
#if HAVE_TERM_H
# include <term.h>
#endif
#if NEED_TERMCAP_H
# include <termcap.h>
#endif

#endif /* atacterm_H */
