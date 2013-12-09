/* $Id: table.h,v 3.5 2013/12/08 22:06:12 tom Exp $ */

#ifndef table_H
#define table_H

#ifndef TABLE_DATATYPE
typedef char TABLE_DATATYPE;	/* originally 'int', but that won't work with strcmp */
#endif

typedef struct node {
    TABLE_DATATYPE *data;
    struct node *left;
    struct node *right;
    struct node *up;
} NODE;

typedef int (*CMP) (const TABLE_DATATYPE *, const TABLE_DATATYPE *);

typedef struct {
    NODE *tree;
    CMP cmp;
} TABLE;

typedef void (*TabledataFree) (TABLE_DATATYPE *);

/* table.c */
extern TABLE_DATATYPE *table_find(TABLE * table, TABLE_DATATYPE * key, NODE ** node, int matchtype);
extern TABLE_DATATYPE *table_insert(TABLE * table, TABLE_DATATYPE * data, int duplicates);
extern TABLE_DATATYPE *table_next(TABLE * table, NODE ** node);
extern TABLE *table_create(CMP cmp);
extern int intcmp(int a, int b);
extern void table_free(TABLE * table, TabledataFree datafree);

#endif /* table_H */
