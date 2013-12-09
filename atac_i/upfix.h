/* $Id: upfix.h,v 3.4 2013/12/08 22:05:41 tom Exp $ */

#ifndef upfix_H
#define upfix_H

typedef struct {
    int charsleft;
    unsigned long upcase;
    unsigned long lowcase;
    int uscore;
    unsigned long digit;
    size_t prefixlen;
    int unique;
    size_t maxlen;
    char *prefix;
} PREFIX;

/* forward declarations */
extern char *upfix(PREFIX * p);
extern void upfix_exclude(PREFIX * p, char *name);
extern void upfix_free(PREFIX * p);
extern PREFIX *upfix_init(size_t maxlen, char *s);

#endif /* upfix_H */
