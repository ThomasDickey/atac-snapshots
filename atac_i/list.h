/* $Id: list.h,v 3.4 1997/05/11 21:43:07 tom Exp $ */

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

typedef void (*DataFree)(LIST_DATATYPE *);
typedef char*(*DataDump)(LIST_DATATYPE *);

typedef struct link {
#ifdef DEBUG
	char		*magic;
#endif
	LIST_DATATYPE	*data;
	struct link	*next;
	struct link	*prev;
} LIST;

#define LIST_FREE(head,func) list_free( head, (DataFree)func )
#define LIST_NEXT(head,prev,data) list_next( head, prev, (LIST_DATATYPE **)data )
#define LIST_PREV(head,prev,data) list_prev( head, prev, (LIST_DATATYPE **)data )
#define LIST_DUMP(head,func,label) list_dump( head, (DataDump)func, label )

/* list.c */
extern LIST *list_create P_(( void ));
extern int list_delete P_(( LIST *head, LIST **old ));
extern int list_free P_(( LIST *head, DataFree func ));
extern int list_next P_(( LIST *head, LIST **prev, LIST_DATATYPE **data ));
extern int list_prev P_(( LIST *head, LIST **prev, LIST_DATATYPE **data ));
extern int list_put P_(( LIST *head, LIST_DATATYPE *data ));
extern void list_dump P_(( LIST *head, DataDump func, char *label ));

#endif /* list_H */
