/* $Id: error.h,v 3.3 1997/11/03 19:03:00 tom Exp $ */

#ifndef error_H
#define error_H

#include "srcpos.h"

#ifndef GCC_NORETURN
#ifdef __GNUC__
#define GCC_NORETURN __attribute__((noreturn))
#else
#define GCC_NORETURN /*nothing*/
#endif
#endif

/* error.c */
#define ERR_ARGS SRCPOS *srcpos, char *msg, ...
extern int internal_error P_(( ERR_ARGS )) GCC_NORETURN;
extern void lexical_error P_(( ERR_ARGS ));
extern void parse_error P_(( ERR_ARGS )) GCC_NORETURN;
extern void semantic_error P_(( ERR_ARGS ));
extern void supress_warnings P_(( void ));
#undef ERR_ARGS

#endif /* error_H */
