#ifndef rlist_H
#define rlist_H

typedef struct rlist {
    struct rlist *next;
    int sLine;
    int sCol;
    int eLine;
    int eCol;
} RLIST;

extern int rlist_get
  (RLIST ** head, int *sLine, int *sCol, int *eLine, int *eCol);
extern void rlist_reverse
  (RLIST ** head);
extern void rlist_put
  (RLIST ** head, int line1, int col1, int line2, int col2);
extern void rlist_free
  (RLIST ** head);
extern RLIST **rlist_create(void);

#endif /* rlist_H */
