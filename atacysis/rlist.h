#ifndef rlist_H
#define rlist_H

typedef struct rlist {
	struct rlist	*next;
	int		sLine;
	int		sCol;
	int		eLine;
	int		eCol;
} RLIST;

extern int rlist_get
	P_((RLIST **head, int *sLine, int *sCol, int *eLine, int *eCol));
extern void rlist_reverse
	P_((RLIST **head));
extern void rlist_put
	P_((RLIST **head, int line1, int col1, int line2, int col2));
extern void rlist_free
	P_((RLIST **head));
extern RLIST **rlist_create P_((void));

#endif /* rlist_H */
