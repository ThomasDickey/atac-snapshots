/* $Id: allpaths.h,v 3.1 1997/05/11 23:37:42 tom Exp $ */

#ifndef allpaths_H
#define allpaths_H

typedef struct path {
	int		end_id;
	struct path	*next;
	unsigned	nodes[1];
} PATH;

extern PATH *allpaths P_(( DUG *dug, FILE *f, int list_them ));

#endif /* allpaths_H */
