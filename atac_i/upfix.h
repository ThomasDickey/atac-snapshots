/* $Id: upfix.h,v 3.2 1997/05/11 23:13:12 tom Exp $ */

#ifndef upfix_H
#define upfix_H

typedef struct {
	int		charsleft;
	unsigned long	upcase;
	unsigned long	lowcase;
	int 		uscore;
	unsigned long	digit;
	size_t		prefixlen;
	int		unique;
	int		maxlen;
	char		*prefix;
} PREFIX;

/* forward declarations */
extern char *upfix P_(( PREFIX *p ));
extern void upfix_exclude P_(( PREFIX *p, char *name ));
extern void upfix_free P_(( PREFIX *p ));
extern PREFIX *upfix_init P_(( size_t maxlen, char *s ));

#endif /* upfix_H */
