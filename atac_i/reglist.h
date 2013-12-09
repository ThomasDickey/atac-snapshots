/* $Id: reglist.h,v 3.2 2013/12/08 22:06:18 tom Exp $ */

#ifndef reglist_H
#define reglist_H

typedef struct regnode {
    void *data;
    int idno;
    struct regnode *left;
    struct regnode *right;
} REGNODE;

typedef struct {
    REGNODE *tree;
    int idno;
} REGLST;

extern int reglst_insert(REGLST * reglst, void *data);
extern void reglst_free(REGLST * reglst);
extern REGLST *reglst_create(void);

#endif /* reglist_H */
