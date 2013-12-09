/* $Id: list.h,v 3.7 2013/12/09 00:17:28 tom Exp $ */

#ifndef list_H
#define list_H

/*
* List is implemented as a linked list with forward and backward links.
* The head of the list is a sentinal; its data field is not used.
* The backward list is circular.  New entries are added at the end.
*/

#ifndef LIST_DATATYPE
#define LIST_DATATYPE void
#endif

typedef void (*DataFree) (LIST_DATATYPE *);
typedef char *(*DataDump) (LIST_DATATYPE *);

typedef struct link {
#ifdef DEBUG
    char *magic;
#endif
    LIST_DATATYPE *data;
    struct link *next;
    struct link *prev;
} LIST;

#define LIST_FREE(head,func) list_free( head, (DataFree)func )
#define LIST_NEXT(head,prev,data) list_next( head, prev, (LIST_DATATYPE **)data )
#define LIST_PREV(head,prev,data) list_prev( head, prev, (LIST_DATATYPE **)data )
#define LIST_DUMP(head,func,label) list_dump( head, (DataDump)func, label )

/* list.c */
extern LIST *list_create(void);
extern int list_delete(LIST * head, LIST ** old);
extern int list_free(LIST * head, DataFree func);
extern int list_next(LIST * head, LIST ** prev, LIST_DATATYPE ** data);
extern int list_prev(LIST * head, LIST ** prev, LIST_DATATYPE ** data);
extern int list_put(LIST * head, LIST_DATATYPE * data);
extern void list_dump(LIST * head, DataDump func, const char *label);

#endif /* list_H */
