/* $Id: error.h,v 3.5 2013/12/09 00:16:40 tom Exp $ */

#ifndef error_H
#define error_H

#include "srcpos.h"

#ifndef GCC_NORETURN
#ifdef __GNUC__
#define GCC_NORETURN __attribute__((noreturn))
#else
#define GCC_NORETURN		/*nothing */
#endif
#endif

/* error.c */
#define ERR_ARGS SRCPOS *srcpos, const char *msg, ...
extern int internal_error(ERR_ARGS) GCC_NORETURN;
extern void lexical_error(ERR_ARGS);
extern void parse_error(ERR_ARGS) GCC_NORETURN;
extern void semantic_error(ERR_ARGS);
extern void supress_warnings(void);
#undef ERR_ARGS

#endif /* error_H */
