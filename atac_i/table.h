/* $Id: table.h,v 3.4 1997/12/10 01:51:44 tom Exp $ */

#ifndef table_H
#define table_H

#ifndef TABLE_DATATYPE
typedef	char TABLE_DATATYPE;	/* originally 'int', but that won't work with strcmp */
#endif

typedef struct node {
	TABLE_DATATYPE	*data;
	struct node	*left;
	struct node	*right;
	struct node	*up;
} NODE;

typedef	int	(*CMP) P_((const TABLE_DATATYPE *, const TABLE_DATATYPE *));

typedef struct {
	NODE	*tree;
	CMP	cmp;
} TABLE;

typedef void (*TabledataFree) P_((TABLE_DATATYPE *));

/* table.c */
extern TABLE_DATATYPE *table_find P_(( TABLE *table, TABLE_DATATYPE *key, NODE **node, int matchtype ));
extern TABLE_DATATYPE *table_insert P_(( TABLE *table, TABLE_DATATYPE *data, int duplicates ));
extern TABLE_DATATYPE *table_next P_(( TABLE *table, NODE **node ));
extern TABLE *table_create P_(( CMP cmp ));
extern int intcmp P_(( int a, int b ));
extern void table_free P_(( TABLE *table, TabledataFree datafree ));

#endif /* table_H */
