/* $Id: allpaths.h,v 3.2 2013/12/08 22:07:18 tom Exp $ */

#ifndef allpaths_H
#define allpaths_H

typedef struct path {
    int end_id;
    struct path *next;
    unsigned nodes[1];
} PATH;

extern PATH *allpaths(DUG * dug, FILE *f, int list_them);

#endif /* allpaths_H */
