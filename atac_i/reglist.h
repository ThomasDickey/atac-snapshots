/* $Id: reglist.h,v 3.1 1997/05/11 23:05:41 tom Exp $ */

#ifndef reglist_H
#define reglist_H

typedef struct regnode {
	void		*data;
	int		idno;
	struct regnode	*left;
	struct regnode	*right;
} REGNODE;

typedef struct {
	REGNODE	*tree;
	int	idno;
} REGLST;

extern int reglst_insert P_(( REGLST *reglst, void *data ));
extern void reglst_free P_(( REGLST *reglst ));
extern REGLST *reglst_create P_(( void ));

#endif /* reglist_H */
